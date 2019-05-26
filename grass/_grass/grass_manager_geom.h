#ifndef GRASS_MANAGER_GEOM_H
#define GRASS_MANAGER_GEOM_H

#include <vector>
#include <cmath>

#include "grass_element.h"
#include "noise_generator.hpp"
#include "../texture_float.h"
#include "../_plane/plane_sine.h"


class Grass_manager_geom{

public:
   Grass_manager_geom(){

   }

   ~Grass_manager_geom(){

   }

   void init(Plane_sine *plane_sine){

      srand(time(0));

      this->plane_sine = plane_sine;

      lst_grass_elements.resize(nb_grass_elem_side*nb_grass_elem_side);

      std::vector<std::vector<float> > noise_val = noise_gen.get_2D_noise(512, 512, -1.0f, 1.0f, -1.0f, 1.0f);
      tex_wind_noise.set_data(noise_val);

      GLuint pid_grass = load_shaders("grass_geom_vshader.glsl", "grass_blades_fshader.glsl", "grass_geom_gshader.glsl");
      _pid = pid_grass;

      vao_grass = 0;

      {
         glGenVertexArrays(1, &vao_grass);
         glBindVertexArray(vao_grass);

         //dummy point given to the shader
         //the geometry shader will create grass from the matrices
         GLfloat vpoint[] = {
            0.0f, 0.0f, 0.0f, // 0
         };

         GLuint vpoint_index[] = {
            0
         };

         GLfloat per_surface_normals[] = {0, 1, 0,
         };

         GLuint _vbo;

         glGenBuffers(1, &_vbo);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo);
         glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

         GLuint vpoint_id = glGetAttribLocation(_pid, "position");
         glEnableVertexAttribArray(vpoint_id);
         glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

         GLuint _vbo_sur_norm;
         glGenBuffers(1, &_vbo_sur_norm);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
         glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

         GLuint id_pos = glGetAttribLocation(_pid, "surface_normal");
         glEnableVertexAttribArray(id_pos);
         glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

         GLuint _vbo_idx;
         glGenBuffers(1, &_vbo_idx);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);
      }

      //defines the matrices for creating the grass in the geometry shader
      for (size_t i = 0; i < nb_grass_elem_side; i++) {
         for (size_t j = 0; j < nb_grass_elem_side; j++) {
            Grass_element *ge = new Grass_element;
            ge->init_matrices(); //load the matrices
            Transform t;
            float pos_x = -(size_to_draw/2.0f)*(1-(float(i)/(nb_grass_elem_side-1)))+(size_to_draw/2.0f)*(float(i)/(nb_grass_elem_side-1));
            float pos_y = -(size_to_draw/2.0f)*(1-(float(j)/(nb_grass_elem_side-1)))+(size_to_draw/2.0f)*(float(j)/(nb_grass_elem_side-1));

            int val_rand = rand()%1000;
            float frand = float(val_rand)/1000.0f;

            pos_x += frand/10.0f-0.05;
            pos_y += frand/10.0f-0.05;

            t.translate(pos_x, plane_sine->get_height(pos_x, pos_y), pos_y);
            t.rotate(0.0, 1.0, 0.0, pos_x*pos_y*5.244);
            t.scale(0.3, 1.7, 0.3);
            t.scale(1.0, 1.0 + frand*1.0-0.25, 1.0);
            t.translate(0, 2.0, 0);
            ge->set_model_matrix(t.get_matrix());

            mat_vector.push_back(ge->get_transf_0()); //only get one of the side (wont be using face culling for the drawing)

            lst_grass_elements[j+i*nb_grass_elem_side] = ge;
         }
      }

      //texture data definition for the wind (perlin noise)
      glGenTextures(1, &_tex_wind_noise);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_wind_noise.get_width(), tex_wind_noise.get_height(), 0, GL_RED, GL_FLOAT, tex_wind_noise.get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //can be clamp to edge, clamp to border or gl repeat
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_wind");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

      glBindTexture(GL_TEXTURE_2D, 0);

      wind_dir[0] = wind_dir[1] = 0.0f;


      //list of matrices for instanced drawing
      GLuint _vbo_transf;
      glGenBuffers(1, &_vbo_transf);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

      glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector.size()];

      printf("will draw %lu grass\n", mat_vector.size());
      nb_grass_to_draw = mat_vector.size();

      for (size_t i = 0; i < mat_vector.size(); i++) {
         matrices_array[i] = mat_vector[i];
      }

      //buffers the matrices
      glBufferData(GL_ARRAY_BUFFER, mat_vector.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

      uint32_t vec4_size = sizeof(glm::vec4);

      // defines the matrices for the shader, matrices are 4x4, needs to be done in 4 parts
      GLuint model_attrib_0 = glGetAttribLocation(_pid, "model_mat");

      glEnableVertexAttribArray(model_attrib_0);
      glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

      glEnableVertexAttribArray(model_attrib_0+1);
      glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

      glEnableVertexAttribArray(model_attrib_0+2);
      glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2*vec4_size));

      glEnableVertexAttribArray(model_attrib_0+3);
      glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

      glVertexAttribDivisor(model_attrib_0, 1);
      glVertexAttribDivisor(model_attrib_0+1, 1);
      glVertexAttribDivisor(model_attrib_0+2, 1);
      glVertexAttribDivisor(model_attrib_0+3, 1);

      glBindVertexArray(0);

      nb_grass_to_draw = mat_vector.size();
      mat_vector.clear();
      delete[] matrices_array;
   }

   void set_light_pos(GLfloat light_position[3]){
      this->light_position[0] = light_position[0];
      this->light_position[1] = light_position[1];
      this->light_position[2] = light_position[2];
   }

   void set_camera_pos(GLfloat camera_position[3]){
      this->camera_position[0] = camera_position[0];
      this->camera_position[1] = camera_position[1];
      this->camera_position[2] = camera_position[2];
   }

   virtual void set_view_matrix(glm::mat4x4 view){
      this->view_matrix = view;
   }

   virtual void set_projection_matrix(glm::mat4x4 projection){
      this->projection_matrix = projection;
   }

   void draw(){

      wind_dir[0] += 0.0004f;
      if(wind_dir[0] > 40.0f){
         wind_dir[0] = 0;
      }

      wind_dir[1] += 0.0002f;
      if(wind_dir[1] > 40.0f){
         wind_dir[1] = 0;
      }

      glUseProgram(_pid);
      glBindVertexArray(vao_grass);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_wind");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

      glUniform2fv( glGetUniformLocation(_pid, "wind_dir"), 1, this->wind_dir);
      glUniform1f( glGetUniformLocation(_pid, "min_pos"), -20.0f);
      glUniform1f( glGetUniformLocation(_pid, "max_pos"), 20.0f);

      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDisable(GL_CULL_FACE);

      glDrawElementsInstanced(GL_POINTS, 1, GL_UNSIGNED_INT, 0, nb_grass_to_draw);
      glEnable(GL_CULL_FACE);
      glBindVertexArray(0);
      glUseProgram(0);

   }

protected:
   std::vector<Grass_element*> lst_grass_elements;
   Plane_sine *plane_sine;

   Noise_generator noise_gen;
   Texture_float tex_wind_noise;

   uint nb_grass_to_draw;

   float camera_position[3];
   float light_position[3];

   float wind_dir[2];

   glm::mat4x4 view_matrix;
   glm::mat4x4 projection_matrix;

   const uint nb_grass_elem_side = 300;
   const float size_to_draw = 100;

   GLuint _pid;
   GLuint _tex_grass;
   GLuint _tex_wind_noise;
   GLuint vao_grass;

   std::vector<glm::mat4x4> mat_vector;
};

#endif
