#ifndef SPHERE_H
#define SPHERE_H

#include <cmath>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "drawable.h"

const float PI = 3.141592;

// can define the precision (segments) of the sphere and the vertices are
// generated automatically.
// But it doesn't use indexed vertices and is using both per face and per vertex
// normals (in order to test both)
class Sphere : public Drawable{
public:
   void init(GLuint shader_pid, unsigned int vertical_segments, unsigned int horizontal_segments){
      _pid = shader_pid;
      if(_pid == 0) exit(-1);

      inverted = false;

      glUseProgram(_pid);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      if (vertical_segments < 3){
         nb_vertical_lines = 3;
      }
      else{
         nb_vertical_lines = vertical_segments;
      }

      if (horizontal_segments < 3){
         nb_horizontal_lines = 3;
      }
      else{
         nb_horizontal_lines = horizontal_segments;
      }

      _num_vertices = 3*nb_horizontal_lines+3*nb_horizontal_lines+6*(nb_vertical_lines-2)*nb_horizontal_lines;

      GLfloat position[_num_vertices*3];
      GLfloat per_vertex_normals[_num_vertices*3];
      GLfloat per_surface_normals[_num_vertices*3];

      generate_tris(position, per_vertex_normals, per_surface_normals);

      glGenBuffers(1, &_vbo_pos);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
      glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

      id_pos = glGetAttribLocation(_pid, "surface_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_vert_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_vert_norm);
      glBufferData(GL_ARRAY_BUFFER, sizeof(per_vertex_normals), per_vertex_normals, GL_STATIC_DRAW);

      id_pos = glGetAttribLocation(_pid, "vertex_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);


      glBindVertexArray(0);
   }

   void set_invert(bool inverted){
      this->inverted = inverted;
   }


   //minimalist draw, for simpler examples
   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      if(this->inverted){
         glFrontFace(GL_CW);
      }

      glDrawArrays(GL_TRIANGLES, 0, _num_vertices);

      glFrontFace(GL_CCW);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void draw(){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);
      glUniform1ui( glGetUniformLocation(_pid, "lighting_mode"), 2);
      glUniform1ui( glGetUniformLocation(_pid, "activate_specular"), 1);

      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);

      glUniform4fv( glGetUniformLocation(_pid, "clip_coord"), 1, this->clip_coord);
      glUniform1ui( glGetUniformLocation(_pid, "shadow_mapping_effect"), this->shadow_mapping_effect);
      glUniform1ui( glGetUniformLocation(_pid, "shadow_buffer_tex_size"), this->shadow_buffer_texture_width); //width=height in this case

      if(has_shadow_buffer){
         glUniformMatrix4fv( glGetUniformLocation(_pid, "shadow_matrix"), 1, GL_FALSE, glm::value_ptr(this->shadow_matrix));

         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, _shadow_texture_id);
         GLuint tex_id = glGetUniformLocation(_pid, "shadow_buffer_tex");
         glUniform1i(tex_id, 1 /*GL_TEXTURE0*/);
      }

      draw(this->model_matrix, this->view_matrix, this->projection_matrix);

      if(has_shadow_buffer){
         glBindTexture(GL_TEXTURE_2D, 0);
      }

      glUseProgram(0);
      glBindVertexArray(0);


   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo_idx);
      glDeleteBuffers(1, &_vbo_pos);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

