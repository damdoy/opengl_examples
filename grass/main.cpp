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
#include <IL/il.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective

#include "camera.h"
#include "camera_free.h"
#include "shader_helper.h"
#include "_plane/plane.h"
#include "_plane/plane_sine.h"
#include "_grass/grass_manager.h"
#include "_grass/grass_manager_geom.h"
#include "transform.h"
#include "noise_generator.hpp"
#include "texture.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer framebuffer;

Plane_sine plane_test;
Grass_manager grass_manager;
Grass_manager_geom grass_manager_geom;
Quad_screen quad_screen;
Transform plane_test_transf;
glm::mat4x4 projection_mat;

std::vector<Drawable*> lst_drawable;

GLfloat camera_position[3];
GLuint light_mode = 0;

const int win_width = 1280;
const int win_height = 720;

// const int win_width = 400;
// const int win_height = 400;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "grass", NULL, NULL);

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

      // static clock_t begin_time = clock();
      static int image_count = 0;
      static auto begin_time = std::chrono::system_clock::now();

      display();
      image_count++;

      auto end_time = std::chrono::system_clock::now();
      std::chrono::duration<double ,std::milli> diff_ms = end_time-begin_time;

      //displays the framerate every seconds (1000ms)
      if(diff_ms.count() > 1000){
         printf("fps: %f\n", image_count/(diff_ms.count()/1000.0f) );
         begin_time = std::chrono::system_clock::now();
         image_count = 0;
      }

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

   ilInit();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);

   plane_test.init();

   plane_test_transf.translate(0.0f, -2.0f, 0.0f);
   plane_test_transf.scale(50.0f, 3.0f, 50.0f);

   plane_test.set_model_matrix(plane_test_transf.get_matrix());

   lst_drawable.push_back(&plane_test);

   grass_manager.init(&plane_test);
   grass_manager_geom.init(&plane_test);

   GLuint tex_fb = framebuffer.init(win_width, win_height);

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(32.0f, 16.0f, -32.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();
}

void display(){

   cam->get_position(camera_position);

   framebuffer.bind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   plane_test.set_MVP_matrices(plane_test_transf.get_matrix(), cam->getMatrix(), projection_mat);

   /////////////// grass manager with instanced drawing
   grass_manager.set_camera_pos(camera_position);
   grass_manager.set_view_matrix(cam->getMatrix());
   grass_manager.set_projection_matrix(projection_mat);
   grass_manager.draw();
   /////////////// grass manager with geometry shaders
   // grass_manager_geom.set_camera_pos(camera_position);
   // grass_manager_geom.set_view_matrix(cam->getMatrix());
   // grass_manager_geom.set_projection_matrix(projection_mat);
   // grass_manager_geom.draw();

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_camera_pos(camera_position);

      lst_drawable[i]->set_window_dim(win_width, win_height);
      lst_drawable[i]->draw();
   }

   framebuffer.unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   quad_screen.draw(effect_select);
}

void cleanup(){
}
