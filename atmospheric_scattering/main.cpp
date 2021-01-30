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
#include "_sphere/sphere.h"
#include "transform.h"
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

Framebuffer *framebuffer;

Quad_screen quad_screen;

Sphere base_sphere;
Transform base_sphere_transf;

Sphere sky_sphere;
Transform sky_sphere_transf;

glm::mat4x4 projection_mat;

uint trunk_pid;
uint leaves_pid;

std::vector<Drawable*> lst_drawable;

unsigned int light_mode_selected;
GLfloat sun_dir[3];
GLfloat sun_col[3];
GLfloat camera_position[3];
GLfloat camera_direction[3];
GLuint light_mode = 0;
GLuint _tex_inner_integral;

const int win_width = 1280;
const int win_height = 720;

const int planet_radius = 64;
const int atm_radius = 68;

//mean height for atm (where the avg will be)
const float H_0 = 0.25;

// const int win_width = 400;
// const int win_height = 400;

float time_measured;
bool time_activate;

unsigned int effect_select;


int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "atmospheric_scattering", NULL, NULL);

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
         time_activate = true;
      }

      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         time_activate = false;
      }

      display();
      glfwSwapBuffers(window);

      prev_time = current_time;
   }

   cleanup();

   return 0;
}

void t_func(float height_norm, float angle_norm, float *v3_result){
   const int nb_integrations = 50;

   float real_height = planet_radius+height_norm*(atm_radius-planet_radius);
   float real_angle = 3.1415*angle_norm;

   float h_sq = real_height*real_height;

   //formula to fin number of time(ratio) we should multiply {sin(angle), cos(angle)} to be on the circle
   //formula from wolfram alpha => r = sqrt( (0+xsin(angle))^2+(h+xcos(angle))^2) solve for x
   //assume centre of atmosphere is 0,0
   float ratio = 1.0/2.0*(sqrt(2)*sqrt(h_sq*cos(2*real_angle)-h_sq+2*atm_radius*atm_radius)-2*real_height*cos(real_angle) );
   // float pt_on_circle[2] = {ratio*sin(real_angle), real_height+ratio*cos(real_angle)};
   float distance_to_end = sqrt(pow(ratio*sin(real_angle), 2)+pow(ratio*cos(real_angle), 2) );
   float scattered = 0.0f;

   float increment = distance_to_end/nb_integrations;
   float current_pos[2] = {0, real_height};

   for(int i = 0; i < nb_integrations; i++)
   {
      //assume centre of atmosphere is 0
      float distance_to_centre = sqrt(pow(current_pos[0]-0, 2)+pow(current_pos[1]-0, 2));
      float atm_density = (distance_to_centre-planet_radius)/(atm_radius-planet_radius);
      if(atm_density < 0){
         atm_density = 0;
      }
      if(atm_density > 1){
         atm_density = 1;
      }

      scattered += exp(-atm_density/H_0);

      //integral part => evaluate for next increment
      current_pos[0] += increment*sin(real_angle);
      current_pos[1] += increment*cos(real_angle);
   }

   scattered /= nb_integrations;

   //divide distance by a factor to avoid this out scattering to be too strong
   scattered *= distance_to_end/64;
   scattered *= 4*3.1415;

   //out scattering stronger for blue than other
   v3_result[0] = scattered*0.1; //R
   v3_result[1] = scattered*0.2; //G
   v3_result[2] = scattered*0.4; //B

   for (size_t i = 0; i < 3; i++) {
      if(v3_result[i] < 0){
         v3_result[i] = 0;
      }
      if(v3_result[i] > 8){
         v3_result[i] = 8;
      }
   }

}

void precompute_inner_integral(){
   uint size = 256;

   float *array = new float[size*size*3];
   int counter = 0;
   for (size_t i = 0; i < size; i++) {
      float altitude = float(i)/(size-1);
      for (size_t j = 0; j < size; j++) {
         float angle_norm = float(j)/(size-1);

         float result[3];
         int idx = 3*(i+j*size);

         t_func(altitude, angle_norm, result);

         array[idx+0] = result[0];
         array[idx+1] = result[1];
         array[idx+2] = result[2];
         counter+=3;
      }
   }

   glGenTextures(1, &_tex_inner_integral);
   glBindTexture(GL_TEXTURE_2D, _tex_inner_integral);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, array);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   //if not using clamp to edge, some nasty artifacts can occur on the sides of the lookup table
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   delete[] array;
}

