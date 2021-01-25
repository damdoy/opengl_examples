#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective

#include "camera.h"
#include "shader_helper.h"
#include "_plane/plane.h"
#include "_sphere/sphere.h"

const GLfloat vpoint[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,};

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera cam;
Plane plane;
Sphere sphere;
glm::mat4x4 projection_mat = glm::mat4(1.0);
glm::mat4x4 sphere_model_mat = glm::mat4(1.0);

GLuint pid_shader_plane;
GLuint pid_shader_sphere;

GLfloat light_position[3];
GLfloat camera_position[3];
GLfloat spot_dir[3];
GLuint light_mode = 0;
bool activate_specular = true;
bool activate_spot = true;
uint light_animation;

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

   GLFWwindow* window = glfwCreateWindow(win_width, win_height, "sphere_light", NULL, NULL);

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

   //mainloop with input control
   while(glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window)){
      glfwPollEvents();

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
         activate_specular = true;
      }
      else if(glfwGetKey(window, 'A') == GLFW_PRESS){
         activate_specular = false;
      }

      if(glfwGetKey(window, 'W') == GLFW_PRESS){
         activate_spot = true;
      }
      else if(glfwGetKey(window, 'Q') == GLFW_PRESS){
         activate_spot = false;
      }

      if(glfwGetKey(window, 'Y') == GLFW_PRESS){
         light_animation = 0; //side light
      }
      else if(glfwGetKey(window, 'X') == GLFW_PRESS){
         light_animation = 1; // rotating light
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

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glViewport(0,0,win_width,win_height);
   projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 10.0f);

   light_animation = 0;

   light_position[0] = 0.0; //x
   light_position[1] = 0.0; //up
   light_position[2] = 5.0; //z

   pid_shader_plane = load_shaders("plane_vshader.glsl", "plane_fshader.glsl");
   pid_shader_sphere = load_shaders("sphere_vshader.glsl", "sphere_fshader.glsl");
   plane.init(pid_shader_plane);
   sphere.init(pid_shader_sphere, 16, 16);
   sphere_model_mat = glm::translate(sphere_model_mat, glm::vec3(0.0, 0.5, 0.0));
   sphere_model_mat = glm::scale(sphere_model_mat, glm::vec3(0.5, 0.5, 0.5));
   cam.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam.get_position(camera_position);
}

void display(){
   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   plane.draw(glm::mat4(1.0), cam.getMatrix(), projection_mat);
   float angle = glfwGetTime()*0.5;
   glm::mat4 sphere_model_mat_rot = glm::rotate(sphere_model_mat, angle, glm::vec3(0.0, 1.0, 0.0));

   //move light up and down with time make the coord y of light go from 0 to 10
   if(light_animation == 0){
      light_position[0] = 0.0;
      light_position[1] = 10.0 - fabs(10.0-fmod(2*glfwGetTime(), 20.0));
      light_position[2] = 5.0;

      //have the spot directed vaguely to the sphere
      spot_dir[0] = 0.0;
      spot_dir[1] = -1.0/1.4142;
      spot_dir[2] = -1.0/1.4142;
   }
   else{
      //circle rotating above the sphere
      light_position[0] = 2*sin(glfwGetTime()*0.5);
      light_position[1] = 3.0;
      light_position[2] = 2*cos(glfwGetTime()*0.5);

      //spot centered on the sphere (0,0)
      //will be normalized by the shader
      //division on x and z is to off-center the spot (more natural)
      spot_dir[0] = -light_position[0]/1.2;
      spot_dir[1] = -light_position[1];
      spot_dir[2] = -light_position[2]/1.2;
   }

   //update positions and options to shaders
   glUseProgram(pid_shader_plane);
   glUniform3fv( glGetUniformLocation(pid_shader_plane, "light_position"), 1, light_position);
   glUniform3fv( glGetUniformLocation(pid_shader_plane, "camera_position"), 1, camera_position);
   glUniform1ui( glGetUniformLocation(pid_shader_plane, "lighting_mode"), light_mode);
   glUniform1ui( glGetUniformLocation(pid_shader_plane, "activate_specular"), activate_specular);
   glUniform3fv( glGetUniformLocation(pid_shader_plane, "spot_direction"), 1, spot_dir);
   glUniform1ui( glGetUniformLocation(pid_shader_plane, "activate_spot"), activate_spot);
   glUseProgram(0);

   glUseProgram(pid_shader_sphere);
   glUniform3fv( glGetUniformLocation(pid_shader_sphere, "light_position"), 1, light_position);
   glUniform3fv( glGetUniformLocation(pid_shader_sphere, "camera_position"), 1, camera_position);
   glUniform1ui( glGetUniformLocation(pid_shader_sphere, "lighting_mode"), light_mode);
   glUniform1ui( glGetUniformLocation(pid_shader_sphere, "activate_specular"), activate_specular);
   glUniform3fv( glGetUniformLocation(pid_shader_sphere, "spot_direction"), 1, spot_dir);
   glUniform1ui( glGetUniformLocation(pid_shader_sphere, "activate_spot"), activate_spot);
   glUseProgram(0);

   sphere.draw(sphere_model_mat_rot, cam.getMatrix(), projection_mat);
}

void cleanup(){
}
