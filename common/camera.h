#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//fixed camera
class Camera{
public:
   Camera(){

   }

   virtual void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up){
      this->eye = eye;
      this->center = center;
      this->up = up;
   }

   virtual void lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ){
      lookAt(glm::vec3(eyeX, eyeY, eyeZ), glm::vec3(centerX, centerY, centerZ), glm::vec3(upX, upY, upZ));
   }

   void get_position(float position[3]){
      position[0] = eye.x;
      position[1] = eye.y;
      position[2] = eye.z;
   }

   void get_direction(float direction[3]){
      direction[0] = center.x-eye.x;
      direction[1] = center.y-eye.y;
      direction[2] = center.z-eye.z;
   }

   void get_center(float center[3]){
      center[0] = this->center.x;
      center[1] = this->center.y;
      center[2] = this->center.z;
   }


   glm::mat4x4 getMatrix(){
      return glm::lookAt(eye, center, up);
   }

   glm::mat4x4 get_reflection_matrix(float height){
      //just return another lookat on the other side of the plane (only plane along y)
      return glm::lookAt(glm::vec3(eye.x, -eye.y+height*2, eye.z), glm::vec3(center.x, -center.y+height*2, center.z), up);
   }

   glm::mat4x4 get_perspective_mat(){
      return glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);
   }

   virtual void input_handling(char, float){

   }

   virtual void update_pos(){

   }

   virtual void set_window_size(unsigned int win_width, unsigned int win_height){
      this->win_width = win_width;
      this->win_height = win_height;
   }

protected:
   //for the lookat functon
   glm::vec3 eye;
   glm::vec3 center;
   glm::vec3 up;

   unsigned int win_width;
   unsigned int win_height;
};

#endif