protected:
   GLuint _vao;
   GLuint _vbo_pos;
   GLuint _vbo_idx;
   GLuint _vbo_sur_norm;
   GLuint _vbo_vert_norm;
   GLuint _pid;
   int _num_vertices;
   bool inverted;

   // this defines the precision of the sphere, in term of vertical lines and horizontal
   // in order to have something that ressembles a sphere, a minimum value is defined
   // for both
   unsigned int nb_vertical_lines;
   unsigned int nb_horizontal_lines;

   void normalize_vec3(float in_x, float in_y, float in_z, float *out_x, float *out_y, float *out_z){
      float length = sqrt(in_x*in_x+in_y*in_y+in_z*in_z);

      *out_x = in_x/length;
      *out_y = in_y/length;
      *out_z = in_z/length;
   }

   //algorithm from https://gamedev.stackexchange.com/questions/16585/how-do-you-programmatically-generate-a-sphere
   void generate_tris(GLfloat *array, GLfloat *per_vertex_normals, GLfloat *per_surface_normals){

      unsigned int counter_vertices = 0;
      unsigned int counter_surface_norm = 0;
      unsigned int counter_vertex_norm = 0;

      GLfloat norm_x, norm_y, norm_z;

      for(uint vert = 0; vert < nb_vertical_lines; vert++){
         float theta1 = float(vert)/(nb_vertical_lines)*PI;
         float theta2 = float(vert+1)/(nb_vertical_lines)*PI;

         for(uint hor = 0; hor < nb_horizontal_lines; hor++){
            float phi1 = float(hor)/nb_horizontal_lines*2*PI;
            float phi2 = float(hor+1)/nb_horizontal_lines*2*PI;

            float vertex11_x = sin(theta1)*cos(phi1);
            float vertex11_y = sin(theta1)*sin(phi1);
            float vertex11_z = cos(theta1);

            float vertex12_x = sin(theta1)*cos(phi2);
            float vertex12_y = sin(theta1)*sin(phi2);
            float vertex12_z = cos(theta1);

            float vertex21_x = sin(theta2)*cos(phi1);
            float vertex21_y = sin(theta2)*sin(phi1);
            float vertex21_z = cos(theta2);

            float vertex22_x = sin(theta2)*cos(phi2);
            float vertex22_y = sin(theta2)*sin(phi2);
            float vertex22_z = cos(theta2);

            if(vert == 0){ //top of sphere, only one tri
               array[counter_vertices] = vertex11_x;
               array[counter_vertices+1] = vertex11_y;
               array[counter_vertices+2] = vertex11_z;

               array[counter_vertices+3] = vertex21_x;
               array[counter_vertices+4] = vertex21_y;
               array[counter_vertices+5] = vertex21_z;

               array[counter_vertices+6] = vertex22_x;
               array[counter_vertices+7] = vertex22_y;
               array[counter_vertices+8] = vertex22_z;

               norm_x = (vertex21_y-vertex11_y)*(vertex22_z-vertex11_z)-(vertex21_z-vertex11_z)*(vertex22_y-vertex11_y);
               norm_y = (vertex21_z-vertex11_z)*(vertex22_x-vertex11_x)-(vertex21_x-vertex11_x)*(vertex22_z-vertex11_z);
               norm_z = (vertex21_x-vertex11_x)*(vertex22_y-vertex11_y)-(vertex21_y-vertex11_y)*(vertex22_x-vertex11_x);

               normalize_vec3(norm_x, norm_y, norm_z, &norm_x, &norm_y, &norm_z);

               per_surface_normals[counter_surface_norm] = norm_x;
               per_surface_normals[counter_surface_norm+1] = norm_y;
               per_surface_normals[counter_surface_norm+2] = norm_z;

               per_surface_normals[counter_surface_norm+3] = norm_x;
               per_surface_normals[counter_surface_norm+4] = norm_y;
               per_surface_normals[counter_surface_norm+5] = norm_z;

               per_surface_normals[counter_surface_norm+6] = norm_x;
               per_surface_normals[counter_surface_norm+7] = norm_y;
               per_surface_normals[counter_surface_norm+8] = norm_z;

               normalize_vec3(vertex11_x, vertex11_y, vertex11_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm] = norm_x;
               per_vertex_normals[counter_vertex_norm+1] = norm_y;
               per_vertex_normals[counter_vertex_norm+2] = norm_z;

               normalize_vec3(vertex21_x, vertex21_y, vertex21_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+3] = norm_x;
               per_vertex_normals[counter_vertex_norm+4] = norm_y;
               per_vertex_normals[counter_vertex_norm+5] = norm_z;

               normalize_vec3(vertex22_x, vertex22_y, vertex22_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+6] = norm_x;
               per_vertex_normals[counter_vertex_norm+7] = norm_y;
               per_vertex_normals[counter_vertex_norm+8] = norm_z;

               counter_vertices += 9;
               counter_surface_norm += 9;
               counter_vertex_norm += 9;
            }
            else if(vert == nb_vertical_lines-1){ //bottom of sphere, only one tri
               array[counter_vertices] = vertex11_x;
               array[counter_vertices+1] = vertex11_y;
               array[counter_vertices+2] = vertex11_z;

               array[counter_vertices+3] = vertex21_x;
               array[counter_vertices+4] = vertex21_y;
               array[counter_vertices+5] = vertex21_z;

               array[counter_vertices+6] = vertex12_x;
               array[counter_vertices+7] = vertex12_y;
               array[counter_vertices+8] = vertex12_z;

               norm_x = (vertex21_y-vertex11_y)*(vertex22_z-vertex11_z)-(vertex21_z-vertex11_z)*(vertex22_y-vertex11_y);
               norm_y = (vertex21_z-vertex11_z)*(vertex22_x-vertex11_x)-(vertex21_x-vertex11_x)*(vertex22_z-vertex11_z);
               norm_z = (vertex21_x-vertex11_x)*(vertex22_y-vertex11_y)-(vertex21_y-vertex11_y)*(vertex22_x-vertex11_x);

               normalize_vec3(norm_x, norm_y, norm_z, &norm_x, &norm_y, &norm_z);

               per_surface_normals[counter_surface_norm] = norm_x;
               per_surface_normals[counter_surface_norm+1] = norm_y;
               per_surface_normals[counter_surface_norm+2] = norm_z;

               per_surface_normals[counter_surface_norm+3] = norm_x;
               per_surface_normals[counter_surface_norm+4] = norm_y;
               per_surface_normals[counter_surface_norm+5] = norm_z;

               per_surface_normals[counter_surface_norm+6] = norm_x;
               per_surface_normals[counter_surface_norm+7] = norm_y;
               per_surface_normals[counter_surface_norm+8] = norm_z;

               normalize_vec3(vertex11_x, vertex11_y, vertex11_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm] = norm_x;
               per_vertex_normals[counter_vertex_norm+1] = norm_y;
               per_vertex_normals[counter_vertex_norm+2] = norm_z;

               normalize_vec3(vertex21_x, vertex21_y, vertex21_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+3] = norm_x;
               per_vertex_normals[counter_vertex_norm+4] = norm_y;
               per_vertex_normals[counter_vertex_norm+5] = norm_z;

               normalize_vec3(vertex12_x, vertex12_y, vertex12_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+6] = norm_x;
               per_vertex_normals[counter_vertex_norm+7] = norm_y;
               per_vertex_normals[counter_vertex_norm+8] = norm_z;

               counter_vertices += 9;
               counter_surface_norm += 9;
               counter_vertex_norm += 9;
            }
            else{
               array[counter_vertices] = vertex11_x;
               array[counter_vertices+1] = vertex11_y;
               array[counter_vertices+2] = vertex11_z;

               array[counter_vertices+3] = vertex22_x;
               array[counter_vertices+4] = vertex22_y;
               array[counter_vertices+5] = vertex22_z;

               array[counter_vertices+6] = vertex12_x;
               array[counter_vertices+7] = vertex12_y;
               array[counter_vertices+8] = vertex12_z;

               norm_x = (vertex22_y-vertex11_y)*(vertex12_z-vertex11_z)-(vertex22_z-vertex11_z)*(vertex12_y-vertex11_y);
               norm_y = (vertex22_z-vertex11_z)*(vertex12_x-vertex11_x)-(vertex22_x-vertex11_x)*(vertex12_z-vertex11_z);
               norm_z = (vertex22_x-vertex11_x)*(vertex12_y-vertex11_y)-(vertex22_y-vertex11_y)*(vertex12_x-vertex11_x);

               normalize_vec3(norm_x, norm_y, norm_z, &norm_x, &norm_y, &norm_z);

               per_surface_normals[counter_surface_norm] = norm_x;
               per_surface_normals[counter_surface_norm+1] = norm_y;
               per_surface_normals[counter_surface_norm+2] = norm_z;

               per_surface_normals[counter_surface_norm+3] = norm_x;
               per_surface_normals[counter_surface_norm+4] = norm_y;
               per_surface_normals[counter_surface_norm+5] = norm_z;

               per_surface_normals[counter_surface_norm+6] = norm_x;
               per_surface_normals[counter_surface_norm+7] = norm_y;
               per_surface_normals[counter_surface_norm+8] = norm_z;

               normalize_vec3(vertex11_x, vertex11_y, vertex11_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm] = norm_x;
               per_vertex_normals[counter_vertex_norm+1] = norm_y;
               per_vertex_normals[counter_vertex_norm+2] = norm_z;

               normalize_vec3(vertex22_x, vertex22_y, vertex22_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+3] = norm_x;
               per_vertex_normals[counter_vertex_norm+4] = norm_y;
               per_vertex_normals[counter_vertex_norm+5] = norm_z;

               normalize_vec3(vertex12_x, vertex12_y, vertex12_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+6] = norm_x;
               per_vertex_normals[counter_vertex_norm+7] = norm_y;
               per_vertex_normals[counter_vertex_norm+8] = norm_z;

               counter_vertices += 9;
               counter_surface_norm += 9;
               counter_vertex_norm += 9;

               array[counter_vertices] = vertex11_x;
               array[counter_vertices+1] = vertex11_y;
               array[counter_vertices+2] = vertex11_z;

               array[counter_vertices+3] = vertex21_x;
               array[counter_vertices+4] = vertex21_y;
               array[counter_vertices+5] = vertex21_z;

               array[counter_vertices+6] = vertex22_x;
               array[counter_vertices+7] = vertex22_y;
               array[counter_vertices+8] = vertex22_z;

               norm_x = (vertex21_y-vertex11_y)*(vertex22_z-vertex11_z)-(vertex21_z-vertex11_z)*(vertex22_y-vertex11_y);
               norm_y = (vertex21_z-vertex11_z)*(vertex22_x-vertex11_x)-(vertex21_x-vertex11_x)*(vertex22_z-vertex11_z);
               norm_z = (vertex21_x-vertex11_x)*(vertex22_y-vertex11_y)-(vertex21_y-vertex11_y)*(vertex22_x-vertex11_x);

               normalize_vec3(norm_x, norm_y, norm_z, &norm_x, &norm_y, &norm_z);

               per_surface_normals[counter_surface_norm] = norm_x;
               per_surface_normals[counter_surface_norm+1] = norm_y;
               per_surface_normals[counter_surface_norm+2] = norm_z;

               per_surface_normals[counter_surface_norm+3] = norm_x;
               per_surface_normals[counter_surface_norm+4] = norm_y;
               per_surface_normals[counter_surface_norm+5] = norm_z;

               per_surface_normals[counter_surface_norm+6] = norm_x;
               per_surface_normals[counter_surface_norm+7] = norm_y;
               per_surface_normals[counter_surface_norm+8] = norm_z;

               normalize_vec3(vertex11_x, vertex11_y, vertex11_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm] = norm_x;
               per_vertex_normals[counter_vertex_norm+1] = norm_y;
               per_vertex_normals[counter_vertex_norm+2] = norm_z;

               normalize_vec3(vertex21_x, vertex21_y, vertex21_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+3] = norm_x;
               per_vertex_normals[counter_vertex_norm+4] = norm_y;
               per_vertex_normals[counter_vertex_norm+5] = norm_z;

               normalize_vec3(vertex22_x, vertex22_y, vertex22_z, &norm_x, &norm_y, &norm_z);
               per_vertex_normals[counter_vertex_norm+6] = norm_x;
               per_vertex_normals[counter_vertex_norm+7] = norm_y;
               per_vertex_normals[counter_vertex_norm+8] = norm_z;

               counter_vertices += 9;
               counter_surface_norm += 9;
               counter_vertex_norm += 9;
            }
         }
      }
   }
};

#endif
