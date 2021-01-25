#ifndef PLANE_H
#define PLANE_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "../texture.h"
#include "../drawable.h"

class Plane : public Drawable{
protected:
   static const int NB_COMPONENTS_VTEXCOORD = 8;
public:
   void init(GLuint pid){
      _pid = pid;
      if(_pid == 0) exit(-1);

      tessellation_shader_active = false;

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      GLfloat vpoint[] = {
         -1.0f, 0.0f, 1.0f,
         1.0f, 0.0f, 1.0f,
         1.0f, 0.0f, -1.0f,
         -1.0f,  0.0f, -1.0f,
      };

      GLfloat normals[] = {
         0.0f, 1.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
      };

      GLuint vpoint_index[] = {
         0, 1, 2,
         0, 2, 3
      };

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);
      GLuint vpoint_id = glGetAttribLocation(_pid, "vpoint");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      //texture will cover the whole plane
      vtexcoord[0] = 0.0f, vtexcoord[1] = 0.0f;
      vtexcoord[2] = 1.0f, vtexcoord[3] = 0.0f;
      vtexcoord[4] = 1.0f, vtexcoord[5] = 1.0f;
      vtexcoord[6] = 0.0f, vtexcoord[7] = 1.0f;

      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "vtexcoord");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glGenBuffers(1, &_vbo_normals);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_normals);
      glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

      glBindVertexArray(0);
      glUseProgram(0);
   }

   void init(GLuint pid, GLuint vao){
      this->_pid = pid;
      this->_vao = vao;
   }

   void set_texture(const Texture *tex){
      if(_tex != 0){
         //delete current texture
         glDeleteTextures(1, &_tex);
         _tex = 0;
      }

      //texture data definition
      glGenTextures(1, &_tex);
      glBindTexture(GL_TEXTURE_2D, _tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->get_width(), tex->get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, tex->get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glGenerateMipmap(GL_TEXTURE_2D);
      GLuint tex_id = glGetUniformLocation(_pid, "tex");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
   }

   void set_texture_filtering(GLint filtering){
      glBindTexture(GL_TEXTURE_2D, _tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
   }

   void set_texture_anisotropic(GLfloat anisotropic_value){ //needs GL_ARB_texture_filter_anisotropic or EXT
      glBindTexture(GL_TEXTURE_2D, _tex);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_value);
   }

   void set_texture_coord(GLfloat coord[NB_COMPONENTS_VTEXCOORD]){
      for (size_t i = 0; i < NB_COMPONENTS_VTEXCOORD; i++) {
         vtexcoord[i] = coord[i];
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }

   void set_texture_scale(GLfloat scale_x, GLfloat scale_y){
      for (size_t i = 0; i < NB_COMPONENTS_VTEXCOORD; i+=2) {
         vtexcoord[i] *= scale_x;
      }

      for (size_t i = 1; i < NB_COMPONENTS_VTEXCOORD; i+=2) {
         vtexcoord[i] *= scale_y;
      }

      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }

   void set_tessellation_shader_active(bool active){
      this->tessellation_shader_active = active;
   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex);

      //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      if(this-> tessellation_shader_active){
         glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, 0);
      }
      else{
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void draw(){

      draw(this->model_matrix, this->view_matrix, this->projection_matrix);
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo);
      glDeleteBuffers(1, &_vbo_tex);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

protected:
   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _vbo_normals;
   GLuint _pid;
   GLuint _tex;

   GLfloat vtexcoord[NB_COMPONENTS_VTEXCOORD];

   bool tessellation_shader_active;
};

#endif
