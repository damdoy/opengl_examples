#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"
#include "../transform.h"

#define PROFILING 1

enum Function_select{
   select_linear,
   select_ease, //ease is a polynomial that gives a smooth result better than linear but not as realistic as perlin
   select_perlin
};

//generate a terrain with hills of size 1x1
//and centred at the origin
//the height of the terrain goes from -1 to 1
//so basically a cube of size 1x1x1 centred at the origin
//the terrain is generated with a perlin noise
class Terrain{
public:
   //sub_x and sub_y define subdivision of the terrain
   //it defines the definition of the terrain
   // a 4x3 terrain will contain 12 quads
   void init(unsigned int sub_x, unsigned int sub_y){
      _pid = load_shaders("terrain_vshader.glsl", "terrain_fshader.glsl");
      if(_pid == 0) exit(-1);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      this->sub_x = sub_x;
      this->sub_y = sub_y;
      nb_vertices = sub_x*sub_y;
      unsigned int nb_quads = (sub_x-1)*(sub_y-1);
      unsigned int nb_tris = nb_quads*2;
      nb_indices = nb_tris*3;

      vertices = new GLfloat[nb_vertices*3];
      indices = new GLuint[nb_indices];
      normals = new GLfloat[nb_vertices*3];

      //fill the indices array
      set_indices();

      //parameters for the perlin noise
      noise_start_seg = 4; //how many subdivision at each noise level
      noise_levels = 6; //how many recursions
      noise_start_factor = 0.70f; //multiplication factor for the first noise level
      noise_factor = 0.40f; //multiplication factor for the next noise level
      func_select = select_perlin; //select the function

      heightmap.resize(sub_x);
      heightmap_normals.resize(sub_x);
      for(unsigned int i = 0; i < sub_x; i++){
         heightmap[i].resize(sub_y);
         heightmap_normals[i].resize(sub_y);
         for(unsigned int j = 0; j < sub_y; j++){
            heightmap_normals[i][j].resize(3);
         }
      }

      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      //generates heightmap as well as normals
      generate_heightmap( &Terrain::function_recurs_noise, sub_x, sub_y);
      std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

      //fills the buffers
      generate_terrain();

      unsigned long duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

      printf("duration generation heightmap: %lums\n", duration);


      /*printf("vertices (%d):\n", nb_vertices*3);
      for(int i = 0; i < nb_vertices*3; i+=3){
         printf("%d, %f, %f, %f\n", i, vertices[i], vertices[i+1], vertices[i+2]);
      }

      printf("indices (%d):\n", nb_indices);
      for(int i = 0; i < nb_indices; i+=3){
         printf("%d, %d, %d, %d\n", i, indices[i], indices[i+1], indices[i+2]);
      }*/

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
   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection, GLfloat light_position[3], GLfloat camera_position[3], bool activate_colour, bool activate_heightmap){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, camera_position);

      glUniform1ui( glGetUniformLocation(_pid, "activate_colour"), activate_colour);
      glUniform1ui( glGetUniformLocation(_pid, "activate_heightmap"), activate_heightmap);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      glDrawElements(GL_TRIANGLES, nb_indices, GL_UNSIGNED_INT, 0);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   //draw with transform matrix in memory
   void draw(glm::mat4x4 view, glm::mat4x4 projection, GLfloat light_position[3], GLfloat camera_position[3], bool activate_colour, bool activate_heightmap){
      this->draw(model_transf.get_matrix(), view, projection, light_position, camera_position, activate_colour, activate_heightmap);
   }

   //set model matrix
   void set_model(Transform transf){
      model_transf = transf;
   }

   //get height for a point without appling any transform
   float get_height_relative_pos(float pos_x, float pos_y){
      return function_recurs_noise(pos_x, pos_y);
   }

