#pragma once

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

GLuint load_shader(const char *path, GLenum shader_type){
   std::string shader_code;

   std::ifstream file_stream(path, std::ios::in);
   if(file_stream.is_open()) {
      shader_code = std::string(std::istreambuf_iterator<char>(file_stream),
                                     std::istreambuf_iterator<char>());
      file_stream.close();
   }
   else{
      std::cout << "could not open " << path << std::endl;
      return 0;
   }

   const char *c_str_shader_code = shader_code.c_str();

   GLuint shader = glCreateShader(shader_type);
   glShaderSource(shader, 1, &c_str_shader_code , NULL);
   glCompileShader(shader);
   return shader;
}

GLuint load_shaders(const char *vshader, const char *fshader){
   GLuint vshader_id = load_shader(vshader, GL_VERTEX_SHADER);
   GLuint fshader_id = load_shader(fshader, GL_FRAGMENT_SHADER);
   GLint success = GL_FALSE;

   glGetShaderiv(vshader_id, GL_COMPILE_STATUS, &success);
   if(!success) {
      char log[2048];
      int size_returned = 0;
      glGetShaderInfoLog(vshader_id, 2048, &size_returned, log);
      std::cout << "error at vertex shader compilation : " << log << std::endl;
      glDeleteShader(vshader_id);
      return 0;
   }

   glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &success);
   if(!success) {
      char log[2048];
      glGetShaderInfoLog(fshader_id, 2048, NULL, log);
      std::cout << "error at fragment shader compilation : " << log << std::endl;
      glDeleteShader(vshader_id);
      glDeleteShader(fshader_id);
      return 0;
   }

   GLuint program_id = glCreateProgram();
   glAttachShader(program_id, vshader_id);
   glAttachShader(program_id, fshader_id);
   glLinkProgram(program_id);

   glGetProgramiv(program_id, GL_LINK_STATUS, &success);
   if(!success) {
      char log[2048];
      int size_returned = 0;
      glGetProgramInfoLog(program_id, 2048, &size_returned, log);
      std::cout << "error in linking the shaders : " << log << std::endl;
      return 0;
   }

   glDeleteShader(vshader_id);
   glDeleteShader(fshader_id);

   return program_id;
}

//pass the text of the shader as parameter
GLuint load_shaders_text(const char *vshader_text, const char *fshader_text){
   GLuint vshader_id = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vshader_id, 1, &vshader_text , NULL);
   glCompileShader(vshader_id);
   GLuint fshader_id = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fshader_id, 1, &fshader_text , NULL);
   glCompileShader(fshader_id);
   GLint success = GL_FALSE;

   glGetShaderiv(vshader_id, GL_COMPILE_STATUS, &success);
   if(!success) {
      char log[2048];
      glGetShaderInfoLog(fshader_id, 2048, NULL, log);
      std::cout << "error at vertex shader compilation : " << log << std::endl;
      glDeleteShader(vshader_id);
      return 0;
   }

   glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &success);
   if(!success) {
      char log[2048];
      glGetShaderInfoLog(fshader_id, 2048, NULL, log);
      std::cout << "error at fragment shader compilation : " << log << std::endl;
      glDeleteShader(vshader_id);
      glDeleteShader(fshader_id);
      return 0;
   }

   GLuint program_id = glCreateProgram();
   glAttachShader(program_id, vshader_id);
   glAttachShader(program_id, fshader_id);
   glLinkProgram(program_id);

   glGetProgramiv(program_id, GL_LINK_STATUS, &success);
   if(!success) {
      char log[2048];
      glGetShaderInfoLog(program_id, 2048, NULL, log);
      std::cout << "error in linking the shaders : " << log << std::endl;
      return 0;
   }

   glDeleteShader(vshader_id);
   glDeleteShader(fshader_id);

   return program_id;
}
