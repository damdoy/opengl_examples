#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"

class Cube{
public:
   void init(GLuint pid){
      _pid = pid;
      if(_pid == 0) exit(-1);

      glUseProgram(_pid);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      GLfloat position[] = {-1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         -1.0f,  1.0f,  1.0f,
         -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         -1.0f,  1.0f, -1.0f};

      GLuint index[] = {0, 1, 2,//front
         0, 2, 3,
         1, 5, 6,//right
         1, 6, 2,
         5, 4, 7,//back
         5, 7, 6,
         4, 0, 3,//left
         4, 3, 7,
         3, 2, 6,//top
         3, 6, 7,
         1, 0, 4,//bottom
         1, 4, 5};


      glGenBuffers(1, &_vbo_pos);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
      glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      _num_indices = sizeof(index)/sizeof(GLuint);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

      glBindVertexArray(0);
   }

   void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      //load the matrices to the shader as uniforms
      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

      glDrawElements(GL_TRIANGLES, _num_indices, GL_UNSIGNED_INT, 0);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo_idx);
      glDeleteBuffers(1, &_vbo_pos);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

private:
   GLuint _vao;
   GLuint _vbo_pos;
   GLuint _vbo_idx;
   GLuint _pid;
   int _num_indices;
};
