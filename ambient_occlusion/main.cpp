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
#include "_cube/cube.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"
#include "depth_framebuffer.hpp"
#include "_AO/normal_framebuffer.hpp"
#include "_AO/AO_framebuffer.hpp"

#define DEPTH_TEXTURE_SIZE 1024

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer framebuffer;

#define NB_CUBE_SIDE 5

Cube cube[NB_CUBE_SIDE*NB_CUBE_SIDE];
Transform cube_transf[NB_CUBE_SIDE*NB_CUBE_SIDE];
Cube cube_base;
Quad_screen quad_screen;
Transform cube_base_transf;
glm::mat4x4 projection_mat;

Depth_framebuffer depth_framebuffer;
Normal_framebuffer normal_framebuffer;
AO_framebuffer ao_framebuffer;

std::vector<Drawable*> lst_drawable;

GLfloat light_position[3];
GLfloat camera_position[3];

unsigned int shadow_mapping_effect = 0;
unsigned int AO_effect = 0;

const unsigned int win_width = 1280;
const unsigned int win_height = 720;

// const int win_width = 400;
// const int win_height = 400;

unsigned int effect_select;

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "ambient_occlusion", NULL, NULL);

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

      if(glfwGetKey(window, 'E') == GLFW_PRESS){
         //shadow_mapping_effect = 0;
         AO_effect = 0;
      }
      if(glfwGetKey(window, 'R') == GLFW_PRESS){
         //shadow_mapping_effect = 1;
         AO_effect = 1;
      }
      if(glfwGetKey(window, 'T') == GLFW_PRESS){
         //shadow_mapping_effect = 2;
         AO_effect = 2;
      }
      if(glfwGetKey(window, 'Y') == GLFW_PRESS){
         //shadow_mapping_effect = 3;
         AO_effect = 3;
      }
      if(glfwGetKey(window, 'U') == GLFW_PRESS){
         //shadow_mapping_effect = 4;
         AO_effect = 4;
      }
      if(glfwGetKey(window, 'F') == GLFW_PRESS){
         //shadow_mapping_effect = 5;
         AO_effect = 5;
      }
      if(glfwGetKey(window, 'G') == GLFW_PRESS){
         //shadow_mapping_effect = 6;
         AO_effect = 6;
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

   glClearColor(1.0, 1.0, 1.0, 1.0);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);

   light_position[0] = 10.0; //x
   light_position[1] = 10.0; //up
   light_position[2] = 10.0; //z

   //init array of cubes
   GLuint pid_cube = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");
   for (size_t j = 0; j < NB_CUBE_SIDE; j++) {
      for (size_t i = 0; i < NB_CUBE_SIDE; i++) {
         int idx = j*NB_CUBE_SIDE+i;
         int diff_i = i-(NB_CUBE_SIDE)/2;
         int diff_j = j-(NB_CUBE_SIDE)/2;
         cube[idx].init(pid_cube);
         cube[idx].set_color(float(i)/NB_CUBE_SIDE, 1.0, float(j)/NB_CUBE_SIDE);
         cube_transf[idx].scale(1.0f, 1.0f/(sqrt(diff_i*diff_i+diff_j*diff_j)+3.0f)*20.0f, 1.0f);
         cube_transf[idx].translate(diff_i*2.0f, 0.0f, diff_j*2.0f);
         cube_transf[idx].scale(0.9f, 1.0f, 0.9f); //make them with little gaps between to display AO

         lst_drawable.push_back(&cube[idx]);
      }
   }
   cube_base.init(pid_cube);
   cube_base.set_color(1.0f, 1.0f, 1.0f);

   lst_drawable.push_back(&cube_base);

   depth_framebuffer.init(win_width, win_height);
   depth_framebuffer.set_perspective_mat(glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f));
   normal_framebuffer.init(win_width, win_height);
   ao_framebuffer.init(win_width, win_height);

   cube_base_transf.translate(0.0f, -2.0f, 0.0f);
   cube_base_transf.scale(20.0f, 1.0f, 20.0f);

   GLuint tex_fb = framebuffer.init(win_width, win_height);

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);
   quad_screen.set_ao_texture(ao_framebuffer.get_texture_id());

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(6.0f, 6.0f, -4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();

   depth_framebuffer.set_camera(cam);
}

void display(){

   cam->get_position(camera_position);

   depth_framebuffer.set_light_pos(camera_position);
   normal_framebuffer.set_camera(cam);
   ao_framebuffer.set_camera(cam);

   //to be done before binding any fb (since it loads a fb itself)
   depth_framebuffer.draw_fb(&lst_drawable);
   normal_framebuffer.draw_fb(&lst_drawable);

   framebuffer.bind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   for (size_t i = 0; i < NB_CUBE_SIDE*NB_CUBE_SIDE; i++) {
      cube[i].set_MVP_matrices(cube_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
      cube[i].set_light_pos(light_position);
      cube[i].set_camera_pos(camera_position);
   }

   cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
   cube_base.set_light_pos(light_position);
   cube_base.set_camera_pos(camera_position);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_shadow_buffer_texture_size(win_width, win_height);
      lst_drawable[i]->set_shadow_mapping_effect(shadow_mapping_effect);
      lst_drawable[i]->set_window_dim(win_width, win_height);
      lst_drawable[i]->draw();

      lst_drawable[i]->set_shadow_buffer_texture(depth_framebuffer.get_texture_id());
      lst_drawable[i]->set_shadow_matrix(depth_framebuffer.get_shadow_mat());
   }

   framebuffer.unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   ao_framebuffer.set_AO_effect(AO_effect);
   ao_framebuffer.set_texture_depth_buffer(depth_framebuffer.get_texture_id());
   ao_framebuffer.set_texture_normal_buffer(normal_framebuffer.get_texture_id());

   ao_framebuffer.draw_fb();

   //display the various buffers on screen
   //quad_screen.load_texture(depth_framebuffer.get_texture_id());
   //quad_screen.load_texture(normal_framebuffer.get_texture_id());
   //quad_screen.load_texture(ao_framebuffer.get_texture_id());

   quad_screen.draw(effect_select);
}

void cleanup(){
}
