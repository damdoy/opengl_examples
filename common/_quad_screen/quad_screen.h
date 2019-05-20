#pragma once

// two triangles that should take all the screen space to draw the framebuffer
class Quad_screen{

public:
   //texture is the framebuffer
   void init(GLuint texture, unsigned int screen_width, unsigned int screen_height, GLuint pid){

      this->_width = screen_width;
      this->_height = screen_height;

      _pid = pid;
      if(_pid == 0) exit(-1);
      glUseProgram(_pid);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      const GLfloat vpoint[] = { -1.0f, -1.0f, 0.0f,
                        +1.0f, -1.0f, 0.0f,
                        -1.0f, +1.0f, 0.0f,
                        +1.0f, +1.0f, 0.0f };

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid, "vpoint");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      const GLfloat vtexcoord[] = { /*V1*/ 0.0f, 0.0f,
                                 /*V2*/ 1.0f, 0.0f,
                                 /*V3*/ 0.0f, 1.0f,
                                 /*V4*/ 1.0f, 1.0f};

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "vtexcoord");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, NULL);

      this->_tex = texture; //frambuffer
      glBindTexture(GL_TEXTURE_2D, _tex);
      GLuint tex_id = glGetUniformLocation(_pid, "tex");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

      glBindVertexArray(0);
      glUseProgram(0);
  }

   void cleanup(){
     /// TODO
   }

   void draw(unsigned int effect_select){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glUniform1ui( glGetUniformLocation(_pid, "effect_select"), effect_select);
      glUniform1f( glGetUniformLocation(_pid, "tex_width"), _width);
      glUniform1f( glGetUniformLocation(_pid, "tex_height"), _height);
      
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glBindVertexArray(0);
      glUseProgram(0);
   }

protected:
     GLuint _vao;
     GLuint _pid;
     GLuint _vbo;
     GLuint _tex;
     unsigned int _width;
     unsigned int _height;
};