void init(){

   time_measured = 0.0f;
   effect_select = 0;
   time_activate = true;

   glClearColor(0.0, 0.0, 0.0, 1.0); //space

   ilInit();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glEnable(GL_BLEND);
   // glDepthMask(GL_FALSE);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   // glBlendFunc(GL_SRC_ALPHA, GL_ONE);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);

   light_mode_selected = 3;

   GLuint pid_shader_sphere = load_shaders("base_sphere_vshader.glsl", "base_sphere_fshader.glsl");
   base_sphere.init(pid_shader_sphere, 128, 128);
   base_sphere_transf.scale(planet_radius, planet_radius, planet_radius);
   lst_drawable.push_back(&base_sphere);

   GLuint pid_shader_sky_sphere = load_shaders("sky_sphere_vshader.glsl", "sky_sphere_fshader.glsl");
   sky_sphere.init(pid_shader_sky_sphere, 128, 128);
   sky_sphere_transf.scale(atm_radius, atm_radius, atm_radius);
   lst_drawable.push_back(&sky_sphere);
   sky_sphere.set_invert(true);

   precompute_inner_integral();

   glUseProgram(pid_shader_sky_sphere);

   float atm_centre[3] = {0,0,0};
   glUniform3fv( glGetUniformLocation(pid_shader_sky_sphere, "atm_centre"), 1, atm_centre);
   glUniform1f( glGetUniformLocation(pid_shader_sky_sphere, "planet_radius"), planet_radius);
   glUniform1f( glGetUniformLocation(pid_shader_sky_sphere, "atm_radius"), atm_radius);
   glUniform1i(glGetUniformLocation(pid_shader_sky_sphere, "tex_t_func"), 0 /*GL_TEXTURE0*/);

   glUseProgram(pid_shader_sphere);

   glUniform3fv( glGetUniformLocation(pid_shader_sphere, "atm_centre"), 1, atm_centre);
   glUniform1f( glGetUniformLocation(pid_shader_sphere, "planet_radius"), planet_radius);
   glUniform1f( glGetUniformLocation(pid_shader_sphere, "atm_radius"), atm_radius);
   glUniform1i(glGetUniformLocation(pid_shader_sphere, "tex_t_func"), 0 /*GL_TEXTURE0*/);

   glUseProgram(0);

   int glError = glGetError();
   if(glError != GL_NO_ERROR){
      printf("error ogl: %d\n", glError);
   }

   sun_dir[0] = 1/sqrt(2);
   sun_dir[1] = 0;
   sun_dir[2] = 1/sqrt(2);

   //framebuffers
   framebuffer = new Framebuffer();
   GLuint tex_fb = framebuffer->init(win_width, win_height, true);

   GLuint pid_quad_screen = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, pid_quad_screen);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(68.0f, 0.0f, 68.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);
   cam_free.set_speed(15.0f);

   cam = &cam_free;
   cam_free.update_pos();

   //clip coord to tell shader not to draw anything over the water
   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_clip_coord(0, 1, 0, -20);
   }

   sun_col[0] = 1.0;
   sun_col[1] = 1.0;
   sun_col[2] = 0.8;
}

void display(){

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   base_sphere.set_MVP_matrices(base_sphere_transf.get_matrix(), cam->getMatrix(), projection_mat);
   sky_sphere.set_MVP_matrices(sky_sphere_transf.get_matrix(), cam->getMatrix(), projection_mat);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, _tex_inner_integral);

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


   if(time_activate){
      //make the sun go round the planet
      sun_dir[0] = cos(glfwGetTime()/4);
      sun_dir[1] = 0;
      sun_dir[2] = sin(glfwGetTime()/4);
   }
}

void cleanup(){
}
