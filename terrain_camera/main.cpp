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
#include "_terrain/terrain.h"
#include "transform.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;
Camera_fps cam_fps;

Terrain terrain;
glm::mat4x4 projection_mat;

GLfloat light_position[3];
bool moving_light;
GLfloat camera_position[3];
GLuint light_mode = 0;

bool activate_colour = true;
bool activate_heightmap = false;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "terrain_camera", NULL, NULL);

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

   //main loop: control and drawing
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

      //moving
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

      //change view direction
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

      //change camera type
      if(glfwGetKey(window, 'M') == GLFW_PRESS){
         cam = &cam_free;
      }
      if(glfwGetKey(window, 'N') == GLFW_PRESS){
         cam = &cam_fps;
      }

      //have a moving light or not
      if(glfwGetKey(window, 'B') == GLFW_PRESS){
         moving_light = false;
      }
      if(glfwGetKey(window, 'V') == GLFW_PRESS){
         moving_light = true;
      }

      //activate the colours of the terrain
      if(glfwGetKey(window, 'Z') == GLFW_PRESS){
         activate_colour = true;
      }
      if(glfwGetKey(window, 'X') == GLFW_PRESS){
         activate_colour = false;
      }

      //activate heightmap: display the height value as colour black=deep white=high
      if(glfwGetKey(window, 'T') == GLFW_PRESS){
         activate_heightmap = true;
      }
      if(glfwGetKey(window, 'Y') == GLFW_PRESS){
         activate_heightmap = false;
      }

      display();
      glfwSwapBuffers(window);

      prev_time = current_time;
   }

   cleanup();

   return 0;
}

void init(){
   glClearColor(0.4, 0.8, 1.0, 1.0);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.01f, 1000.0f);

   //light far up in the sky
   light_position[0] = 0.0; //x
   light_position[1] = 1000.0; //up
   light_position[2] = 1000.0; //z

   moving_light = false;

   Transform transf;
   //change the scale of the terrain
   transf.scale(384.0f, 32.0f, 384.0f);

   std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
   terrain.init(512, 512);
   std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

   unsigned long duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

   printf("duration to generate the terrain (heightmap + buffers): %lums\n", duration);

   terrain.set_model(transf);
   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

   cam_free.lookAt(3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_speed(25.0f);

   cam_fps.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fps.init(0.5f, &terrain);
   cam_fps.set_speed(10.0f);

   cam = &cam_fps;
   cam_fps.update_pos();
}

void display(){
   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   cam->get_position(camera_position);
   terrain.draw(cam->getMatrix(), projection_mat, light_position, camera_position, activate_colour, activate_heightmap);

   //move light up and down with time make the coord y of light go from 0 to 10
   if(moving_light){
      light_position[1] = 1000.0 - fabs(1000.0-fmod(200*glfwGetTime(), 2000.0));
   }
   else{
      light_position[1] = 1000.0f;
   }
}

void cleanup(){
}
