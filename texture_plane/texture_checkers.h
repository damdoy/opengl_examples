#include "texture.h"


class Texture_checkers : public Texture{
public:
   Texture_checkers(){

   }

   ~Texture_checkers(){
      //super destructor will be called automatically
   }

   //n = number of checkers on one side
   void init(int n){
      this->tex_width = n;
      this->tex_height = n;
      
      this->tex_data = new unsigned char[n*n*3];

      for(int i = 0; i < n; i++){
         for(int j = 0; j < n; j++){
            int ind = (i*n+j)*3;
            if((i+j)%2 == 0){
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
