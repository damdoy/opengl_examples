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

   glm::mat4x4 getMatrix(){
      return glm::lookAt(eye, center, up);
   }

   virtual void input_handling(char){

   }

   virtual void update_pos(){

   }

protected:
   //for the lookat functon
   glm::vec3 eye;
   glm::vec3 center;
   glm::vec3 up;
};

#endif
