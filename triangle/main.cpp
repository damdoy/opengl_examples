#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>

const GLfloat vpoint[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,};

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(512, 512, "triangle", NULL, NULL);

   if( !window ){
      std::cout << "failed to open window" << std::endl;
      return -1;
   }

   glfwMakeContextCurrent(window);

   glewExperimental = GL_TRUE;
   int init_result = glewInit();
   if(init_result != GLEW_NO_ERROR){
      std::cout << "glew error : " << init_result <<"\n";
      return -1;
   }

   init();

   //main display loop
   while(glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window)){
      display();
      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   cleanup();

   return 0;
}

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
      char log[255];
      glGetShaderInfoLog(fshader_id, 255, NULL, log);
      std::cout << "error at vertex shader compilation" << log << std::endl;
      glDeleteShader(vshader_id);
      return 0;
   }

   glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &success);
   if(!success) {
      char log[255];
      glGetShaderInfoLog(fshader_id, 255, NULL, log);
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
      char log[255];
      glGetShaderInfoLog(program_id, 255, NULL, log);
      std::cout << "error in linking the shaders : " << log << std::endl;
      return 0;
   }

   glDeleteShader(vshader_id);
   glDeleteShader(fshader_id);

   return program_id;
}

void init(){
   glClearColor(0.0, 0.0, 0.0, 1.0);

   GLuint shader_id = load_shaders("vshader.glsl", "fshader.glsl");

   if(!shader_id){
      std::cout << "error loading shader\n";
   }

   glUseProgram(shader_id);

   //init VAO
   GLuint vao_id;
   glGenVertexArrays(1, &vao_id);
   glBindVertexArray(vao_id);

   //init VBO for triangle, list of vertices
   GLuint vbo_id;
   glGenBuffers(1, &vbo_id);
   glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

   //assign triangle point as vpoint to the shader
   GLuint vpoint_id = glGetAttribLocation(shader_id, "vpoint");
   glEnableVertexAttribArray(vpoint_id);
   glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
   glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void display(){
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawArrays(GL_TRIANGLES, 0, 3);
}

void cleanup(){
}
