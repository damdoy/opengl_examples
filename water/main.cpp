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
#include "_cube/cube.h"
#include "_sphere/sphere.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"
#include "depth_framebuffer.hpp"
#include "_water/water.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer *framebuffer;
Framebuffer *framebuffer_refraction;
Framebuffer *framebuffer_reflection;
Depth_framebuffer *depth_framebuffer;

//4 sides + 1 bottom
Cube cube_base[5];
Sphere sphere_decoration[2];
Cube cube_decoration[2];
Quad_screen quad_screen;
Transform cube_base_transf[5];
Transform sphere_decoration_transf[2];
Transform cube_decoration_transf[2];
Transform water_transf;
Water water;
glm::mat4x4 projection_mat;

std::vector<Drawable*> lst_drawable;

GLfloat light_position[3];
unsigned int light_mode_selected;
GLfloat camera_position[3];
GLfloat camera_direction[3];
GLuint light_mode = 0;

unsigned int water_effect = 1;

const unsigned int win_width = 1280;
const unsigned int win_height = 720;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "water", NULL, NULL);

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

      if(glfwGetKey(window, 'V') == GLFW_PRESS){
         light_mode_selected = 0;
      }
      if(glfwGetKey(window, 'B') == GLFW_PRESS){
         light_mode_selected = 1;
      }
      if(glfwGetKey(window, 'N') == GLFW_PRESS){
         light_mode_selected = 2;
      }
      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         light_mode_selected = 3;
      }

      if(glfwGetKey(window, 'E') == GLFW_PRESS){
         water_effect = 0;
      }
      if(glfwGetKey(window, 'R') == GLFW_PRESS){
         water_effect = 1;
      }
      if(glfwGetKey(window, 'T') == GLFW_PRESS){
         water_effect = 2;
      }
      if(glfwGetKey(window, 'Y') == GLFW_PRESS){
         water_effect = 3;
      }
      //add other effects
      // if(glfwGetKey('U') == GLFW_PRESS){
      //    water_effect = 4;
      // }
      // if(glfwGetKey('F') == GLFW_PRESS){
      //    water_effect = 5;
      // }
      // if(glfwGetKey('G') == GLFW_PRESS){
      //    water_effect = 6;
      // }

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
   GLuint sphere_pid = load_shaders("sphere_vshader.glsl", "sphere_fshader.glsl");

   //sets up the bassin for water
   for (size_t i = 0; i < 5; i++) {
      cube_base[i].init(cube_pid);
      cube_base[i].set_color(1.0f-0.1f*i, 1.0f-0.1f*i, 1.0f-0.1f*i);
      lst_drawable.push_back(&cube_base[i]);
   }
   //stuff that will go around the bassin
   sphere_decoration[0].init(sphere_pid, 32, 32);
   sphere_decoration[1].init(sphere_pid, 32, 32);
   cube_decoration[0].init(cube_pid);
   cube_decoration[0].set_color(0.6f, 1.0f, 0.6f);
   cube_decoration[1].init(cube_pid);
   cube_decoration[1].set_color(1.0f, 0.6f, 0.6f);
   lst_drawable.push_back(&sphere_decoration[0]);
   lst_drawable.push_back(&sphere_decoration[1]);
   lst_drawable.push_back(&cube_decoration[0]);
   lst_drawable.push_back(&cube_decoration[1]);

   water.init();
   lst_drawable.push_back(&water);


   // position of walls for the bassin
   // 0111
   // 0  3
   // 0  3
   // 0222
   //sets up bassin
   cube_base_transf[0].translate(15.0f, 0.0f, 0.0f);
   cube_base_transf[0].scale(5.0f, 2.5f, 20.0f);
   cube_base_transf[1].translate(-5.0f, 0.0f, -15.0f);
   cube_base_transf[1].scale(15.0f, 2.5f, 5.0f);
   cube_base_transf[2].translate(-5.0f, 0.0f, 15.0f);
   cube_base_transf[2].scale(15.0f, 2.5f, 5.0f);
   cube_base_transf[3].translate(-15.0f, 0.0f, 0.0f);
   cube_base_transf[3].scale(5.0f, 2.5f, 10.0f);
   //bottom of the bassin
   cube_base_transf[4].translate(0.0f, -2.5f, 0.0f);
   cube_base_transf[4].scale(20.0f, 0.5f, 20.0f);

   sphere_decoration_transf[0].translate(0.0f, 0.0f, 4.0f);
   sphere_decoration_transf[0].scale(2.0f, 2.5f, 2.0f);
   sphere_decoration_transf[1].translate(-15.0f, 5.0f, -4.0f);
   sphere_decoration_transf[1].scale(2.5f, 2.5f, 2.5f);

   cube_decoration_transf[0].translate(0.0f, 0.0f, -4.0f);
   cube_decoration_transf[0].scale(2.5f, 2.5f, 2.5f);
   cube_decoration_transf[1].translate(-15.0f, 5.0f, 4.0f);
   cube_decoration_transf[1].scale(2.5f, 2.5f, 2.5f);

   water_transf.translate(0.0f, 2.0f, 0.0f);
   water_transf.scale(20.0f, 20.0f, 20.0f);

   //framebuffers
   framebuffer_refraction = new Framebuffer();
   framebuffer_refraction->init(win_width, win_height, false);
   framebuffer = new Framebuffer();
   framebuffer_reflection = new Framebuffer();
   framebuffer_reflection->init(win_width, win_height, false);
   depth_framebuffer = new Depth_framebuffer();
   depth_framebuffer->init(win_width, win_height);
   depth_framebuffer->set_perspective_mat(projection_mat);
   GLuint tex_fb = framebuffer->init(win_width, win_height, true);

   water.set_texture_refraction(framebuffer_refraction->get_texture());
   water.set_texture_reflection(framebuffer_reflection->get_texture());
   water.set_texture_refraction_depth(depth_framebuffer->get_texture_id());

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(6.0f, 6.0f, -4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();

   depth_framebuffer->set_camera(cam);

   //clip coord to tell shader not to draw anything over the water
   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_clip_coord(0, 1, 0, -2);
   }
}

