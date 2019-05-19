#pragma once

#include <cstdio>
#include <IL/il.h>

class Texture{
public:
   Texture(){
      tex_data = NULL;
   }

   ~Texture(){
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

      tex_data = new unsigned char[tex_width*tex_height*3];

      ilCopyPixels(0, 0, 0, tex_width, tex_height, 1, IL_RGB, IL_UNSIGNED_BYTE, tex_data);

      //unbind the image as the image data has been copyied
      ilBindImage(0);
      ilDeleteImage(tex_id);

      return true;
   }

   //return the data of the image in bytes
   unsigned char *get_tex_data() const {
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
   ILint tex_width;
   ILint tex_height;
   unsigned char *tex_data;
};
