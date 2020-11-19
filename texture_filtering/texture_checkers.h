#include "texture.h"


class Texture_checkers : public Texture{
public:
   Texture_checkers(){
   }

   ~Texture_checkers(){
   }

   //n = number of checkers on one side
   void init(int size, int n){
      this->tex_width = size;
      this->tex_height = size;

      this->tex_data = new unsigned char[size*size*3];

      for(int i = 0; i < size; i++){
         for(int j = 0; j < size; j++){
            int ind = (i*size+j)*3;

            int square_x = i/n;
            int square_y = j/n;

            if((square_x+square_y)%2 == 0){
               tex_data[ind] = 0;
               tex_data[ind+1] = 0;
               tex_data[ind+2] = 0;
            }
            else{
               tex_data[ind] = 255;
               tex_data[ind+1] = 255;
               tex_data[ind+2] = 255;
            }
         }
      }
   }
};
