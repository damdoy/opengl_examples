#pragma once

#include <cstdio>
#include <vector>

//texture that use float instead of uint to represent its values
class Texture_float{
public:
   Texture_float(){
      tex_data = NULL;
   }

   ~Texture_float(){
      if(tex_data != NULL){
         delete tex_data;
      }
   }

   void set_data(const std::vector<std::vector<float> > &tex_data){
      this->tex_height = tex_data[0].size();
      this->tex_width = tex_data.size();

      this->tex_data = new float[tex_height*tex_width];

      for(unsigned int i = 0; i < tex_data.size(); i++){
         for(unsigned int j = 0; j < tex_data[0].size(); j++){

            this->tex_data[i*tex_width+j] = tex_data[i][j];
         }
      }
   }

   //return the data of the image in bytes
   float *get_tex_data() const {
      return tex_data;
   }

   int get_width() const{
      return tex_width;
   }

   int get_height() const{
      return tex_height;
   }

protected:
   unsigned int tex_width;
   unsigned int tex_height;
   float *tex_data;
};
