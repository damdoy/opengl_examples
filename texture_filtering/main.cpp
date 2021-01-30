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
#include "texture.h"
#include "texture_checkers.h"

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

Texture_checkers tex_checkers;
Plane plane;
Transform plane_transf;

std::vector<Drawable*> lst_drawable;

GLfloat camera_position[3];
GLfloat camera_direction[3];

const int win_width = 1280;
const int win_height = 720;

// const int win_width = 400;
// const int win_height = 400;

float time_measured;
bool rotate_plane_active = true;

unsigned int effect_select;

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "texture_filtering", NULL, NULL);

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

      if(glfwGetKey(window, 'X') == GLFW_PRESS){
         plane.set_texture_filtering(GL_NEAREST);
      }
      if(glfwGetKey(window, 'C') == GLFW_PRESS){
         plane.set_texture_filtering(GL_LINEAR);
      }
      if(glfwGetKey(window, 'V') == GLFW_PRESS){
         plane.set_texture_filtering(GL_LINEAR_MIPMAP_NEAREST);
      }
      if(glfwGetKey(window, 'B') == GLFW_PRESS){
         plane.set_texture_filtering(GL_LINEAR_MIPMAP_LINEAR);
      }
      if(glfwGetKey(window, 'N') == GLFW_PRESS){
         rotate_plane_active = false;
      }
      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         rotate_plane_active = true;
      }

      if(glfwGetKey(window, '1') == GLFW_PRESS){
         plane.set_texture_anisotropic(1.0f);
      }
      if(glfwGetKey(window, '2') == GLFW_PRESS){
         plane.set_texture_anisotropic(2.0f);
      }
      if(glfwGetKey(window, '3') == GLFW_PRESS){
         plane.set_texture_anisotropic(4.0f);
      }
      if(glfwGetKey(window, '4') == GLFW_PRESS){
         plane.set_texture_anisotropic(8.0f);
      }

      display();
      glfwSwapBuffers(window);

      prev_time = current_time;
   }

   cleanup();

   return 0;
}

void init(){

   time_measured = 0.0f;
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
   cam_free.set_speed(20.0f);

   GLuint plane_pid = load_shaders("plane_vshader.glsl", "plane_fshader.glsl");
   plane.init(plane_pid);
   //creates checkers texture of size 64x64 with 8 squares by 8 squares
   tex_checkers.init(64, 8);
   plane.set_texture(&tex_checkers);

   plane.set_texture_scale(32, 32);

   plane_transf.scale(512, 1, 512); //will be repeated a lot of time on a massive plane texture

   lst_drawable.push_back(&plane);
}

void display(){

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   static float previous_time = glfwGetTime();

   //rotate the plane to make the aliasing obvious
   if(rotate_plane_active){
      float angle = (previous_time-glfwGetTime())*0.3;
      previous_time = glfwGetTime();
      plane_transf.rotate(0.0f, 1.0f, 0.0f, angle);
   }


   plane.set_MVP_matrices(plane_transf.get_matrix(), cam->getMatrix(), projection_mat);

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
