#ifndef CAMERA_ROTATE_H
#define CAMERA_ROTATE_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

class Camera_rotate : public Camera{
public:
   Camera_rotate(){

   }

   void init(glm::vec3 center, float radius, float speed){
      this->eye = glm::vec3(radius, 0, 0);
      this->center = center;
      this->up = glm::vec3(0, 1, 0);
      this->normal_y_rot = glm::vec3(0, 0, 1);
      this->angle_speed = speed;
   }

   void init(float center_x, float center_y, float center_z, float radius, float speed){
      init(glm::vec3(center_x, center_y, center_z), radius, speed);
   }

   void rotate_angle_x(float angle){
      glm::mat4 rot_mat = glm::mat4(1.0); //identity

      rot_mat = glm::rotate(rot_mat, angle*this->angle_speed, glm::vec3(0.0, 1.0, 0.0));

      this->eye = glm::vec3(rot_mat*glm::vec4(this->eye, 0.0));

      //keep rotate the normal for y_rot so that it rotates with the camera
      this->normal_y_rot = glm::vec3(rot_mat*glm::vec4(this->normal_y_rot, 0.0));

      this->angle_x += angle*this->angle_speed;
   }

   void rotate_angle_y(float angle){
      glm::mat4 rot_mat = glm::mat4(1.0); //identity

      this->angle_y += angle*this->angle_speed;

      if(this->angle_y > M_PI/2){
         this->angle_y = M_PI/2;
         return;
      }

      if(this->angle_y < -M_PI/2){
         this->angle_y = -M_PI/2;
         return;
      }
      rot_mat = glm::rotate(rot_mat, angle*this->angle_speed, this->normal_y_rot);

      this->eye = glm::vec3(rot_mat*glm::vec4(this->eye, 0.0));
   }

protected:
   glm::vec3 normal_y_rot;

   float angle_x;
   float angle_y;

   float angle_speed;
};

#endif
