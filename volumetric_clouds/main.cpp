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
#include "_sphere/sphere.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"

#include "cloud_particles_manager.hpp"

#define TOTAL_NUMBER_PARTICLES 80000

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer *framebuffer;

Cube cube_base;
Quad_screen quad_screen;
Transform cube_base_transf;

Sphere sphere;
Transform sphere_transf;

Cube cube_decoration[4];
Transform cube_decoration_transf[4];

glm::mat4x4 projection_mat;

uint trunk_pid;
uint leaves_pid;

std::vector<Drawable*> lst_drawable;

Noise_generator_3d noise_gen_3d;
Noise_generator noise_gen_2d;
Cloud_particles_manager cloud_manager;
Transform cloud_manager_transf;

float cloud_amount;
const float cloud_amount_delta = 0.01f;

GLfloat sun_dir[3];
GLfloat sun_col[3];
GLfloat camera_position[3];
GLfloat camera_direction[3];

const int win_width = 1280;
const int win_height = 720;

// const int win_width = 400;
// const int win_height = 400;

float time_measured;

unsigned int effect_select;

//TODO wind function
// void wind_func(float pos[3], float ret[3], float time){
//    ret[0] = 1;
//    ret[1] = 0;
//    ret[2] = 0;
// }

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "volumetric_clouds", NULL, NULL);

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

      if(glfwGetKey(window, 'N') == GLFW_PRESS){
         if(cloud_amount-cloud_amount_delta > 0)
            cloud_amount -= cloud_amount_delta;
      }

      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         if(cloud_amount+cloud_amount_delta < 1.0f)
            cloud_amount += cloud_amount_delta;
      }

      if(glfwGetKey(window, 'X') == GLFW_PRESS){ //day
         cloud_manager.set_light_colour(1.0, 1.0, 1.0);
         cloud_manager.set_shadow_colour(0.0, 0.0, 0.0);
         cloud_manager.set_shadow_factor(0.4f);

         glClearColor(0.4, 0.7, 0.9, 1.0); //sky

         sun_dir[0] = 0;
         sun_dir[1] = 1;
         sun_dir[2] = 0;
         cloud_amount += 0.001f; //will force a cloud recalculation
      }

      if(glfwGetKey(window, 'C') == GLFW_PRESS){ //evening
         cloud_manager.set_light_colour(0.9, 0.6, 0.3);
         cloud_manager.set_shadow_colour(0.1, 0.2, 0.3);
         cloud_manager.set_shadow_factor(0.7f);

         glClearColor(0.6, 0.6, 0.3, 1.0); //sky

         //sun a bit under the horizon (simulates sunset)
         sun_dir[0] = 5/sqrt(26);
         sun_dir[1] = -1/sqrt(26);
         sun_dir[2] = 0;
         cloud_amount += 0.001f; //will force a cloud recalculation
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

   GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");

   cube_base.init(cube_pid);
   cube_base.set_color(0.8f, 0.8f, 0.8f);
   cube_base_transf.scale(1000.0f, 0.5f, 1000.0f);
   lst_drawable.push_back(&cube_base);

   GLuint pid_shader_sphere = load_shaders("sphere_vshader.glsl", "sphere_fshader.glsl");
   sphere.init(pid_shader_sphere, 16, 16);
   sphere_transf.translate(0, 2, 0);
   sphere_transf.scale(8, 8, 8);
   lst_drawable.push_back(&sphere);

   cube_decoration[0].init(cube_pid);
   cube_decoration[0].set_color(0.5f, 0.8f, 0.8f);
   cube_decoration_transf[0].translate(40, 100, 40);
   cube_decoration_transf[0].scale(2, 100, 2);
   lst_drawable.push_back(&cube_decoration[0]);

   cube_decoration[1].init(cube_pid);
   cube_decoration[1].set_color(0.8f, 0.5f, 0.8f);
   cube_decoration_transf[1].translate(-40, 100, 40);
   cube_decoration_transf[1].scale(2, 100, 2);
   lst_drawable.push_back(&cube_decoration[1]);

   cube_decoration[2].init(cube_pid);
   cube_decoration[2].set_color(0.8f, 0.8f, 0.5f);
   cube_decoration_transf[2].translate(40, 100, -40);
   cube_decoration_transf[2].scale(2, 100, 2);
   lst_drawable.push_back(&cube_decoration[2]);

   cube_decoration[3].init(cube_pid);
   cube_decoration[3].set_color(0.5f, 0.5f, 0.8f);
   cube_decoration_transf[3].translate(-40, 100, -40);
   cube_decoration_transf[3].scale(2, 100, 2);
   lst_drawable.push_back(&cube_decoration[3]);

   int glError = glGetError();
   if(glError != GL_NO_ERROR){
      printf("error ogl: %d\n", glError);
   }

   GLuint pid_cloud_particle = load_shaders("cloud_particles_vshader.glsl", "cloud_particles_fshader.glsl");
   cloud_manager.init(TOTAL_NUMBER_PARTICLES, pid_cloud_particle);

   cloud_amount = 0.5f;

   cloud_manager.set_light_colour(1.0, 1.0, 1.0);
   cloud_manager.set_shadow_colour(0.0, 0.0, 0.0);
   cloud_manager.set_shadow_factor(0.4f);

   // sun_dir[0] = 1/sqrt(2);
   // sun_dir[1] = 1/sqrt(2);
   // sun_dir[2] = 0;
   sun_dir[0] = 0;
   sun_dir[1] = 1;
   sun_dir[2] = 0;

   cloud_manager.set_sun_dir(sun_dir);

   noise_gen_3d.setup(2, 2, 0.7f, 0.4f, NOISE_SELECT_EASE);
   cloud_manager.set_3d_noise_generator(&noise_gen_3d, 64);

   noise_gen_2d.setup(4, 4, 0.7f, 0.4f, NOISE_SELECT_PERLIN);
   cloud_manager.set_2d_noise_generator(&noise_gen_2d, 256);

   //TODO
   // cloud_manager.set_wind_func(wind_func);

   lst_drawable.push_back(&cloud_manager);

   //clouds way larger than tall
   cloud_manager_transf.translate(0.0f, 128.0f, 0.0f);
   cloud_manager_transf.scale(600.0f, 150.0f, 600.0f);

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
   cam_free.set_speed(40.0f);

   //clip coord to tell shader not to draw anything over the water
   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_clip_coord(0, 1, 0, -20);
   }

   sun_col[0] = 1.0;
   sun_col[1] = 1.0;
   sun_col[2] = 0.8;
}

void display(){

   cloud_manager.set_clouds_amount(cloud_amount);
   cloud_manager.set_time(glfwGetTime());

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
   sphere.set_MVP_matrices(sphere_transf.get_matrix(), cam->getMatrix(), projection_mat);

   for (size_t i = 0; i < 4; i++) {
      cube_decoration[i].set_MVP_matrices(cube_decoration_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
   }

   cloud_manager.set_MVP_matrices(cloud_manager_transf.get_matrix(), cam->getMatrix(), projection_mat);

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_sun_dir(sun_dir);
      lst_drawable[i]->set_sun_col(sun_col);
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
