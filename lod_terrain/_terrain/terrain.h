#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"
#include "transform.h"

#include "noise_generator.hpp"

#define PROFILING 1

class Terrain{
public:

   //rough side dictate if there should be only half of the vertices on the sides, that means one every two vertex is linearly interpolated
   //this allows in case of a LOD, to have seamless connexions between terrains
   void init(unsigned int sub_x, unsigned int sub_y, float start_x, float start_y, float end_x, float end_y, Noise_generator *noise_generator, bool rough_sides[4]){
      _pid = load_shaders("terrain_vshader.glsl", "terrain_fshader.glsl");
      if(_pid == 0) exit(-1);

      init(sub_x, sub_y, start_x, start_y, end_x, end_y, noise_generator, _pid, rough_sides);
   }

   //sub_x and sub_y define subdivision of th eterrain
   //it defines the definition of the terrain
   // a 4x3 terrain will contain 12 quads
   void init(unsigned int sub_x, unsigned int sub_y, float start_x, float start_y, float end_x, float end_y, Noise_generator *noise_generator, GLuint _pid, bool rough_sides[4]){
      // std::chrono::high_resolution_clock::time_point total_t1 = std::chrono::high_resolution_clock::now();
      this->_pid = _pid;

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      this->noise_generator = noise_generator;

      this->start_x = start_x;
      this->start_y = start_y;
      this->end_x = end_x;
      this->end_y = end_y;

      //need +1 other wise, 4x4 will be 3x3 square (due to algos)
      this->sub_x = sub_x+1;
      this->sub_y = sub_y+1;

      nb_vertices = this->sub_x*this->sub_y;
      unsigned int nb_quads = (this->sub_x-1)*(this->sub_y-1);
      unsigned int nb_tris = nb_quads*2;
      nb_indices = nb_tris*3;

      vertices = new GLfloat[nb_vertices*3];
      indices = new GLuint[nb_indices];
      normals = new GLfloat[nb_vertices*3];
      set_indices();

      heightmap.resize(this->sub_x);
      heightmap_normals.resize(this->sub_x);
      for(unsigned int i = 0; i < this->sub_x; i++){
         heightmap[i].resize(this->sub_y);
         heightmap_normals[i].resize(this->sub_y);
         for(unsigned int j = 0; j < this->sub_y; j++){
            heightmap_normals[i][j].resize(3);
         }
      }

      //std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      //generate the heightmap before generating the terrain
      generate_heightmap( this->sub_x, this->sub_y);
      //std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      generate_terrain(rough_sides);

      //unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

      //printf("duration generation heightmap: %lu\n", duration);

      glGenBuffers(1, &_vbo_normals);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_normals);
      glBufferData(GL_ARRAY_BUFFER, nb_vertices*3*sizeof(GLfloat), normals, GL_STATIC_DRAW);

      GLuint norm_id = glGetAttribLocation(_pid, "vertex_normal");
      glEnableVertexAttribArray(norm_id);
      glVertexAttribPointer(norm_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, nb_vertices*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_indices*sizeof(GLuint), indices, GL_STATIC_DRAW);

      glBindVertexArray(0);

      //std::chrono::high_resolution_clock::time_point total_t2 = std::chrono::high_resolution_clock::now();

      //unsigned long total_duration = std::chrono::duration_cast<std::chrono::microseconds>(total_t2 - total_t1).count();

      //printf("terrain init total duration: %lu\n", total_duration);
   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection, GLfloat light_position[3], GLfloat camera_position[3], bool activate_colour, bool activate_heightmap, bool activate_wireframe){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, camera_position);

      glUniform1ui( glGetUniformLocation(_pid, "activate_colour"), activate_colour);
      glUniform1ui( glGetUniformLocation(_pid, "activate_heightmap"), activate_heightmap);
      glUniform1ui( glGetUniformLocation(_pid, "activate_wireframe"), activate_wireframe);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      glDrawElements(GL_TRIANGLES, nb_indices, GL_UNSIGNED_INT, 0);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   //draw with transform matrix in memory
   void draw(glm::mat4x4 view, glm::mat4x4 projection, GLfloat light_position[3], GLfloat camera_position[3], bool activate_colour, bool activate_heightmap, bool activate_wireframe){
      this->draw(model_transf.get_matrix(), view, projection, light_position, camera_position, activate_colour, activate_heightmap, activate_wireframe);
   }

   void set_model(Transform transf){
      model_transf = transf;
   }

   float get_height_relative_pos(float pos_x, float pos_y){
      return noise_generator->get_noise_val(pos_x, pos_y);
   }

   float get_height(float pos_x, float pos_y){
      glm::vec4 vec(pos_x, 1.0f, pos_y, 1.0f);
      glm::mat4 transf_matrix = model_transf.get_matrix();
      glm::vec4 vec_transf = glm::inverse(transf_matrix)*vec;
      float height = noise_generator->get_noise_val(vec_transf[0], vec_transf[2]);
      height = (transf_matrix*glm::vec4(0.0f, height, 0.0f, 1.0f))[1];
      return height;
   }

   void cleanup(){
      if(vertices != NULL){
         delete[] vertices;
         vertices = NULL;
      }

      if(indices != NULL){
         delete[] indices;
         indices = NULL;
      }

      if( normals != NULL){
         delete[] normals;
         normals = NULL;
      }

      glDeleteBuffers(1, &_vbo);
      glDeleteBuffers(1, &_vbo_idx);
      glDeleteBuffers(1, &_vbo_normals);
      glDeleteVertexArrays(1, &_vao);
   }

   ~Terrain(){
      GLfloat *vertices = NULL;
      if (vertices != NULL){
         delete[] vertices;
         vertices = NULL;
      }

      if (indices != NULL){
         delete[] indices;
         indices = NULL;
      }

      if( normals != NULL){
         delete[] normals;
         normals = NULL;
      }
   }

