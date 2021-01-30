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
#include <vector>

#include "camera.h"
#include "camera_free.h"
#include "shader_helper.h"
#include "_cube/cube.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"
#include "_trees/tree.h"

void init();
void display(float time_delta);
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer *framebuffer;

Cube cube_base;
Tree *tree;
Quad_screen quad_screen;
Transform cube_base_transf;
Transform tree_transf;
glm::mat4x4 projection_mat;

uint trunk_pid;
uint leaves_pid;

std::vector<Drawable*> lst_drawable;

GLfloat light_position[3];
unsigned int light_mode_selected;
GLfloat camera_position[3];
GLfloat camera_direction[3];
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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "tree", NULL, NULL);

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

      if(glfwGetKey(window, 'R') == GLFW_PRESS){
         delete tree;
         lst_drawable.pop_back();
         tree = new Tree;
         tree->init(trunk_pid, leaves_pid);
         tree->load();
         lst_drawable.push_back(tree);
      }

      display(time_delta);
      glfwSwapBuffers(window);

      prev_time = current_time;
   }

   cleanup();

   return 0;
}

void init(){

   time_measured = 0.0f;
   effect_select = 0;

   glClearColor(0.3, 0.6, 1.0, 1.0); //sky

   ilInit();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);

   light_position[0] = 0.0; //x
   light_position[1] = 0.0; //up
   light_position[2] = 0.0; //z

   light_mode_selected = 3;

   GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");

   cube_base.init(cube_pid);
   cube_base.set_color(0.8f, 0.8f, 0.8f);
   lst_drawable.push_back(&cube_base);

   int glError = glGetError();
   if(glError != GL_NO_ERROR){
      printf("error ogl: %d\n", glError);
   }

   tree_transf.translate(0.0f, 0.5f, 0.0f);
   tree_transf.scale(5.0f, 5.0f, 5.0f);

   trunk_pid = load_shaders("trunk_vshader.glsl", "trunk_fshader.glsl");
   leaves_pid = load_shaders("leaves_individual_vshader.glsl", "leaves_individual_fshader.glsl");

   tree = new Tree;
   tree->init(trunk_pid, leaves_pid);
   tree->load();
   lst_drawable.push_back(tree);

   cube_base_transf.scale(20.0f, 0.5f, 20.0f);

   //framebuffers
   framebuffer = new Framebuffer();
   GLuint tex_fb = framebuffer->init(win_width, win_height, true);

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(10.0f, 10.0f, -7.0f, 0.0f, 7.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();

   //clip coord to tell shader not to draw anything over the water
   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_clip_coord(0, 1, 0, -20);
   }
}

void display(float time_delta){

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
   tree->set_MVP_matrices(tree_transf.get_matrix(), cam->getMatrix(), projection_mat);

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_light_pos(light_position);
      lst_drawable[i]->set_camera_pos(camera_position);

      lst_drawable[i]->set_camera_direction(camera_direction);

      lst_drawable[i]->set_shadow_buffer_texture_size(win_width, win_height);
      lst_drawable[i]->set_window_dim(win_width, win_height);
   }

   tree->move_leaves(time_delta);

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

   if(light_mode_selected == 0){
      light_position[0] = 10;
      light_position[1] = 15;
      light_position[2] = -10;
   }
   else if(light_mode_selected == 1){
      light_position[0] = 16.0*cos(glfwGetTime()/2);
      light_position[1] = 8;
      light_position[2] = 16.0*sin(glfwGetTime()/2);
   }
   else if(light_mode_selected == 2){
      light_position[0] = 30;
      light_position[1] = 30;
      light_position[2] = 0;
   }
   else if(light_mode_selected == 3){
      light_position[0] = 30;
      light_position[1] = 30;
      light_position[2] = 30;
   }
}

void cleanup(){
}
