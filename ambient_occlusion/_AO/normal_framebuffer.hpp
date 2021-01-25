#ifndef NORMAL_FRAMEBUFFER_HPP
#define NORMAL_FRAMEBUFFER_HPP

#include <cmath>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"
#include "drawable.h"
#include "camera.h"

//keeps normals in a framebuffer
class Normal_framebuffer{
public:
   void init(unsigned tex_width, unsigned tex_height){

      camera = NULL;

      _pid_normal = load_shaders("normal_map_vshader.glsl", "normal_map_fshader.glsl");

      this->tex_width = tex_width;
      this->tex_height = tex_height;

      // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
      glGenFramebuffers(1, &fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);

      // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
      glGenTextures(1, &texture_id);
      glBindTexture(GL_TEXTURE_2D, texture_id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_width, tex_height, 0, GL_RGB, GL_FLOAT, 0);

      glBindTexture(GL_TEXTURE_2D, 0);

      //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

      glGenRenderbuffers(1, &_depth_rb);
      glBindRenderbuffer(GL_RENDERBUFFER, _depth_rb);

      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, tex_width, tex_height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_rb);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "!!!ERROR: depth Framebuffer not OK :(" << std::endl;

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   void set_camera(Camera *cam){
      this->camera = cam;
   }

   void draw_fb(std::vector<Drawable*> *lst_drawable){

      glm::mat4x4 view_mat;
      glm::mat4x4 projection_mat;

      if(this->camera != NULL){
         view_mat = camera->getMatrix();
         projection_mat = camera->get_perspective_mat();
      }

      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glViewport(0, 0, tex_width, tex_height);

      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      // Draw from the light’s point of view
      for (size_t i = 0; i < lst_drawable->size(); i++) {
         //update the view and projection matrices

         GLuint shader_before = lst_drawable->at(i)->get_shader();
         lst_drawable->at(i)->set_view_matrix(view_mat);
         lst_drawable->at(i)->set_projection_matrix(projection_mat);
         lst_drawable->at(i)->set_shader(_pid_normal);

         lst_drawable->at(i)->draw();
         lst_drawable->at(i)->set_shader(shader_before);
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   GLuint get_texture_id(){
      return texture_id;
   }

protected:
   GLuint texture_id;
   unsigned int tex_width;
   unsigned int tex_height;
   GLuint fbo;
   GLuint _pid_normal;
   GLuint _depth_rb;

   Camera *camera;
};

#endif
