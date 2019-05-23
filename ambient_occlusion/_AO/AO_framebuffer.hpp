#ifndef AO_FRAMEBUFFER_HPP
#define AO_FRAMEBUFFER_HPP

//post processing for the ambiant occlusion
//only needs the normal map framebuffer and the depth buffer and output the AO buffer
class AO_framebuffer{
public:
   void init(unsigned tex_width, unsigned tex_height){

      camera = NULL;

      _pid_AO = load_shaders("AO_vshader.glsl", "AO_fshader.glsl");

      this->tex_width = tex_width;
      this->tex_height = tex_height;

      glUseProgram(_pid_AO);

      // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
      glGenFramebuffers(1, &_fbo_out);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_out);

      // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
      glGenTextures(1, &texture_out_id);
      glBindTexture(GL_TEXTURE_2D, texture_out_id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_width, tex_height, 0, GL_RGB, GL_FLOAT, 0);

      glBindTexture(GL_TEXTURE_2D, 0);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_out_id, 0);

      glGenRenderbuffers(1, &_depth_rb);
      glBindRenderbuffer(GL_RENDERBUFFER, _depth_rb);

      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, tex_width, tex_height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_rb);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "!!!ERROR: depth Framebuffer not OK :(" << std::endl;

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      const GLfloat vpoint[] = { -1.0f, -1.0f, 0.0f,
                        +1.0f, -1.0f, 0.0f,
                        -1.0f, +1.0f, 0.0f,
                        +1.0f, +1.0f, 0.0f };

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid_AO, "vpoint");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      const GLfloat vtexcoord[] = { /*V1*/ 0.0f, 0.0f,
                                 /*V2*/ 1.0f, 0.0f,
                                 /*V3*/ 0.0f, 1.0f,
                                 /*V4*/ 1.0f, 1.0f};

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid_AO, "vtexcoord");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, NULL);

      glUseProgram(0);
      glUseProgram(0);
   }

   void set_texture_depth_buffer(GLuint depth_buffer_tex){
      this->_tex_depth_buffer = depth_buffer_tex;
   }

   void set_texture_normal_buffer(GLuint normal_buffer_tex){
      this->_tex_normal_map = normal_buffer_tex;
   }

   GLuint get_texture_id(){
      return texture_out_id;
   }

   void set_camera(Camera *cam){
      this->camera = cam;
   }

   void set_AO_effect(GLuint effect){
      this->_AO_effect = effect;
   }

   void draw_fb(){
      assert(camera != NULL);
      glUseProgram(_pid_AO);
      glBindVertexArray(_vao);

      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_out);

      glViewport(0, 0, tex_width, tex_height);

      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      glUniform1f( glGetUniformLocation(_pid_AO, "tex_width"), tex_width);
      glUniform1f( glGetUniformLocation(_pid_AO, "tex_height"), tex_height);
      glUniform1ui( glGetUniformLocation(_pid_AO, "AO_effect"), _AO_effect);

      glUniformMatrix4fv( glGetUniformLocation(_pid_AO, "view"), 1, GL_FALSE, glm::value_ptr(camera->getMatrix()));
      glUniformMatrix4fv( glGetUniformLocation(_pid_AO, "projection"), 1, GL_FALSE, glm::value_ptr(camera->get_perspective_mat()));

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_normal_map);
      GLuint tex_id = glGetUniformLocation(_pid_AO, "tex_normal_map");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_depth_buffer);
      tex_id = glGetUniformLocation(_pid_AO, "tex_depth_buffer");
      glUniform1i(tex_id, 1 /*GL_TEXTURE0*/);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
      glUseProgram(0);
   }

protected:
   GLuint texture_out_id;
   unsigned int tex_width;
   unsigned int tex_height;
   GLuint _fbo_out;
   GLuint _pid_AO;
   GLuint _depth_rb;

   GLuint _tex_normal_map;
   GLuint _tex_depth_buffer;
   GLuint _AO_effect;

   GLuint _vao;
   GLuint _vbo;

   Camera *camera;
};

#endif