private:
   Transform model_transf;

   Noise_generator *noise_generator;

   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_normals;
   GLuint _vbo_idx;
   GLuint _pid;

   float start_x;
   float start_y;
   float end_x;
   float end_y;

   unsigned int sub_x;
   unsigned int sub_y;

   GLfloat *vertices = NULL;
   GLuint *indices = NULL;
   GLfloat *normals = NULL;

   unsigned int nb_vertices;
   unsigned int nb_indices;

   std::vector<std::vector<float> > heightmap;
   std::vector<std::vector<std::vector<float> > > heightmap_normals;

   void print_debug_vec3(float vec[3]){
      printf("(%f, %f, %f\n)", vec[0], vec[1], vec[2]);
   }

   //fill the heights for each position of the terrain
   //the heightmap has been generated before, allowing for less calculation for the
   //same point
   //the rough_side allow to set a side as a level higher for LOD implementation
   //in this case, one every two heghtmap is taken and the middle is linearly interpolated from the two others
   void generate_terrain(bool rough_side[4]){

      //rough sides:
      // 0:north  1:east   2:south  3:west

      for(uint j = 0; j < sub_y; j++){
         for(uint i = 0; i < sub_x; i++){

            //makes that the relatives go from -1 to 1
            GLfloat relative_x = (float(i)/(sub_x-1));
            relative_x = (1.0-relative_x)*start_x + relative_x*end_x; //find real position with parameter using linear interp
            //rel_z not y as y is up
            GLfloat relative_z = (float(j)/(sub_y-1));
            relative_z = (1.0-relative_z)*start_y + relative_z*end_y;

            unsigned int cur_pos = (j*sub_x+i)*3;

            vertices[cur_pos] = relative_x;
            vertices[cur_pos+1] = heightmap[i][j];//(*this.*func)(relative_x, relative_z);
            vertices[cur_pos+2] = relative_z;

            normals[cur_pos+0] = heightmap_normals[i][j][0];//cross_product_vec[0];
            normals[cur_pos+1] = heightmap_normals[i][j][1];
            normals[cur_pos+2] = heightmap_normals[i][j][2];
         }
      }

      if( (rough_side[1] || rough_side[3]) ){ //right(1) or left(3)
         for(uint j = 1; j < sub_y; j+=2){
            int i = 0;

            if(rough_side[3]){
               i = 0;

               unsigned int prev_pos = ((j-1)*sub_x+i)*3;
               unsigned int cur_pos = (j*sub_x+i)*3;
               unsigned int next_pos = ((j+1)*sub_x+i)*3;

               vertices[cur_pos+1] = 0.5f*vertices[prev_pos+1] + 0.5f*vertices[next_pos+1];

               normals[cur_pos+0] = 0.5f*normals[prev_pos+0]+0.5f*normals[next_pos+0];
               normals[cur_pos+1] = 0.5f*normals[prev_pos+1]+0.5f*normals[next_pos+1];
               normals[cur_pos+2] = 0.5f*normals[prev_pos+2]+0.5f*normals[next_pos+2];
            }
            if (rough_side[1]){
               i=sub_x-1;

               unsigned int prev_pos = ((j-1)*sub_x+i)*3;
               unsigned int cur_pos = (j*sub_x+i)*3;
               unsigned int next_pos = ((j+1)*sub_x+i)*3;

               vertices[cur_pos+1] = 0.5f*vertices[prev_pos+1] + 0.5f*vertices[next_pos+1];

               normals[cur_pos+0] = 0.5f*normals[prev_pos+0]+0.5f*normals[next_pos+0];
               normals[cur_pos+1] = 0.5f*normals[prev_pos+1]+0.5f*normals[next_pos+1];
               normals[cur_pos+2] = 0.5f*normals[prev_pos+2]+0.5f*normals[next_pos+2];
            }
         }
      }

      if( (rough_side[0] || rough_side[2]) ) { //top(0) or bottom(2)
         // end_j = sub_y-1;
         for(uint i = 1; i < sub_x; i+=2){
            int j = 0;

            if(rough_side[0]){
               j = 0;
               unsigned int prev_pos = (j*sub_x+i-1)*3;
               unsigned int cur_pos = (j*sub_x+i)*3;
               unsigned int next_pos = (j*sub_x+i+1)*3;

               vertices[cur_pos+1] = 0.5f*vertices[prev_pos+1] + 0.5f*vertices[next_pos+1];

               normals[cur_pos+0] = 0.5f*normals[prev_pos+0]+0.5f*normals[next_pos+0];
               normals[cur_pos+1] = 0.5f*normals[prev_pos+1]+0.5f*normals[next_pos+1];
               normals[cur_pos+2] = 0.5f*normals[prev_pos+2]+0.5f*normals[next_pos+2];
            }
            if (rough_side[2]){
               j = sub_y-1;
               unsigned int prev_pos = (j*sub_x+i-1)*3;
               unsigned int cur_pos = (j*sub_x+i)*3;
               unsigned int next_pos = (j*sub_x+i+1)*3;

               vertices[cur_pos+1] = 0.5f*vertices[prev_pos+1] + 0.5f*vertices[next_pos+1];

               normals[cur_pos+0] = 0.5f*normals[prev_pos+0]+0.5f*normals[next_pos+0];
               normals[cur_pos+1] = 0.5f*normals[prev_pos+1]+0.5f*normals[next_pos+1];
               normals[cur_pos+2] = 0.5f*normals[prev_pos+2]+0.5f*normals[next_pos+2];
            }
         }
      }
   }

   void generate_heightmap( unsigned int grid_x, unsigned int grid_y){
      const float EPSILON = 0.001;

      for(uint j = 0; j < grid_x; j++){
         for(uint i = 0; i < grid_y; i++){

            //makes that the relatives go from -1 to 1
            GLfloat relative_x = (float(i)/(grid_x-1));
            relative_x = (1.0-relative_x)*start_x + relative_x*end_x; //find real position with parameter using linear interp
            //rel_z not y as y is up
            GLfloat relative_z = (float(j)/(grid_y-1));
            relative_z = (1.0-relative_z)*start_y + relative_z*end_y;

            heightmap[i][j] = noise_generator->get_noise_val(relative_x, relative_z);


            //save normals
            float derivative_x[3];
            float derivative_z[3];

            derivative_x[0] = 2*EPSILON;
            derivative_x[1] = noise_generator->get_noise_val(relative_x+EPSILON, relative_z)-noise_generator->get_noise_val(relative_x-EPSILON, relative_z);
            derivative_x[2] = 0.0f;

            normalize_vec3(derivative_x);

            derivative_z[0] = 0.0f;
            derivative_z[1] = noise_generator->get_noise_val(relative_x, relative_z+EPSILON)-noise_generator->get_noise_val(relative_x, relative_z-EPSILON);
            derivative_z[2] = 2*EPSILON;

            normalize_vec3(derivative_z);

            float cross_product_vec[3];

            cross_product(derivative_z, derivative_x, cross_product_vec);

            heightmap_normals[i][j][0] = cross_product_vec[0];
            heightmap_normals[i][j][1] = cross_product_vec[1];
            heightmap_normals[i][j][2] = cross_product_vec[2];
         }
      }
   }

   void normalize_vec3(float vec[3]){
      float length = sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);

      vec[0] = vec[0]/length;
      vec[1] = vec[1]/length;
      vec[2] = vec[2]/length;
   }

   void cross_product(float vec_a[3], float vec_b[3], float vec_out[3]){
      vec_out[0] = vec_a[1]*vec_b[2]-vec_a[2]*vec_b[1];
      vec_out[1] = vec_a[2]*vec_b[0]-vec_a[0]*vec_b[2];
      vec_out[2] = vec_a[0]*vec_b[1]-vec_a[1]*vec_b[0];
   }


   void set_indices(){
      for (uint j = 0; j < sub_y-1; j++){
         for(uint i = 0; i < sub_x-1; i++){
            unsigned int v0 = j*sub_x+i;
            unsigned int v1 = j*sub_x+i+1;
            unsigned int v2 = (j+1)*sub_x+i;
            unsigned int v3 = (j+1)*sub_x+i+1;

            unsigned int index_ptr = (j*(sub_x-1)+i)*6;

            indices[index_ptr] = v0;
            indices[index_ptr+1] = v3;
            indices[index_ptr+2] = v1;

            indices[index_ptr+3] = v0;
            indices[index_ptr+4] = v2;
            indices[index_ptr+5] = v3;
         }
      }
   }
};

#endif
