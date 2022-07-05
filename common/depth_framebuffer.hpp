#ifndef DEPTH_FRAMEBUFFER_HPP
#define DEPTH_FRAMEBUFFER_HPP

#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"
#include "drawable.h"


//framebuffer that keeps the depth information in its texture
class Depth_framebuffer{

public:
   void init(unsigned texture_size, float perspective = 3.1415f/1.6f){
      init(texture_size, texture_size, perspective);
   }

   void init(unsigned tex_width, unsigned tex_height, float perspective = 3.1415f/1.6f){

      this->tex_width = tex_width;
      this->tex_height = tex_height;

      this->light_dir[0] = 0;
      this->light_dir[1] = 0;
      this->light_dir[2] = 0;

      // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
      glGenFramebuffers(1, &depth_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

      // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
      glGenTextures(1, &depth_texture_id);
      glBindTexture(GL_TEXTURE_2D, depth_texture_id);
      glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, tex_width, tex_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture_id, 0);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "!!!ERROR: depth Framebuffer not OK :(" << std::endl;

      // Disable color rendering as there are no color attachments
      glDrawBuffer(GL_NONE);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      //default perspective matrix
      set_perspective_mat(glm::perspective(perspective, (float)tex_width/(float)tex_height, 1.0f, 1000.0f));
   }

   void set_light_pos(GLfloat light_position[3]){
      this->light_position[0] = light_position[0];
      this->light_position[1] = light_position[1];
      this->light_position[2] = light_position[2];
   }

   void set_light_dir(GLfloat light_dir[3]){
      this->light_dir[0] = light_dir[0];
      this->light_dir[1] = light_dir[1];
      this->light_dir[2] = light_dir[2];
   }

   void set_camera(Camera *cam){
      this->camera = cam;
   }

   glm::mat4x4 get_depth_view_mat(){
      if(camera == NULL){
         glm::vec3 eye(light_position[0], light_position[1], light_position[2]);
         glm::vec3 center(light_dir[0], light_dir[1], light_dir[2]);
         glm::vec3 up(0.0f, 1.0f, 0.0f);

         return glm::lookAt(eye, center, up);

      }
      else{
         return this->camera->getMatrix();
      }
   }

   glm::mat4x4 get_depth_perspective_mat(){
      return get_perspective_mat();
   }

   glm::mat4x4 get_shadow_mat(){
      glm::mat4x4 bias_matrix(
         0.5, 0.0, 0.0, 0.0,
         0.0, 0.5, 0.0, 0.0,
         0.0, 0.0, 0.5, 0.0,
         0.5, 0.5, 0.5, 1.0
      );

      return bias_matrix*get_depth_perspective_mat()*get_depth_view_mat();
   }

   void draw_fb(std::vector<Drawable*> *lst_drawable){

      glm::vec3 eye(light_position[0], light_position[1], light_position[2]);
      glm::vec3 center(light_dir[0], light_dir[1], light_dir[2]);
      glm::vec3 up(0.0f, 1.0f, 0.0f);

      glm::mat4x4 view_mat = glm::lookAt(eye, center, up);

      if(this->camera != NULL){
         view_mat = camera->getMatrix();
      }
      glm::mat4x4 projection_mat = get_perspective_mat();

      glClearDepth(1.0f);

      // Bind the "depth only" FBO and set the viewport to the size
      // of the depth texture
      glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
      glViewport(0, 0, tex_width, tex_height);

      glClear(GL_DEPTH_BUFFER_BIT);

      // Enable polygon offset to resolve depth-fighting isuses
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2.0f, 4.0f);

      // Draw from the light’s point of view
      for (size_t i = 0; i < lst_drawable->size(); i++) {
         //update the view and projection matrices
         lst_drawable->at(i)->set_view_matrix(view_mat);
         lst_drawable->at(i)->set_projection_matrix(projection_mat);

         lst_drawable->at(i)->draw();
      }

      glDisable(GL_POLYGON_OFFSET_FILL);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   GLuint get_texture_id(){
      return depth_texture_id;
   }

   void set_perspective_mat(glm::mat4x4 perspective_mat){
      projection_matrix = perspective_mat;
   }

protected:
   GLuint depth_texture_id;
   unsigned int tex_width;
   unsigned int tex_height;
   GLuint depth_fbo;
   GLfloat light_position[3];
   GLfloat light_dir[3];
   glm::mat4x4 projection_matrix;
   glm::mat4x4 view_matrix;
   Camera *camera;

   glm::mat4x4 get_perspective_mat(){

      return projection_matrix;
      // if(type == 0){ //FOR SHADOW MAPPING (compatibility)
      //    return glm::perspective(3.1415f/1.6f, (float)tex_width/(float)tex_height, 1.0f, 1000.0f);
      // }
      // else{ //FOR same perspective as camera
      //    return glm::perspective(3.1415f/2.0f, (float)tex_width/(float)tex_height, 0.1f, 1000.0f);
      // }

   }

};

#endif
