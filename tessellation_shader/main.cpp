#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
// #define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <IL/il.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective
#include <vector>

#include "camera.h"
#include "camera_free.h"
#include "shader_helper.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"
#include "_plane/plane.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer *framebuffer;

Quad_screen quad_screen;

glm::mat4x4 projection_mat;

Plane plane[9];
Transform plane_transf[9];

std::vector<Drawable*> lst_drawable;

GLfloat camera_position[3];
GLfloat camera_direction[3];

const int win_width = 1280;
const int win_height = 720;

// const int win_width = 400;
// const int win_height = 400;

unsigned int effect_select;
bool activate_wireframe = true;
bool fixed_tessellation_level_active = false;
uint fixed_tessellation_level = 1;

GLuint plane_pid;

GLuint compile_shader(const char *shader_filename, GLenum shader_type, const char *shader_description)
{
   GLuint shader_id = load_shader(shader_filename, shader_type);

   GLint success = GL_FALSE;

   glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
   if(!success) {
      char log[2048];
      glGetShaderInfoLog(shader_id, 2048, NULL, log);
      std::cout << "error at " << shader_description << " compilation : " << log << std::endl;
      glDeleteShader(shader_id);
      return 0;
   }

   return shader_id;
}

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "tessellation_shader", NULL, NULL);

   if( !window ){
      std::cout << "failed to open window" << std::endl;
      return -1;
   }

   glfwMakeContextCurrent(window);
   glewExperimental = GL_TRUE;
   if(glewInit() != GLEW_NO_ERROR){
      std::cout << "glew error\n";
      return -1;
   }

   init();

   while(glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window)){
      glfwPollEvents();

      static float prev_time = 0;

      float current_time = glfwGetTime();
      float time_delta = current_time-prev_time;

      if(glfwGetKey(window, 'S') == GLFW_PRESS){
         cam->input_handling('S', time_delta);
      }
      if(glfwGetKey(window, 'A') == GLFW_PRESS){
         cam->input_handling('A', time_delta);
      }
      if(glfwGetKey(window, 'W') == GLFW_PRESS){
         cam->input_handling('W', time_delta);
      }
      if(glfwGetKey(window, 'D') == GLFW_PRESS){
         cam->input_handling('D', time_delta);
      }

      if(glfwGetKey(window, 'L') == GLFW_PRESS){
         cam->input_handling('L', time_delta);
      }
      if(glfwGetKey(window, 'J') == GLFW_PRESS){
         cam->input_handling('J', time_delta);
      }
      if(glfwGetKey(window, 'K') == GLFW_PRESS){
         cam->input_handling('K', time_delta);
      }
      if(glfwGetKey(window, 'I') == GLFW_PRESS){
         cam->input_handling('I', time_delta);
      }

      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         activate_wireframe = true;
      }
      if(glfwGetKey(window, 'N') == GLFW_PRESS){
         activate_wireframe = false;
      }

      if(glfwGetKey(window, '0') == GLFW_PRESS){
         fixed_tessellation_level_active = false;
      }

      for (size_t i = 1; i <= 9; i++) {
         if(glfwGetKey(window, '0'+i) == GLFW_PRESS){
            fixed_tessellation_level_active = true;
            fixed_tessellation_level = i;
         }
      }

      display();
      glfwSwapBuffers(window);

      prev_time = current_time;
   }

   cleanup();

   return 0;
}

void init(){

   effect_select = 0;

   glClearColor(0.4, 0.7, 0.9, 1.0); //sky

   ilInit();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);

   int glError = glGetError();
   if(glError != GL_NO_ERROR){
      printf("error ogl: %d\n", glError);
   }

   //framebuffers
   framebuffer = new Framebuffer();
   GLuint tex_fb = framebuffer->init(win_width, win_height, true);

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(10.0f, 15.0f, -7.0f, 0.0f, 6.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();
   cam_free.set_speed(10.0f);

   //patches of 3 verices for tess shader => triangle
   glPatchParameteri(GL_PATCH_VERTICES, 3);

   //compile the shader here
   plane_pid = glCreateProgram();
   GLuint vs_pid = compile_shader("plane_vshader.glsl", GL_VERTEX_SHADER, "vertex shader");
   GLuint fs_pid = compile_shader("plane_fshader.glsl", GL_FRAGMENT_SHADER, "fragment shader");
   GLuint tcs_pid = compile_shader("plane_tcshader.glsl", GL_TESS_CONTROL_SHADER, "TC shader");
   GLuint tes_pid = compile_shader("plane_teshader.glsl", GL_TESS_EVALUATION_SHADER, "TE shader");
   glAttachShader(plane_pid, vs_pid);
   glAttachShader(plane_pid, fs_pid);
   glAttachShader(plane_pid, tcs_pid);
   glAttachShader(plane_pid, tes_pid);
   glLinkProgram(plane_pid);

   GLint success = GL_FALSE;
   glGetProgramiv(plane_pid, GL_LINK_STATUS, &success);
   if(!success) {
      char log[2048];
      int size_returned = 0;
      glGetProgramInfoLog(plane_pid, 2048, &size_returned, log);
      std::cout << "error in linking the shaders : " << log << std::endl;
   }

   glDeleteShader(vs_pid);
   glDeleteShader(fs_pid);
   glDeleteShader(tcs_pid);
   glDeleteShader(tes_pid);

   //setup multiple planes
   for (size_t i = 0; i < 9; i++) {
      plane[i].init(plane_pid);
      plane[i].set_tessellation_shader_active(true);

      plane_transf[i].scale(4, 4, 4);

      lst_drawable.push_back(&plane[i]);
   }

   int idx = 0;
   for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
         plane_transf[idx].translate(i*2, 1, j*2);
         idx++;
      }
   }

}

void display(){

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   if(activate_wireframe){
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glClearColor(1.0, 1.0, 1.0, 1.0);
   }
   else{
      glClearColor(0.6, 0.8, 1.0, 1.0);
   }

   for (size_t i = 0; i < 9; i++) {
      plane[i].set_MVP_matrices(plane_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
   }

   glUseProgram(plane_pid);

   //will be used by the Tessellation control shader
   glUniform1i( glGetUniformLocation(plane_pid, "fixed_tessellation_level_active"), fixed_tessellation_level_active);
   glUniform1i( glGetUniformLocation(plane_pid, "fixed_tessellation_level"), fixed_tessellation_level);

   glUseProgram(0);

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_camera_pos(camera_position);

      lst_drawable[i]->set_camera_direction(camera_direction);

      lst_drawable[i]->set_shadow_buffer_texture_size(win_width, win_height);
      lst_drawable[i]->set_window_dim(win_width, win_height);
   }

   framebuffer->bind(); //print to the real, color, multisample frambuffer

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->draw();
   }

   framebuffer->unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   quad_screen.draw(effect_select);
}

void cleanup(){
}
