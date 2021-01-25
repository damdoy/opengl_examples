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

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;

Framebuffer framebuffer;

Cube cube[2];
Cube cube_base;
Sphere sphere[2];
Quad_screen quad_screen;
Transform cube_base_transf;
Transform cube_transf[2];
Transform sphere_transf[2];
glm::mat4x4 projection_mat;

Depth_framebuffer depth_framebuffer;

std::vector<Drawable*> lst_drawable;

GLfloat light_position[3];
unsigned int light_mode_selected;
GLfloat camera_position[3];
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

      if(glfwGetKey(window, 'S') == GLFW_PRESS){
         cam->input_handling('S');
      }
      if(glfwGetKey(window, 'A') == GLFW_PRESS){
         cam->input_handling('A');
      }

      if(glfwGetKey(window, 'W') == GLFW_PRESS){
         cam->input_handling('W');
      }
      if(glfwGetKey(window, 'D') == GLFW_PRESS){
         cam->input_handling('D');
      }

      if(glfwGetKey(window, 'L') == GLFW_PRESS){
         cam->input_handling('L');
      }

      if(glfwGetKey(window, 'J') == GLFW_PRESS){
         cam->input_handling('J');
      }
      if(glfwGetKey(window, 'K') == GLFW_PRESS){
         cam->input_handling('K');
      }
      if(glfwGetKey(window, 'I') == GLFW_PRESS){
         cam->input_handling('I');
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
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.01f, 10000.0f);

   light_position[0] = 0.0; //x
   light_position[1] = 0.0; //up
   light_position[2] = 0.0; //z

   light_mode_selected = 1;

   GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");
   cube[0].init(cube_pid);
   cube[1].init(cube_pid);
   cube_base.init(cube_pid);

   GLuint sphere_pid = load_shaders("sphere_vshader.glsl", "sphere_fshader.glsl");
   sphere[0].init(sphere_pid, 32, 32);
   sphere[1].init(sphere_pid, 32, 32);

   lst_drawable.push_back(&cube[0]);
   lst_drawable.push_back(&cube[1]);
   lst_drawable.push_back(&cube_base);
   lst_drawable.push_back(&sphere[0]);
   lst_drawable.push_back(&sphere[1]);

   depth_framebuffer.init(DEPTH_TEXTURE_SIZE);

   cube_transf[0].scale(1.0f, 3.0f, 1.0f);
   cube_transf[0].translate(2.0f, 0.0f, 2.0f);

   cube_transf[1].scale(1.0f, 5.0f, 1.0f);
   cube_transf[1].translate(-2.0f, 0.0f, -2.0f);

   cube_base_transf.translate(0.0f, -2.0f, 0.0f);
   cube_base_transf.scale(20.0f, 1.0f, 20.0f);

   sphere_transf[0].translate(-2.0f, 0.0f, 2.0f);
   sphere_transf[1].translate(2.0f, 0.0f, -2.0f);

   GLuint tex_fb = framebuffer.init(win_width, win_height);

   GLuint quad_pid = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, quad_pid);


   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam_free.lookAt(-3.0f, 3.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam = &cam_free;
   cam_free.update_pos();
}

void display(){

   depth_framebuffer.set_light_pos(light_position);
   //to be done before binding any fb (since it loads a fb itself)
   //draw the scene from the point of view of the light to the depth buffer
   //which will be given to the various shaders of opjects
   depth_framebuffer.draw_fb(&lst_drawable);

   framebuffer.bind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   cam->get_position(camera_position);

   cube[0].set_MVP_matrices(cube_transf[0].get_matrix(), cam->getMatrix(), projection_mat);
   cube[0].set_light_pos(light_position);
   cube[0].set_camera_pos(camera_position);
   cube[1].set_MVP_matrices(cube_transf[1].get_matrix(), cam->getMatrix(), projection_mat);
   cube[1].set_light_pos(light_position);
   cube[1].set_camera_pos(camera_position);

   cube[0].set_shadow_buffer_texture(depth_framebuffer.get_texture_id());
   cube[0].set_shadow_matrix(depth_framebuffer.get_shadow_mat());
   cube[1].set_shadow_buffer_texture(depth_framebuffer.get_texture_id());
   cube[1].set_shadow_matrix(depth_framebuffer.get_shadow_mat());

   sphere[0].set_MVP_matrices(sphere_transf[0].get_matrix(), cam->getMatrix(), projection_mat);
   sphere[0].set_light_pos(light_position);
   sphere[0].set_camera_pos(camera_position);
   sphere[1].set_MVP_matrices(sphere_transf[1].get_matrix(), cam->getMatrix(), projection_mat);
   sphere[1].set_light_pos(light_position);
   sphere[1].set_camera_pos(camera_position);

   sphere[0].set_shadow_buffer_texture(depth_framebuffer.get_texture_id());
   sphere[0].set_shadow_matrix(depth_framebuffer.get_shadow_mat());
   sphere[1].set_shadow_buffer_texture(depth_framebuffer.get_texture_id());
   sphere[1].set_shadow_matrix(depth_framebuffer.get_shadow_mat());

   cube_base.set_MVP_matrices(cube_base_transf.get_matrix(), cam->getMatrix(), projection_mat);
   cube_base.set_light_pos(light_position);
   cube_base.set_camera_pos(camera_position);

   cube_base.set_shadow_buffer_texture(depth_framebuffer.get_texture_id());
   cube_base.set_shadow_matrix(depth_framebuffer.get_shadow_mat());

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_shadow_buffer_texture_size(DEPTH_TEXTURE_SIZE);
      lst_drawable[i]->set_shadow_mapping_effect(shadow_mapping_effect);
      lst_drawable[i]->draw();
   }

   framebuffer.unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   //diplay the depth texture on the screen
   //quad_screen.load_texture(depth_framebuffer.get_texture_id());
   quad_screen.draw(effect_select);

   //move light up and down with time make the coord y of light go from 0 to 10
   if(light_mode_selected == 0){
      light_position[0] = 0.0;
      light_position[1] = 32.0*cos(glfwGetTime()/2);
      light_position[2] = 32.0*sin(glfwGetTime()/2);
   }
   else if(light_mode_selected == 1){
      light_position[0] = 16.0*cos(glfwGetTime()/2);
      light_position[1] = 8;
      light_position[2] = 16.0*sin(glfwGetTime()/2);
   }
   else if(light_mode_selected == 2){
      light_position[0] = 4;
      light_position[1] = 5;
      light_position[2] = 3;
   }
   else if(light_mode_selected == 3){
      light_position[0] = 7;
      light_position[1] = 3;
      light_position[2] = 10;
   }
}

void cleanup(){
}
