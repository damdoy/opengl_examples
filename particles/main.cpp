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
#include "_particles_manager/particles_manager.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);
void wind_func(float pos[3], float ret[3], float time);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer *framebuffer;

Cube cube_base;
Quad_screen quad_screen;
Transform cube_base_transf;
glm::mat4x4 projection_mat;

Particles_manager snow_particles_manager;
Particles_manager fire_particles_manager;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "particles", NULL, NULL);

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

   cube_base.init(cube_pid);
   cube_base.set_color(0.8f, 0.8f, 0.8f);
   lst_drawable.push_back(&cube_base);

   //intialize the particle managers
   GLuint snow_particles_pid = load_shaders("snow_particles_vshader.glsl", "snow_particles_fshader.glsl");

   snow_particles_manager.init(20000, snow_particles_pid);
   snow_particles_manager.set_emiter_boundary(-20, 20, 29, 31, -20, 20);
   snow_particles_manager.set_life_duration_sec(2, 5);
   snow_particles_manager.set_initial_velocity(0, -30.0f/5.0f, 0, 0, 1.0f, 0); //30/5 unit per second, with +- 1.0
   snow_particles_manager.set_wind_func(wind_func);

   lst_drawable.push_back(&snow_particles_manager);

   GLuint fire_particles_pid = load_shaders("fire_particles_vshader.glsl", "fire_particles_fshader.glsl");
   fire_particles_manager.init(2000, fire_particles_pid);
   fire_particles_manager.set_emiter_boundary(-1, 1, 0, 1, -1, 1);
   fire_particles_manager.set_life_duration_sec(2, 5);
   fire_particles_manager.set_initial_velocity(0, 5.0f, 0, 0, 1.0f, 0);
   fire_particles_manager.set_wind_func(wind_func);

   lst_drawable.push_back(&fire_particles_manager);

   cube_base_transf.scale(20.0f, 0.5f, 20.0f);

   //framebuffers
   framebuffer = new Framebuffer();
   GLuint tex_fb = framebuffer->init(win_width, win_height, true);

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(6.0f, 6.0f, -4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();

   //clip coord to tell shader not to draw anything over the water
   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_clip_coord(0, 1, 0, -2);
   }
}

void display(){

   snow_particles_manager.set_time(glfwGetTime());
   fire_particles_manager.set_time(glfwGetTime());

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
   snow_particles_manager.set_view_matrix(cam->getMatrix());
   snow_particles_manager.set_projection_matrix(projection_mat);

   fire_particles_manager.set_view_matrix(cam->getMatrix());
   fire_particles_manager.set_projection_matrix(projection_mat);

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_light_pos(light_position);
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

void wind_func(float pos[3], float ret[3], float time){
   ret[0] = (sin((pos[1]+pos[2]+time)/2 ))*2;
   ret[1] = (cos((pos[0]+pos[2]+time)/2 ))*2;
   ret[2] = (sin((pos[1]+pos[2]+time)/2 ))*2;
}

void cleanup(){
}
