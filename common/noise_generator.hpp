#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

#include <vector>

enum Noise_Function_select{
   NOISE_SELECT_LINEAR,
   NOISE_SELECT_EASE,
   NOISE_SELECT_PERLIN,
   NOISE_SELECT_VORONOI
};

class Noise_generator{
public:

   Noise_generator(){
      noise_start_seg = 4;
      noise_levels = 4;
      noise_start_factor = 0.7f;
      noise_factor = 0.4f;
      noise_func_select = NOISE_SELECT_PERLIN;
      rand_seed = 0;
   }

   void setup(uint noise_start_seg, uint noise_levels, float noise_start_factor, float noise_factor, Noise_Function_select noise){
      this->noise_start_seg = noise_start_seg;
      this->noise_levels = noise_levels;
      this->noise_start_factor = noise_start_factor;
      this->noise_factor = noise_factor;
      this->noise_func_select = noise;
   }

   void set_noise_level(unsigned int noise_level){
      this->noise_levels = noise_level;
   }

   void set_noise_function(Noise_Function_select noise_func){
      noise_func_select = noise_func;
   }

   std::vector<std::vector<float> > get_2D_noise(unsigned int size_2d_x, unsigned int size_2d_y, float min_x, float max_x, float min_y, float max_y){
      std::vector<std::vector<float> > ret_vec;

      ret_vec.resize(size_2d_y);
      for(uint i = 0; i < size_2d_y; i++){
         ret_vec[i].resize(size_2d_x);
         for(uint j = 0; j < size_2d_x; j++){

            //makes that the relatives go from 0 to 1
            GLfloat relative_x = (float(i)/(size_2d_x-1));
            //rel_z not y as y is up
            GLfloat relative_y = (float(j)/(size_2d_y-1));

            //linear interpolation to find needed position
            relative_x = (1.0f-relative_x)*min_x+relative_x*max_x;
            relative_y = (1.0f-relative_y)*min_y+relative_y*max_y;

            //unsigned int cur_pos = (j*sub_x+i)*3;
            ret_vec[i][j] = function_recurs_noise(relative_x, relative_y);
         }
      }
      return ret_vec;
   }

   float get_noise_val(float pos_x, float pos_y){
      return function_recurs_noise(pos_x, pos_y);
   }

   // Noise_generator& operator=(const Noise_generator& input){
   //    Noise_generator ret(input);
   //    return ret;
   // }

   // Noise_generator(const Noise_generator& old)
   // {
   //    size = old_str.size;
   //    s = new char[size+1];
   //    strcpy(s, old_str.s);
   // }

protected:

   unsigned int noise_start_seg;
   unsigned int noise_levels;
   float noise_start_factor;
   float noise_factor;
   Noise_Function_select noise_func_select;
   unsigned int rand_seed;

   //returns the noise value (height) for a given point (not recursive by the way)
   float function_recurs_noise(float x, float y){
      unsigned int segmentation = noise_start_seg; //2 = squares of 0.5 (1/2) (please be a power of two)
      float val_ret = 0.0f;
      float amplitude = noise_start_factor;

      for (uint i = 0; i < noise_levels; i++){
         switch(noise_func_select){
         case NOISE_SELECT_LINEAR:
            val_ret = (2.0f*function_frac_linear(x, y, segmentation)-1.0f)*amplitude+val_ret;
            break;
         case NOISE_SELECT_EASE:
            val_ret = (2.0f*function_frac_ease(x, y, segmentation)-1.0f)*amplitude+val_ret;
            break;
         case NOISE_SELECT_PERLIN:
            val_ret = (2.0f*function_frac_perlin(x, y, segmentation)-1.0f)*amplitude+val_ret;
            break;
         case NOISE_SELECT_VORONOI:
            val_ret = (2.0f*function_frac_voronoi(x, y, segmentation)-1.0f)*amplitude+val_ret;
            break;
         }

         segmentation *= 2;
         amplitude *= noise_factor;
      }
      return val_ret;
   }

   float function_frac_linear(float x, float y, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;

      float rand_val_00;
      float rand_val_01;
      float rand_val_10;
      float rand_val_11;

      find_rand_vals(x_corr, y_corr, square_size, &rand_val_00, &rand_val_01, &rand_val_10, &rand_val_11);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;

      float lin_x_0 = (1.0f-frac_x)*rand_val_00 + (frac_x)*rand_val_10;
      float lin_x_1 = (1.0f-frac_x)*rand_val_01+ (frac_x)*rand_val_11;
      float lin = (1.0f-frac_y)*lin_x_0+ (frac_y)*lin_x_1;

      return lin;
   }

   //random values but with a smoothing function
   float function_frac_ease(float x, float y, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;

      float rand_val_00;
      float rand_val_01;
      float rand_val_10;
      float rand_val_11;

      find_rand_vals(x_corr, y_corr, square_size, &rand_val_00, &rand_val_01, &rand_val_10, &rand_val_11);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;

      frac_x = 6*pow(frac_x, 5)-15*pow(frac_x, 4)+10*pow(frac_x, 3);
      frac_y = 6*pow(frac_y, 5)-15*pow(frac_y, 4)+10*pow(frac_y, 3);

      float lin_x_0 = (1.0f-frac_x)*rand_val_00 + (frac_x)*rand_val_10;
      float lin_x_1 = (1.0f-frac_x)*rand_val_01+ (frac_x)*rand_val_11;
      float lin = (1.0f-frac_y)*lin_x_0+ (frac_y)*lin_x_1;

      return lin;
   }

