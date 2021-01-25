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
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp> //perspective

#include <IL/il.h>

#include "camera.h"
#include "shader_helper.h"
#include "_plane/plane.h"
#include "texture.h"
#include "texture_checkers.h"

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera cam;
Plane plane;
//init matrices with identity
glm::mat4x4 projection_mat = glm::mat4(1.0);
glm::mat4x4 plane_model_mat = glm::mat4(1.0);

Texture plane_texture;
Texture_checkers tex_checkers;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "texture_plane", NULL, NULL);

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

      if(glfwGetKey(window, '1')==GLFW_PRESS){
         plane.set_texture(&plane_texture);
      }
      else if(glfwGetKey(window, '2') == GLFW_PRESS){
         plane.set_texture(&tex_checkers);
      }

      display();
      glfwSwapBuffers(window);
   }

   cleanup();

   return 0;
}

void init(){
   glClearColor(1.0, 1.0, 1.0, 1.0);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(60.0f*2*3.1415f/360.0f, (float)win_width/(float)win_height, 0.1f, 10.0f);

   ilInit();

   bool loaded = plane_texture.load_image("the_red_pepper.png");
   tex_checkers.init(8);

   if(!loaded){
      std::cout << "texture could not load :(" << std::endl;
   }

   GLuint plane_pid = load_shaders("plane_vshader.glsl", "plane_fshader.glsl");
   plane.init(plane_pid);
   plane.set_texture(&plane_texture);

   cam.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void display(){
   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   float angle = glfwGetTime();

   //rotation matrix for the textured plane
   glm::mat4x4 plane_rot = glm::rotate(plane_model_mat, angle, glm::vec3(0.0, 1.0, 0.0));
   plane.draw(plane_rot, cam.getMatrix(), projection_mat);
}

void cleanup(){
}
