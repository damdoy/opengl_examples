#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "../texture.h"

class Plane{
public:

   void init(GLuint pid){
      _pid = pid;
      if(_pid == 0) exit(-1);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      GLfloat vpoint[] = {
         -1.0f, 0.0f, 1.0f,
         1.0f, 0.0f, 1.0f,
         1.0f, 0.0f, -1.0f,
         -1.0f,  0.0f, -1.0f,
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

      //texture coord definition
      const GLfloat vtexcoord[] = { /*V1*/ 0.0f, 1.0f,
                                    /*V2*/ 1.0f, 1.0f,
                                    /*V3*/ 1.0f, 0.0f,
                                    /*V4*/ 0.0f, 0.0f};

      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "vtexcoord");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

      glBindVertexArray(0);
      glUseProgram(0);
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
      GLuint tex_id = glGetUniformLocation(_pid, "tex");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex);

      //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo);
      glDeleteBuffers(1, &_vbo_tex);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

private:
   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _pid;
   GLuint _tex;
};
