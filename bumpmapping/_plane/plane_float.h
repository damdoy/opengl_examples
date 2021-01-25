#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_helper.h"
#include "../texture_float.h"
#include "_plane/plane.h"

//plane with floating texture
class Plane_float : public Plane{
public:

   void set_texture(const Texture_float *tex){
      if(_tex != 0){
         //delete current texture
         glDeleteTextures(1, &_tex);
         _tex = 0;
      }

      //texture data definition
      glGenTextures(1, &_tex);
      glBindTexture(GL_TEXTURE_2D, _tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex->get_width(), tex->get_height(), 0, GL_RED, GL_FLOAT, tex->get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      GLuint tex_id = glGetUniformLocation(_pid, "tex");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection, GLfloat light_position[3], GLfloat camera_position[3]){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, camera_position);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex);

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

private:
   GLuint _tex;
};