void display(){

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   water.set_effect(water_effect);

   depth_framebuffer->set_light_pos(light_position);

   for (size_t i = 0; i < 5; i++) {
      cube_base[i].set_MVP_matrices(cube_base_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
   }
   for (size_t i = 0; i < 2; i++) {
      cube_decoration[i].set_MVP_matrices(cube_decoration_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
      sphere_decoration[i].set_MVP_matrices(sphere_decoration_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
   }

   water.set_MVP_matrices(water_transf.get_matrix(), cam->getMatrix(), projection_mat);
   water.set_time(glfwGetTime()); //time for the movement of waves

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_light_pos(light_position);
      lst_drawable[i]->set_camera_pos(camera_position);

      lst_drawable[i]->set_camera_direction(camera_direction);

      lst_drawable[i]->set_shadow_buffer_texture_size(win_width, win_height);
      lst_drawable[i]->set_window_dim(win_width, win_height);

      lst_drawable[i]->set_shadow_buffer_texture(depth_framebuffer->get_texture_id());
      lst_drawable[i]->set_shadow_matrix(depth_framebuffer->get_shadow_mat());
   }

   water.set_enabled(false); //dont display water in refraction
   //==============REFRACTION STEP
   framebuffer_refraction->bind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->draw();
   }

   framebuffer_refraction->unbind();

   depth_framebuffer->draw_fb(&lst_drawable);

   water.set_enabled(true);

   //==============REFLEXION STEP
   framebuffer_reflection->bind();
   glEnable(GL_CLIP_DISTANCE0);

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   //use the different view matrix which is from under the water
   for (size_t i = 0; i < 5; i++) {
      cube_base[i].set_MVP_matrices(cube_base_transf[i].get_matrix(), cam->get_reflection_matrix(2), projection_mat);
   }
   for (size_t i = 0; i < 2; i++) {
      cube_decoration[i].set_MVP_matrices(cube_decoration_transf[i].get_matrix(), cam->get_reflection_matrix(2), projection_mat);
      sphere_decoration[i].set_MVP_matrices(sphere_decoration_transf[i].get_matrix(), cam->get_reflection_matrix(2), projection_mat);
   }

   water.set_MVP_matrices(water_transf.get_matrix(), cam->get_reflection_matrix(2), projection_mat);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->draw();
   }

   glDisable(GL_CLIP_DISTANCE0);
   framebuffer_reflection->unbind();

   //sets back the normal view matrix
   for (size_t i = 0; i < 5; i++) {
      cube_base[i].set_MVP_matrices(cube_base_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
   }
   for (size_t i = 0; i < 2; i++) {
      cube_decoration[i].set_MVP_matrices(cube_decoration_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
      sphere_decoration[i].set_MVP_matrices(sphere_decoration_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
   }

   water.set_MVP_matrices(water_transf.get_matrix(), cam->getMatrix(), projection_mat);

   //===============FINAL STEP
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

   // display depth buffer
   //quad_screen.load_texture(depth_framebuffer.get_texture_id());

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