   //get height of a point which was trasformed
   float get_height(float pos_x, float pos_y){
      //transform back the point to (1,1)
      glm::vec4 vec(pos_x, 1.0f, pos_y, 1.0f);
      glm::mat4 transf_matrix = model_transf.get_matrix();
      glm::vec4 vec_transf = glm::inverse(transf_matrix)*vec;

      //get height for the point
      float height = function_recurs_noise(vec_transf[0], vec_transf[2]);
      height = (transf_matrix*glm::vec4(0.0f, height, 0.0f, 1.0f))[1];
      return height;
   }

   void cleanup(){
      if(vertices != NULL){
         delete vertices;
         vertices = NULL;
      }

      if(indices != NULL){
         delete indices;
         indices = NULL;
      }

      glDeleteBuffers(1, &_vbo);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

private:
   Transform model_transf;

   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_normals;
   GLuint _vbo_idx;
   GLuint _pid;

   unsigned int sub_x;
   unsigned int sub_y;

   GLfloat *vertices = NULL;
   GLuint *indices = NULL;
   GLfloat *normals = NULL;

   unsigned int nb_vertices;
   unsigned int nb_indices;

   //static vars for the noise generation
   unsigned int noise_start_seg;
   unsigned int noise_levels;
   float noise_start_factor;
   float noise_factor;
   Function_select func_select;

   std::vector<std::vector<float> > heightmap;
   std::vector<std::vector<std::vector<float> > > heightmap_normals;

   float static function_flat(float, float){
      return 0.0f;
   }

   float function_sin(float x, float y){
      return sin(x*y*10)/10;
   }

   //apply a recursive noise to the given point
   float function_recurs_noise(float x, float y){
      unsigned int segmentation = noise_start_seg; //2 = squares of 0.5 (1/2) (please be a power of two)
      float val_ret = 0.0f;
      float factor = noise_start_factor;


      for (uint i = 0; i < noise_levels; i++){
         switch(func_select){
         case select_linear:
            val_ret = (2.0f*function_frac_linear(x, y, segmentation)-1.0f)*factor +val_ret;
            break;
         case select_ease:
            val_ret = (2.0f*function_frac_ease(x, y, segmentation)-1.0f)*factor +val_ret;
            break;
         case select_perlin:
            val_ret = (2.0f*function_frac_perlin(x, y, segmentation)-1.0f)*factor +val_ret;
            break;
         }

         segmentation *= 2;
         factor *= noise_factor;
      }
      return val_ret;
   }

   float function_frac_linear(float x, float y, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;

      //printf("x was %f, x_corr is %f\n", x, x_corr);

      float rand_val_00;
      float rand_val_01;
      float rand_val_10;
      float rand_val_11;

      find_rand_vals(x_corr, y_corr, square_size, &rand_val_00, &rand_val_01, &rand_val_10, &rand_val_11);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;

      float lin_x_0 = (1.0f-frac_x)*rand_val_00 + (frac_x)*rand_val_10;
      float lin_x_1 = (1.0f-frac_x)*rand_val_01+ (frac_x)*rand_val_11;
      float lin = (1.0f-frac_y)*lin_x_0+ (frac_y)*lin_x_1;

      //printf("rem:%f, frac_x:%f rv00:%f, rv10%f, lx0:%f\n", remainder(x_corr, square_size), frac_x, rand_val_00, rand_val_10, lin_x_0);

      return lin;
   }

   //ease is a polynomial that looks smoother than linear, but not as natural as perlin
   float function_frac_ease(float x, float y, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;

      //printf("x was %f, x_corr is %f\n", x, x_corr);

      float rand_val_00;
      float rand_val_01;
      float rand_val_10;
      float rand_val_11;

      find_rand_vals(x_corr, y_corr, square_size, &rand_val_00, &rand_val_01, &rand_val_10, &rand_val_11);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;

      frac_x = 6*pow(frac_x, 5)-15*pow(frac_x, 4)+10*pow(frac_x, 3);
      frac_y = 6*pow(frac_y, 5)-15*pow(frac_y, 4)+10*pow(frac_y, 3);

      float lin_x_0 = (1.0f-frac_x)*rand_val_00 + (frac_x)*rand_val_10;
      float lin_x_1 = (1.0f-frac_x)*rand_val_01+ (frac_x)*rand_val_11;
      float lin = (1.0f-frac_y)*lin_x_0+ (frac_y)*lin_x_1;

      //printf("rem:%f, frac_x:%f rv00:%f, rv10%f, lx0:%f\n", remainder(x_corr, square_size), frac_x, rand_val_00, rand_val_10, lin_x_0);

      return lin;
   }

   //find perlin calulated height value for this point
   float function_frac_perlin(float x, float y, unsigned int segmentation){
      float square_size = 1.0f/(segmentation); //segmentation reduces height value

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;

      float rand_val_00;
      float rand_val_01;
      float rand_val_10;
      float rand_val_11;

      float gradient_00[2];
      float gradient_01[2];
      float gradient_10[2];
      float gradient_11[2];

      //find deterministic random values for x_corr and y_corr
      find_rand_vals(x_corr, y_corr, square_size, &rand_val_00, &rand_val_01, &rand_val_10, &rand_val_11);

      //find random gradients
      find_gradient_from_rand(rand_val_00*32, gradient_00);
      find_gradient_from_rand(rand_val_01*32, gradient_01);
      find_gradient_from_rand(rand_val_10*32, gradient_10);
      find_gradient_from_rand(rand_val_11*32, gradient_11);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;

      float vec_00[2] = {frac_x, frac_y}; // top left vector to pixel
      float vec_01[2] = {frac_x, -(1.0f-frac_y)}; // bottom left vector to pixel
      float vec_10[2] = { -(1.0f-frac_x) , frac_y}; // top right vector to pixel
      float vec_11[2] = { -(1.0f-frac_x), -(1.0f-frac_y)}; // bottom right vector to pixel

      float dot_00 = vec_00[0]*gradient_00[0]+vec_00[1]*gradient_00[1];
      float dot_01 = vec_01[0]*gradient_01[0]+vec_01[1]*gradient_01[1];
      float dot_10 = vec_10[0]*gradient_10[0]+vec_10[1]*gradient_10[1];
      float dot_11 = vec_11[0]*gradient_11[0]+vec_11[1]*gradient_11[1];

      frac_x = 6*pow(frac_x, 5)-15*pow(frac_x, 4)+10*pow(frac_x, 3);
      frac_y = 6*pow(frac_y, 5)-15*pow(frac_y, 4)+10*pow(frac_y, 3);

      float pixel_x1 = (1.0f-frac_x)*dot_00+frac_x*dot_10;
      float pixel_x2 = (1.0f-frac_x)*dot_01+frac_x*dot_11;
      float pixel = (1.0f-frac_y)*pixel_x1+frac_y*pixel_x2;
      pixel += 0.5f;

      return pixel;
   }

   void find_gradient_from_rand(unsigned int rand_val, float gradient[2]){
      switch(rand_val%4){
      case 0:
         gradient[0] = 1.0f;
         gradient[1] = 1.0f;
         break;
      case 1:
         gradient[0] = -1.0f;
         gradient[1] = 1.0f;
         break;
      case 2:
         gradient[0] = 1.0f;
         gradient[1] = -1.0f;
         break;
      case 3:
         gradient[0] = -1.0f;
         gradient[1] = -1.0f;
         break;
      }
   }

   void find_rand_vals(float x_corr, float y_corr, float square_size, float *rand_00, float *rand_01, float *rand_10, float *rand_11){

      float nearest_square_x = x_corr-fmod(x_corr, square_size);
      float nearest_square_y = y_corr-fmod(y_corr, square_size);
      float nearest_square_x_2 = x_corr-fmod(x_corr, square_size) + square_size;
      float nearest_square_y_2 = y_corr-fmod(y_corr, square_size) + square_size;

      unsigned int seed_x = (nearest_square_x*1024*11);
      unsigned int seed_y = (nearest_square_y*1024*11);
      unsigned int seed_x_2 = (nearest_square_x_2*1024*11);
      unsigned int seed_y_2 = (nearest_square_y_2*1024*11);

      *rand_00 = rand2d(seed_x, seed_y)/(float)MAX_RAND;
      *rand_01 = rand2d(seed_x, seed_y_2)/(float)MAX_RAND;
      *rand_10 = rand2d(seed_x_2, seed_y)/(float)MAX_RAND;
      *rand_11 = rand2d(seed_x_2, seed_y_2)/(float)MAX_RAND;
   }

   void print_debug_vec3(float vec[3]){
      printf("(%f, %f, %f\n)", vec[0], vec[1], vec[2]);
   }

   const unsigned int MAX_RAND = 0xFFFFFFFF;

   //repeatable rand functions
   unsigned int rand(){
      static unsigned int seed = 0;
      unsigned int a = 1664525;
      unsigned int c = 1013904223;
      seed = (a * seed + c);
      return seed;
   }

   unsigned int rand(unsigned int v){
      unsigned int a = 1664525;
      unsigned int c = 1013904223;
      return ((a * v + c));
   }

   unsigned int rand2d(unsigned int x, unsigned int y){
      //return (rand(x) + rand(y))%MAX_RAND;
      return rand(y+rand(x));
   }

   //take a 2d height function as parameter
   void generate_terrain(){

      for(uint j = 0; j < sub_y; j++){
         for(uint i = 0; i < sub_x; i++){

            //makes that the relatives go from -1 to 1
            GLfloat relative_x = (float(i)/(sub_x-1))*2-1;
            //rel_z not y as y is up
            GLfloat relative_z = (float(j)/(sub_y-1))*2-1;

            unsigned int cur_pos = (j*sub_x+i)*3;

            vertices[cur_pos] = relative_x;
            vertices[cur_pos+1] = heightmap[i][j];//(*this.*func)(relative_x, relative_z);
            vertices[cur_pos+2] = relative_z;

            normals[cur_pos+0] = heightmap_normals[i][j][0];//cross_product_vec[0];
            normals[cur_pos+1] = heightmap_normals[i][j][1];
            normals[cur_pos+2] = heightmap_normals[i][j][2];
         }
      }
   }

   //fills array with heightmaps values
   void generate_heightmap( float (Terrain:: *func)(float, float), unsigned int grid_x, unsigned int grid_y){
      const float EPSILON = 0.0001;

      for(uint j = 0; j < grid_x; j++){
         for(uint i = 0; i < grid_y; i++){

            //makes that the relatives go from -1 to 1
            GLfloat relative_x = (float(i)/(grid_x-1))*2-1;
            //y dir is up
            GLfloat relative_z = (float(j)/(grid_y-1))*2-1;

            //fill height map for each vertex
            heightmap[i][j] = (*this.*func)(relative_x, relative_z);

            //save normals
            //derivative_x[1] = (*this.*func)(relative_x+EPSILON, relative_z)-(*this.*func)(relative_x, relative_z);
            float derivative_x[3];
            float derivative_z[3];

            //calculate derivatives for each point of the grid using epsilon (far from optimized)
            derivative_x[0] = 2*EPSILON;
            derivative_x[1] = (*this.*func)(relative_x+EPSILON, relative_z)-(*this.*func)(relative_x-EPSILON, relative_z);
            derivative_x[2] = 0.0f;

            normalize_vec3(derivative_x);

            derivative_z[0] = 0.0f;
            derivative_z[1] = (*this.*func)(relative_x, relative_z+EPSILON)-(*this.*func)(relative_x, relative_z-EPSILON);
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
            //printf("%d\n", index_ptr);

            indices[index_ptr] = v0;
            indices[index_ptr+1] = v3;
            indices[index_ptr+2] = v1;

            indices[index_ptr+3] = v0;
            indices[index_ptr+4] = v2;
            indices[index_ptr+5] = v3;

            //printf("index: %d, %d, %d || %d, %d, %d\n", v0, v3, v1, v0, v2, v3);
         }
      }
   }
};

#endif
