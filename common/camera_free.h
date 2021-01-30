#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

//free moving camera
class Camera_free : public Camera{
public:
   Camera_free(){
      this->speed = 4.0f;
      this->angle_change_per_second = 1.8f;
   }

   virtual void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up){
      const float EPSILON = 0.000001f;
      Camera::lookAt(eye, center, up);
      glm::vec3 direction = center-eye;
      direction = glm::normalize(direction);
      if (direction[0] < EPSILON && direction[0] > -EPSILON){
         direction[0] = EPSILON;
      }
      angle_up = atan(sqrt(direction[0]*direction[0]+direction[2]*direction[2])/direction[1]);
      if(angle_up < 0.0f){
         angle_up = 3.1415f+angle_up;
      }
      // printf("dir x:%f, dir y: %f, angle_up:%f\n", direction[0], direction[1], angle_up);
      angle_side = atan(direction[2]/direction[0]);

      if(direction[0] < 0.0f){
         angle_side += 3.1415f;
      }

      // printf("dir x:%f, dir z: %f, side:%f\n", direction[0], direction[2], angle_side);

      //normalize the center vector
      normalize(direction);
      center = eye+direction;

      this->update_pos();
   }

   virtual void lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ){
      lookAt(glm::vec3(eyeX, eyeY, eyeZ), glm::vec3(centerX, centerY, centerZ), glm::vec3(upX, upY, upZ));
   }

   void set_speed(float speed){
      this->speed = speed;
   }

   void set_view_speec(float speed){
      this->angle_change_per_second = speed;
   }

   virtual void input_handling(char key, float time_delta){

      glm::vec3 direction;
      glm::vec3 speed_factor = glm::vec3(speed)*time_delta;

      direction = center - eye;
      direction = normalize(direction);

      glm::vec3 left_dir = normalize(cross(up, direction));
      glm::vec3 right_dir = normalize(cross(direction, up));

      switch(key){
         case 'W':
            eye = eye + speed_factor*direction;
            center = center + speed_factor*direction;
         break;
         case 'S':
            eye = eye - speed_factor*direction;
            center = center - speed_factor*direction;
         break;
         case 'A':
            eye = eye + speed_factor*left_dir;
            center = center + speed_factor*left_dir;
         break;
         case 'D':
            eye = eye + speed_factor*right_dir;
            center = center + speed_factor*right_dir;
      }

      float angle_change = this->angle_change_per_second*time_delta;

      if(key == 'J'){
         angle_side -= angle_change;
         change_cam_orientation();
      }
      if(key == 'L'){
         angle_side += angle_change;
         change_cam_orientation();
      }
      if(key == 'I'){
         if(angle_up-angle_change > 0.0f)
            angle_up -= angle_change;
         change_cam_orientation();
      }
      if(key == 'K'){
         if(angle_up+angle_change < 3.14f)
            angle_up += angle_change;

         change_cam_orientation();
      }
   }

   virtual void update_pos(){
      change_cam_orientation();
   }

protected:
   float angle_up;
   float angle_side;

   float angle_change_per_second;

   //units per seconds
   float speed;

   virtual void change_cam_orientation(){

      float x = sin(angle_up)*cos(angle_side);
      float y = cos(angle_up);
      float z = sin(angle_up)*sin(angle_side);

      glm::vec3 direction(x, y, z);

      center = eye + direction;
   }
};
