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
#include "_sphere/sphere.h"
#include "transform.h"
#include "_quad_screen/quad_screen.h"
#include "framebuffer.h"
#include "drawable.h"
#include "depth_framebuffer.hpp"

#define DEPTH_TEXTURE_SIZE 512

const float cam_fov_angle = 3.1415f/2.0f;
const float cam_fov_angle_shadowmap = 3.1415f/1.8f;

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer framebuffer;
GLuint quad_pid;

#define NB_CUBES 5
Cube cube[NB_CUBES];
Cube cube_base;
Sphere sphere[2];
Quad_screen quad_screen;
Transform cube_base_transf;
Transform cube_transf[5];
Transform sphere_transf[2];
glm::mat4x4 projection_mat;
glm::mat4x4 projection_mat_depth;
glm::mat4x4 projection_mat_shadowmap_depth;

Depth_framebuffer depth_framebuffer_camera;
Depth_framebuffer depth_framebuffer_light;

std::vector<Drawable*> lst_drawable;

GLfloat light_position[3];
unsigned int light_mode_selected;
GLfloat camera_position[3];
GLfloat camera_direction[3];
GLuint light_mode = 0;

bool activate_colour = true;
bool activate_heightmap = false;
unsigned int shadow_mapping_effect = 1;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "shadow_mapping", NULL, NULL);

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

      if(glfwGetKey(window, 'V') == GLFW_PRESS){ //moving light
         light_mode_selected = 0;
      }
      if(glfwGetKey(window, 'B') == GLFW_PRESS){ //fixed light
         light_mode_selected = 1;
      }
      if(glfwGetKey(window, 'N') == GLFW_PRESS){ //moving light
         light_mode_selected = 2;
      }
      if(glfwGetKey(window, 'M') == GLFW_PRESS){ //moving light
         light_mode_selected = 3;
      }

      if(glfwGetKey(window, 'E') == GLFW_PRESS){
         shadow_mapping_effect = 0;
      }
      if(glfwGetKey(window, 'R') == GLFW_PRESS){
         shadow_mapping_effect = 1;
      }
      if(glfwGetKey(window, 'T') == GLFW_PRESS){
         shadow_mapping_effect = 2;
      }
      if(glfwGetKey(window, 'Y') == GLFW_PRESS){
         shadow_mapping_effect = 3;
      }
      if(glfwGetKey(window, 'U') == GLFW_PRESS){
         shadow_mapping_effect = 4;
      }
      if(glfwGetKey(window, 'F') == GLFW_PRESS){
         shadow_mapping_effect = 5;
      }
      if(glfwGetKey(window, 'G') == GLFW_PRESS){
         shadow_mapping_effect = 6;
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
   //different projection matrices for different needs (shadow map need wider angle fov)
   projection_mat = glm::perspective(cam_fov_angle, (float)win_width/(float)win_height, 0.01f, 10000.0f);
   projection_mat_depth = glm::perspective(cam_fov_angle, (float)win_width/(float)win_height, 0.1f, 1000.0f);
   projection_mat_shadowmap_depth = glm::perspective(cam_fov_angle_shadowmap, (float)win_width/(float)win_height, 2.0f, 1000.0f);

   light_position[0] = 0.0; //x
   light_position[1] = 0.0; //up
   light_position[2] = 0.0; //z

   light_mode_selected = 1;

   GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");
   for (size_t i = 0; i < NB_CUBES; i++) {
       cube[i].init(cube_pid);
   }
   cube_base.init(cube_pid);

   GLuint sphere_pid = load_shaders("sphere_vshader.glsl", "sphere_fshader.glsl");
   sphere[0].init(sphere_pid, 32, 32);
   sphere[1].init(sphere_pid, 32, 32);

   for (size_t i = 0; i < NB_CUBES; i++) {
       lst_drawable.push_back(&cube[i]);
   }
   lst_drawable.push_back(&cube_base);
   lst_drawable.push_back(&sphere[0]);
   lst_drawable.push_back(&sphere[1]);

   depth_framebuffer_camera.init(win_width, win_height, cam_fov_angle);
   depth_framebuffer_light.init(DEPTH_TEXTURE_SIZE);

   depth_framebuffer_camera.set_perspective_mat(projection_mat_depth);
   depth_framebuffer_light.set_perspective_mat(projection_mat_shadowmap_depth);

   //position of the cubes to make something interesting for the volumetric light
   cube_transf[0].scale(5.0f, 5.0f, 1.0f);
   cube_transf[0].translate(1.2f, 0.0f, 0.0f);

   cube_transf[1].scale(5.0f, 5.0f, 1.0f);
   cube_transf[1].translate(-1.2f, 0.0f, 0.0f);

   cube_transf[2].scale(1.0f, 1.0f, 1.0f);
   cube_transf[2].translate(0.0f, 0.0f, 0.0f);

   cube_transf[3].scale(1.0f, 1.0f, 1.0f);
   cube_transf[3].translate(0.0f, 4.0f, 0.0f);

   cube_transf[4].scale(1.0f, 8.0f, 1.0f);
   cube_transf[4].translate(10.0f, 0.0f, -10.0f);

   cube_base_transf.translate(0.0f, -2.0f, 0.0f);
   cube_base_transf.scale(20.0f, 1.0f, 20.0f);

   sphere_transf[0].translate(-2.0f, 0.0f, 2.0f);
   sphere_transf[1].translate(2.0f, 0.0f, -2.0f);

   GLuint tex_fb = framebuffer.init(win_width, win_height);
   GLuint depth_tex_fb = depth_framebuffer_camera.get_texture_id();

   quad_pid = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, quad_pid);
   quad_screen.set_depth_texture(depth_tex_fb);

   glUseProgram(quad_pid);
   glUniform1f( glGetUniformLocation(quad_pid, "fov_angle"), cam_fov_angle);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam_free.lookAt(-3.0f, 3.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam = &cam_free;
   cam_free.update_pos();
}