   //calculate the perlin noise value for a given xy point
   float function_frac_perlin(float x, float y, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;

      float rand_val_00;
      float rand_val_01;
      float rand_val_10;
      float rand_val_11;

      float gradient_00[2];
      float gradient_01[2];
      float gradient_10[2];
      float gradient_11[2];

      find_rand_vals(x_corr, y_corr, square_size, &rand_val_00, &rand_val_01, &rand_val_10, &rand_val_11);

      find_gradient_from_rand(rand_val_00*32, gradient_00);
      find_gradient_from_rand(rand_val_01*32, gradient_01);
      find_gradient_from_rand(rand_val_10*32, gradient_10);
      find_gradient_from_rand(rand_val_11*32, gradient_11);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;

      float vec_00[2] = {frac_x, frac_y}; // top left vector to pixel
      float vec_01[2] = {frac_x, -(1.0f-frac_y)}; // bottom left vector to pixel
      float vec_10[2] = { -(1.0f-frac_x) , frac_y}; // top right vector to pixel
      float vec_11[2] = { -(1.0f-frac_x), -(1.0f-frac_y)}; // bottom right vector to pixel

      float dot_00 = vec_00[0]*gradient_00[0]+vec_00[1]*gradient_00[1];
      float dot_01 = vec_01[0]*gradient_01[0]+vec_01[1]*gradient_01[1];
      float dot_10 = vec_10[0]*gradient_10[0]+vec_10[1]*gradient_10[1];
      float dot_11 = vec_11[0]*gradient_11[0]+vec_11[1]*gradient_11[1];

      frac_x = 6*pow(frac_x, 5)-15*pow(frac_x, 4)+10*pow(frac_x, 3);
      frac_y = 6*pow(frac_y, 5)-15*pow(frac_y, 4)+10*pow(frac_y, 3);

      float pixel_x1 = (1.0f-frac_x)*dot_00+frac_x*dot_10;
      float pixel_x2 = (1.0f-frac_x)*dot_01+frac_x*dot_11;
      float pixel = (1.0f-frac_y)*pixel_x1+frac_y*pixel_x2;
      pixel += 0.5f;

      return pixel;
   }

   float function_frac_voronoi(float x, float y, unsigned int){
      x = (x*0.5+0.5);
      y = (y*0.5+0.5);

      x *= 4;
      y *= 4;

      float p[2];
      float f[2];

      p[0] = floorf(x);
      p[1] = floorf(y);

      f[0] = fmod(x, 1.0f);
      f[1] = fmod(y, 1.0f);

      float res = 8.0f;
      int prec = 1;

      for( int j=-prec; j<=prec; j++ ){
         for( int i=-prec; i<=prec; i++ )
         {
            int b[2];
            b[0] = i;
            b[1] = j;

            float r[2];
            r[0] = b[0] - f[0] + fmod((p[0]+b[0]+p[1]+b[1])*1.13f, 1.0f);
            r[1] = b[1] - f[1] + fmod((p[0]+b[0]+p[1]+b[1])*1.74f, 1.0f);
            float d = r[0]*r[0]+r[1]*r[1];

            if(d < res){
               res = d;
            }
         }
      }
      return sqrt(res);
   }

   void find_gradient_from_rand(unsigned int rand_val, float gradient[2]){
      switch(rand_val%4){
      case 0:
         gradient[0] = 1.0f;
         gradient[1] = 1.0f;
         break;
      case 1:
         gradient[0] = -1.0f;
         gradient[1] = 1.0f;
         break;
      case 2:
         gradient[0] = 1.0f;
         gradient[1] = -1.0f;
         break;
      case 3:
         gradient[0] = -1.0f;
         gradient[1] = -1.0f;
         break;
      }
   }

   //find random values for a given point, the random values should be everytime the same
   //4 random values for perlin, find the vectors
   void find_rand_vals(float x_corr, float y_corr, float square_size, float *rand_00, float *rand_01, float *rand_10, float *rand_11){

      float nearest_square_x = x_corr-fmod(x_corr, square_size);
      float nearest_square_y = y_corr-fmod(y_corr, square_size);
      float nearest_square_x_2 = x_corr-fmod(x_corr, square_size) + square_size;
      float nearest_square_y_2 = y_corr-fmod(y_corr, square_size) + square_size;

      unsigned int seed_x = nearest_square_x*1024*11;
      unsigned int seed_y = nearest_square_y*1024*11;
      unsigned int seed_x_2 = nearest_square_x_2*1024*11;
      unsigned int seed_y_2 = nearest_square_y_2*1024*11;

      *rand_00 = rand2d(seed_x, seed_y)/(float)MAX_RAND;
      *rand_01 = rand2d(seed_x, seed_y_2)/(float)MAX_RAND;
      *rand_10 = rand2d(seed_x_2, seed_y)/(float)MAX_RAND;
      *rand_11 = rand2d(seed_x_2, seed_y_2)/(float)MAX_RAND;
   }

   void print_debug_vec3(float vec[3]){
      printf("(%f, %f, %f\n)", vec[0], vec[1], vec[2]);
   }

   static const unsigned int MAX_RAND = 0xFFFFFFFF;

   unsigned int rand(){
      static unsigned int seed = 0;
      unsigned int a = 1664525;
      unsigned int c = 1013904223;
      seed = (a * seed + c);
      return seed;
   }

   //deterministic rand, given a v value
   unsigned int rand(unsigned int v){
      unsigned int a = 1664525;
      unsigned int c = 1013904223;
      return ((a * (v+rand_seed) + c));
   }

   unsigned int rand2d(unsigned int x, unsigned int y){
      return rand(y+rand(x));
   }
};

#endif
