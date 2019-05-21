#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "_terrain_quad/qterrain.h"

class Camera_fps : public Camera_free{
public:

   void init(float height, QTerrain *terrain){
      set_height(height);
      set_terrain(terrain);

      this->update_pos();
   }

   void set_height(float height){
      this->height = height;
   }

   void set_terrain(QTerrain *terrain){
      this->terrain = terrain;
   }

   virtual void input_handling(char key){

      glm::vec3 direction;
      glm::vec3 speed_factor = glm::vec3(0.1f);

      direction = center - eye;
      direction = glm::vec3(direction[0], 0.0f, direction[2]);
      direction = normalize(direction);

      glm::vec3 left_dir = normalize(cross(up, direction));
      glm::vec3 right_dir = normalize(cross(direction, up));

      if(key == 'W' || key == 'A' || key == 'S' || key == 'D'){
         float height_before = eye[1];

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

         float height_diff = terrain->get_height(eye[0], eye[2]) - height_before;

         eye[1] += height_diff+height;
         center[1] += height_diff+height;
      }
      else {
         Camera_free::input_handling(key);
      }

   }

protected:
   QTerrain *terrain; //collision
   float height;
};
