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
#include "_plane/plane_float.h"
#include "transform.h"
#include "noise_generator.hpp"
#include "texture_float.h"

const GLfloat vpoint[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,};

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera_free cam_free;

Plane_float plane_perlin;
Plane_float plane_voronoi;
Transform plane_transf;
glm::mat4x4 projection_mat;
Texture_float plane_texture_perlin;
Texture_float plane_texture_voronoi;

Noise_generator noise_gen;

uint noise_selection;

GLfloat light_position[3];
bool moving_light;
GLfloat camera_position[3];
GLuint light_mode = 0;

const int win_width = 1280;
const int win_height = 720;

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "bumpmapping", NULL, NULL);

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

      if(glfwGetKey(window, '0') == GLFW_PRESS){
         light_mode = 0;
      }
      else if(glfwGetKey(window, '1') == GLFW_PRESS){
         light_mode = 1;
      }
      else if(glfwGetKey(window, '2') == GLFW_PRESS){
         light_mode = 2;
      }

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

      if(glfwGetKey(window, 'B') == GLFW_PRESS){
         moving_light = false;
      }
      if(glfwGetKey(window, 'V') == GLFW_PRESS){
         moving_light = true;
      }

      if(glfwGetKey(window, '1') == GLFW_PRESS){
         noise_selection = 1;
      }
      if(glfwGetKey(window, '2') == GLFW_PRESS){
         noise_selection = 2;
      }

      display();
      glfwSwapBuffers(window);

      prev_time = current_time;
   }

   cleanup();

   return 0;
}

void init(){
   glClearColor(1.0, 1.0, 1.0, 1.0);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.01f, 1000.0f);

   light_position[0] = 0.0; //x
   light_position[1] = 2.0; //up
   light_position[2] = 0.0; //z

   moving_light = true;

   plane_transf.scale(2.0f, 1.0f, 2.0f);
   plane_transf.translate(0.0f, -1.0f, 0.0f);

   GLuint pid_plane_shaders = load_shaders("plane_tex_vshader.glsl", "plane_tex_fshader.glsl");
   plane_perlin.init(pid_plane_shaders);
   plane_voronoi.init(pid_plane_shaders);

   std::vector<std::vector<float> > plane_texture_data = noise_gen.get_2D_noise(1024, 1024, -1.0f, 1.0f, -1.0f, 1.0f);
   plane_texture_perlin.set_data(plane_texture_data);
   plane_perlin.set_texture(&plane_texture_perlin);

   noise_gen.setup(4, 1, 0.5f, 0.5f, NOISE_SELECT_VORONOI);
   std::vector<std::vector<float> > plane_texture_data_voronoi = noise_gen.get_2D_noise(1024, 1024, -1.0f, 1.0f, -1.0f, 1.0f);
   plane_texture_voronoi.set_data(plane_texture_data_voronoi);
   plane_voronoi.set_texture(&plane_texture_voronoi);

   cam_free.lookAt(3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam = &cam_free;
   cam_free.update_pos();

   noise_selection = 1;
}

void display(){
   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   cam->get_position(camera_position);

   //make the light go around the bump mapped tile
   if(moving_light){
      light_position[0] = 2.0*cos(glfwGetTime());
      light_position[2] = 2.0*sin(glfwGetTime());
   }

   if(noise_selection == 1){
      plane_perlin.draw(plane_transf.get_matrix(), cam->getMatrix(), projection_mat, light_position, camera_position);
   }
   else if(noise_selection == 2){
      plane_voronoi.draw(plane_transf.get_matrix(), cam->getMatrix(), projection_mat, light_position, camera_position);
   }

}

void cleanup(){
}
