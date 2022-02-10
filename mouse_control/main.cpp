#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective

#include "camera_rotate.h"
#include "shader_helper.h"
#include "_plane/plane.h"
#include "_cube/cube.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera_rotate cam;
Plane plane;
Cube cube;
glm::mat4x4 projection_mat;
glm::mat4x4 cube_model_mat;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "mouse_control", NULL, NULL);

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
      display();
      glfwSwapBuffers(window);
      glfwPollEvents();

      static bool press = false;
      static double prev_x_press = 0, prev_y_press = 0;

      int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
      if (state == GLFW_PRESS)
      {
         double pos_x, pos_y;
         glfwGetCursorPos(window, &pos_x, &pos_y);

         //if first press, set current positions as previous ones to avoid a jump
         if(!press){
            press = true;
            prev_x_press = pos_x;
            prev_y_press = pos_y;
         }

         double diff_x = pos_x-prev_x_press;
         double diff_y = pos_y-prev_y_press;

         cam.rotate_angle_x(-diff_x);
         cam.rotate_angle_y(diff_y);

         prev_x_press = pos_x;
         prev_y_press = pos_y;

      }
      else{
         if(press){
            press = false;
         }
      }
   }

   cleanup();

   return 0;
}

void init(){
   glClearColor(0.2, 0.2, 0.2, 1.0);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 10.0f);

   GLuint plane_pid = load_shaders("plane_vshader.glsl", "plane_fshader.glsl");
   plane.init(plane_pid);
   GLuint cube_pid = load_shaders("cube_vshader.glsl", "cube_fshader.glsl");
   cube.init(cube_pid);

   //transformations for the cube : put it on the plane which is at default at height 0
   cube_model_mat = glm::mat4(1.0); //identity
   cube_model_mat = glm::translate(cube_model_mat, glm::vec3(0.0, 0.5, 0.0));
   cube_model_mat = glm::scale(cube_model_mat, glm::vec3(0.5, 0.5, 0.5));

   //default values for the cam, look at the centrer (0,0,0) with radius 2 and slower speed
   cam.init(0, 0, 0, 2, 1/160.0f);

   //put the camera a bit higher by default
   cam.rotate_angle_y(50);
}

void display(){
   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   plane.draw(glm::mat4(1.0), cam.getMatrix(), projection_mat);

   cube.draw(cube_model_mat, cam.getMatrix(), projection_mat);
}

void cleanup(){
}