void display(){

   cam->get_position(camera_position);
   cam->get_center(camera_direction);

   depth_framebuffer_camera.set_light_pos(camera_position);
   depth_framebuffer_camera.set_light_dir(camera_direction);
   depth_framebuffer_light.set_light_pos(light_position);
   //to be done before binding any fb (since it loads a fb itself)
   //draw the scene from the point of view of the light to the depth buffer
   //which will be given to the various shaders of opjects
   depth_framebuffer_light.draw_fb(&lst_drawable);
   depth_framebuffer_camera.draw_fb(&lst_drawable);

   framebuffer.bind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   for (size_t i = 0; i < NB_CUBES; i++) {
       cube[i].set_MVP_matrices(cube_transf[i].get_matrix(), cam->getMatrix(), projection_mat);
       cube[i].set_light_pos(light_position);
       cube[i].set_camera_pos(camera_position);

       cube[i].set_shadow_buffer_texture(depth_framebuffer_light.get_texture_id());
       cube[i].set_shadow_matrix(depth_framebuffer_light.get_shadow_mat());
   }

   sphere[0].set_MVP_matrices(sphere_transf[0].get_matrix(), cam->getMatrix(), projection_mat);
   sphere[0].set_light_pos(light_position);
   sphere[0].set_camera_pos(camera_position);
   sphere[1].set_MVP_matrices(sphere_transf[1].get_matrix(), cam->getMatrix(), projection_mat);
   sphere[1].set_light_pos(light_position);
   sphere[1].set_camera_pos(camera_position);

   sphere[0].set_shadow_buffer_texture(depth_framebuffer_light.get_texture_id());
   sphere[0].set_shadow_matrix(depth_framebuffer_light.get_shadow_mat());
   sphere[1].set_shadow_buffer_texture(depth_framebuffer_light.get_texture_id());
   sphere[1].set_shadow_matrix(depth_framebuffer_light.get_shadow_mat());

   cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
   cube_base.set_light_pos(light_position);
   cube_base.set_camera_pos(camera_position);

   cube_base.set_shadow_buffer_texture(depth_framebuffer_light.get_texture_id());
   cube_base.set_shadow_matrix(depth_framebuffer_light.get_shadow_mat());

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_shadow_buffer_texture_size(DEPTH_TEXTURE_SIZE);
      lst_drawable[i]->set_shadow_mapping_effect(shadow_mapping_effect);
      lst_drawable[i]->draw();
   }

   framebuffer.unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   //prepare the quad that will do the post processing (shadow map + vol. light)
   glUseProgram(quad_pid);
   glUniformMatrix4fv( glGetUniformLocation(quad_pid, "camera_transform_matrix"), 1, GL_FALSE, glm::value_ptr(cam->getMatrix()));
   glUniformMatrix4fv( glGetUniformLocation(quad_pid, "shadow_map_transform_mat"), 1, GL_FALSE, glm::value_ptr(depth_framebuffer_light.get_shadow_mat()));

   //get depth texture from the point of view of light for the vol. light
   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_2D, depth_framebuffer_light.get_texture_id());
   GLuint tex_id = glGetUniformLocation(quad_pid, "shadow_map_depth_tex");
   glUniform1i(tex_id, 3 );

   //give the projection matrix used for the depth buffer to the post processing shader
   glUniformMatrix4fv( glGetUniformLocation(quad_pid, "proj_transform_matrix"), 1, GL_FALSE, glm::value_ptr(projection_mat_depth));

   //diplay the depth texture on the screen
   //quad_screen.load_texture(depth_framebuffer_light.get_texture_id());
   quad_screen.draw(effect_select);

   //move light up and down with time make the coord y of light go from 0 to 10
   if(light_mode_selected == 0){
      light_position[0] = 16.0*cos(glfwGetTime()/8);
      light_position[1] = 4;
      light_position[2] = 16.0*sin(glfwGetTime()/8);
   }
   else if(light_mode_selected == 1){
      light_position[0] = 16.0*cos(glfwGetTime()/8);
      light_position[1] = 8;
      light_position[2] = 16.0*sin(glfwGetTime()/8);
   }
   else if(light_mode_selected == 2){
      light_position[0] = -3;
      light_position[1] = 6;
      light_position[2] = 10;
   }
   else if(light_mode_selected == 3){
      light_position[0] = 7;
      light_position[1] = 3;
      light_position[2] = 10;
   }
}

void cleanup(){
}
