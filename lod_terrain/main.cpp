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
#include "camera_fps.h"
#include "shader_helper.h"
#include "transform.h"
#include "noise_generator.hpp"
#include "_quad_screen/quad_screen.h"
#include "_terrain_quad/qterrain.h"
#include "framebuffer.h"

#include "_terrain_quad/qtree_test.hpp"

//defines the factor for the lod
//lower = high precision terrain far away
//higher = high precision terrain near (better performance)
// takes into account the model matrix in qterrain, no need to adjust for it
#define FACTOR_DISTANCE_LOD 1.5f
//#define FACTOR_DISTANCE_LOD 2.0f

//tells, at level lod 0, which size should the sub terrain be
//if value is X, then there would be (X*2) by (X*2) squares in the terrain
//each lod level multiplies this value by 2
#define TERRAIN_INTIAL_GRANULARITY 8

//max lod levels
#define LOD_MAX_LEVELS 8


//the level of the noise generator defines the number of passes that it will go through
//to generate the noise, there is no need to have lots of passes if the terrain is not
//granular (in terms of triangles)
//the noise generator level is chosen as a function of log2(terrain_granularity)
//to avoid having a noise generation overload at high granularity, this option allows
//to limit the level of noise gen
#define NOISE_GENERATOR_MAX_LEVEL 6

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;
Camera_fps cam_fps;

Framebuffer framebuffer;

QTerrain qterrain;
Quad_screen quad_screen;
Transform terrain_transf;
glm::mat4x4 projection_mat;

Noise_generator noise_gen;

GLfloat light_position[3];
bool moving_light;
bool activate_wireframe;
GLfloat camera_position[3];

const int win_width = 1280;
const int win_height = 720;

unsigned int effect_select; //from framebuffer example, keep at 0 (normal)

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "lod_terrain", NULL, NULL);

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

      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         cam = &cam_free;
      }
      if(glfwGetKey(window, 'N') == GLFW_PRESS){
         cam = &cam_fps;
      }

      if(glfwGetKey(window, 'B') == GLFW_PRESS){
         moving_light = false;
      }
      if(glfwGetKey(window, 'V') == GLFW_PRESS){
         moving_light = true;
      }

      if(glfwGetKey(window, 'C') == GLFW_PRESS){
         activate_wireframe = false;
      }
      if(glfwGetKey(window, 'X') == GLFW_PRESS){
         activate_wireframe = true;
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

   glClearColor(0.6, 0.8, 1.0, 1.0);

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

   moving_light = true;
   activate_wireframe = false;

   GLuint tex_fb = framebuffer.init(win_width, win_height);

   noise_gen.setup(4, 8, 0.7f, 0.5f, NOISE_SELECT_PERLIN);

   qterrain.init(&noise_gen, FACTOR_DISTANCE_LOD, TERRAIN_INTIAL_GRANULARITY, LOD_MAX_LEVELS, NOISE_GENERATOR_MAX_LEVEL);
   //terrain_transf.scale(2048, 128, 2048);
   terrain_transf.scale(512, 32, 512);
   qterrain.set_model(terrain_transf);

   GLuint quad_pid = load_shaders("quad_screen_vshader.glsl", "quad_screen_fshader.glsl");
   quad_screen.init(tex_fb, win_width, win_height, quad_pid);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam_free.lookAt(-3.0f, 20.0f, 0.0f, 50.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_speed(25.0f);

   cam_fps.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fps.init(0.5f, &qterrain);
   cam = &cam_free;
   cam_free.update_pos();

   qtree_test_base();
   qtree_test_neighbour_simple();
   qtree_test_neighbour_of_child();
   qtree_test_neighbour_different_parents();
}

void display(){

   //update the lod precision from the camera position
   qterrain.update_lod_camera(camera_position);

   framebuffer.bind();
   if(activate_wireframe){
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glClearColor(1.0, 1.0, 1.0, 1.0);
   }
   else{
      glClearColor(0.6, 0.8, 1.0, 1.0);
   }

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   cam->get_position(camera_position);

   qterrain.draw(cam->getMatrix(), projection_mat, light_position, camera_position, true, false, activate_wireframe);
   framebuffer.unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   quad_screen.draw(effect_select);

   //rough sun movement
   if(moving_light){
      light_position[1] = 2048.0*cos(glfwGetTime()/2);
      light_position[2] = 2048.0*sin(glfwGetTime()/2);
   }
}

void cleanup(){
}
