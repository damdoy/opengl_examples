/**
Small triange test in opengl to test minimal functionalities
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective

#include "camera.h"
#include "camera_free.h"
#include "shader_helper.h"
#include "_plane/plane.h"
#include "_cube/cube.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer framebuffer;

Cube cube;
Plane plane;
Quad_screen quad_screen;
Transform plane_transf;
Transform cube_transf;
glm::mat4x4 projection_mat;

GLfloat camera_position[3];

const int win_width = 1280;
const int win_height = 720;

float time_measured;

unsigned int effect_select;

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "framebuffer", NULL, NULL);

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

      if(glfwGetKey(window, '0') == GLFW_PRESS){
         effect_select = 0; //normal
      }
      else if(glfwGetKey(window, '1') == GLFW_PRESS){
         effect_select = 1; //inverted colours
      }
      else if(glfwGetKey(window, '2') == GLFW_PRESS){
         effect_select = 2; //small gaussian blur
      }
      else if(glfwGetKey(window, '3') == GLFW_PRESS){
         effect_select = 3; //medium gaussian blur
      }
      else if(glfwGetKey(window, '4') == GLFW_PRESS){
         effect_select = 4; //big gaussian blur
      }
      else if(glfwGetKey(window, '5') == GLFW_PRESS){
         effect_select = 5; //edge detection
      }
      else if(glfwGetKey(window, '6') == GLFW_PRESS){
         effect_select = 6; //broken glass
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

   glClearColor(1.0, 1.0, 1.0, 1.0);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.01f, 1000.0f);

   plane_transf.scale(2.0f, 1.0f, 2.0f);
   plane_transf.translate(0.0f, -1.0f, 0.0f);

   GLuint tex_fb = framebuffer.init(win_width, win_height);

   GLuint plane_pid = load_shaders("plane_vshader.glsl", "plane_fshader.glsl");
   plane.init(plane_pid);
   GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");
   cube.init(cube_pid);
   GLuint quad_pid = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, quad_pid);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam_free.lookAt(3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam = &cam_free;
   cam_free.update_pos();
}

void display(){
   framebuffer.bind();
   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   cam->get_position(camera_position);

   cube.draw(cube_transf.get_matrix(), cam->getMatrix(), projection_mat);
   plane.draw(plane_transf.get_matrix(), cam->getMatrix(), projection_mat);
   framebuffer.unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   quad_screen.draw(effect_select);

   if(time_measured != 0.0f){
      float diff = glfwGetTime()-time_measured;
      cube_transf.rotate(0.0f, 1.0f, 0.0f, diff);
   }

   time_measured = glfwGetTime();

}

void cleanup(){
}
