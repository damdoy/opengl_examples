#pragma once

#include <cstdio>
#include <IL/il.h>

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

   //load image from disk
   bool load_image(const char *path){
      ilGenImages(1, &tex_id);
      ilBindImage(tex_id);
      ILboolean result = ilLoadImage(path);
      if(result == IL_FALSE){
         return false;
      }

      tex_width = ilGetInteger(IL_IMAGE_WIDTH);
      tex_height = ilGetInteger(IL_IMAGE_HEIGHT);

      tex_data = new float[tex_width*tex_height*3];
      ilCopyPixels(0, 0, 0, tex_width, tex_height, 1, IL_RGB, IL_FLOAT, tex_data);

      //unbind the image as the image data has been copyied
      ilBindImage(0);
      ilDeleteImage(tex_id);

      return true;
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
   ILuint tex_id;
   unsigned int tex_width;
   unsigned int tex_height;
   float *tex_data;
};
