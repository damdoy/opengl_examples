#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective

class Transform{
public:
   Transform(){
      transform_mat = glm::mat4(1.0f); //identity
   }

   void scale(glm::vec3 vec){
      transform_mat = glm::scale(transform_mat, vec);
   }

   void scale(float x, float y, float z){
      this->scale(glm::vec3(x, y, z));
   }

   glm::mat4 get_matrix(){
      return transform_mat;
   }

   glm::vec3 transform_point(glm::vec3 point3){
      glm::vec4 point4 = glm::vec4(point3, 1.0f);
      point4 = transform_mat*point4;
      return glm::vec3(point4);
   }

   ~Transform(){
   }


protected:
   glm::mat4 transform_mat;
};

#endif
