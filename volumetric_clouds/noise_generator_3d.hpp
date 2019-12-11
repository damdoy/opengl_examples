#ifndef NOISE_GENERATOR_3D_HPP
#define NOISE_GENERATOR_3D_HPP

#include "noise_generator.hpp"
#include <vector>

class Noise_generator_3d {
public:
   Noise_generator_3d(){
      noise_start_seg = 4;
      noise_levels = 4;
      noise_start_factor = 0.7f;
      noise_factor = 0.4f;
      noise_func_select = NOISE_SELECT_LINEAR;
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

   std::vector<std::vector<std::vector<float> > > get_3D_noise(uint size_3d_x, uint size_3d_y, uint size_3d_z,
      float min_x, float max_x, float min_y, float max_y, float min_z, float max_z){
      std::vector<std::vector<std::vector<float> > > ret_vec;

      ret_vec.resize(size_3d_z);
      for (uint i = 0; i < size_3d_z; i++) {
         ret_vec[i].resize(size_3d_y);
         for(uint j = 0; j < size_3d_y; j++){
            ret_vec[i][j].resize(size_3d_x);
            for(uint k = 0; k < size_3d_x; k++){

               //makes that the relative vals go from 0 to 1
               GLfloat relative_x = (float(i)/(size_3d_x-1));
               GLfloat relative_y = (float(j)/(size_3d_y-1));
               GLfloat relative_z = (float(k)/(size_3d_z-1));

               //linear interpolation to find needed position
               relative_x = (1.0f-relative_x)*min_x+relative_x*max_x;
               relative_y = (1.0f-relative_y)*min_y+relative_y*max_y;
               relative_z = (1.0f-relative_z)*min_z+relative_z*max_z;

               ret_vec[i][j][k] = get_noise_val(relative_x, relative_y, relative_z);
            }
         }
      }
      return ret_vec;
   }

   float get_noise_val(float pos_x, float pos_y, float pos_z){

      return function_recurs_noise(pos_x, pos_y, pos_z);
   }

protected:
   unsigned int noise_start_seg;
   unsigned int noise_levels;
   float noise_start_factor;
   float noise_factor;
   Noise_Function_select noise_func_select;
   unsigned int rand_seed;

   //returns the noise value (height) for a given point (not recursive by the way)
   float function_recurs_noise(float x, float y, float z){
      unsigned int segmentation = noise_start_seg; //2 = squares of 0.5 (1/2) (please be a power of two)
      float val_ret = 0.0f;
      float amplitude = noise_start_factor;

      for (uint i = 0; i < noise_levels; i++){
         switch(noise_func_select){
         case NOISE_SELECT_LINEAR:
            val_ret = (2.0f*function_frac_linear(x, y, z, segmentation)-1.0f)*amplitude+val_ret;
            break;
         case NOISE_SELECT_EASE:
            val_ret = (2.0f*function_frac_ease(x, y, z, segmentation)-1.0f)*amplitude+val_ret;
            break;
         //cases not handled for 3d noise
      //    case NOISE_SELECT_PERLIN:
      //       val_ret = (2.0f*function_frac_perlin(x, y, segmentation)-1.0f)*amplitude+val_ret;
      //       break;
         case NOISE_SELECT_VORONOI:
            val_ret = (2.0f*function_frac_voronoi(x, y, z, segmentation)-1.0f)*amplitude+val_ret;
            break;
         default:
            val_ret = 1;
         }

      // val_ret = 1;

         segmentation *= 2;
         amplitude *= noise_factor;
      }
      return val_ret;
   }

   float function_frac_linear(float x, float y, float z, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;
      float z_corr = (z+1.0)/2.0;

      float rand_vals[8];

      find_rand_vals_3d(x_corr, y_corr, z_corr, square_size, rand_vals);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;
      float frac_z = fmod(z_corr, square_size)/square_size;

      float lin_xy_00 = (1.0f-frac_x)*rand_vals[0] + (frac_x)*rand_vals[1];
      float lin_xy_01 = (1.0f-frac_x)*rand_vals[2] + (frac_x)*rand_vals[3];
      float lin_0 = (1.0f-frac_y)*lin_xy_00 + (frac_y)*lin_xy_01;

      //on other plane z
      float lin_xy_10 = (1.0f-frac_x)*rand_vals[4] + (frac_x)*rand_vals[5];
      float lin_xy_11 = (1.0f-frac_x)*rand_vals[6] + (frac_x)*rand_vals[7];
      float lin_1 = (1.0f-frac_y)*lin_xy_10 + (frac_y)*lin_xy_11;

      float lin = (1.0f-frac_z)*lin_0 + (frac_z)*lin_1;

      return lin;
      // return lin_xy_00;
   }

   //random values but with a smoothing function
   float function_frac_ease(float x, float y, float z, unsigned int segmentation){
      float square_size = 1.0f/(segmentation);

      //x and y go from -1 to 1, correct that
      float x_corr = (x+1.0)/2.0;
      float y_corr = (y+1.0)/2.0;
      float z_corr = (z+1.0)/2.0;

      float rand_vals[8];

      find_rand_vals_3d(x_corr, y_corr, z_corr, square_size, rand_vals);

      float frac_x = fmod(x_corr, square_size)/square_size;
      float frac_y = fmod(y_corr, square_size)/square_size;
      float frac_z = fmod(z_corr, square_size)/square_size;

      frac_x = 6*pow(frac_x, 5)-15*pow(frac_x, 4)+10*pow(frac_x, 3);
      frac_y = 6*pow(frac_y, 5)-15*pow(frac_y, 4)+10*pow(frac_y, 3);
      frac_z = 6*pow(frac_z, 5)-15*pow(frac_z, 4)+10*pow(frac_z, 3);

      float lin_xy_00 = (1.0f-frac_x)*rand_vals[0] + (frac_x)*rand_vals[1];
      float lin_xy_01 = (1.0f-frac_x)*rand_vals[2] + (frac_x)*rand_vals[3];
      float lin_0 = (1.0f-frac_y)*lin_xy_00 + (frac_y)*lin_xy_01;

      //on other plane z
      float lin_xy_10 = (1.0f-frac_x)*rand_vals[4] + (frac_x)*rand_vals[5];
      float lin_xy_11 = (1.0f-frac_x)*rand_vals[6] + (frac_x)*rand_vals[7];
      float lin_1 = (1.0f-frac_y)*lin_xy_10 + (frac_y)*lin_xy_11;

      float lin = (1.0f-frac_z)*lin_0 + (frac_z)*lin_1;

      return lin;
   }

   //tentative voronoi for cloud generation, not as great as I thought
   float function_frac_voronoi(float x, float y, float z, unsigned int segmentation){
      float smallest_distance = 10000000000000; //safe assumption

      float square_size = 1.0f/(segmentation);

      int counter = 0;

      //for each cube around the current cube, find voronoi centre
      for (int i = -1; i < 2; i++) {
         for (int j = -1; j < 2; j++) {
            for (int k = -1; k < 2; k++) {
               float centre_pos[3];
               find_voronoi_centre_in_square(x+i*square_size, y+j*square_size, z+k*square_size, square_size, centre_pos);

               float distance = sqrt(pow(x-centre_pos[0], 2)+pow(y-centre_pos[1], 2)+pow(z-centre_pos[2], 2));
               if(distance < smallest_distance){
                  smallest_distance = distance;
               }

               counter++;
            }
         }
      }

      return (smallest_distance/square_size);
   }

   //from top left value in square, find voronoi centre
   void find_voronoi_centre_in_square(float x_corr, float y_corr, float z_corr, float square_size, float centre_pos[3]){
      float ret_val[8];
      find_rand_vals_3d(x_corr, y_corr, z_corr, square_size, ret_val);

      //ret_val[0] = cube top right random value
      float pos_x = ret_val[0];
      //get a random y pos from x, should *feel* random enough
      float pos_y = rand(ret_val[0]*MAX_RAND)/(float)MAX_RAND;
      float pos_z = rand(pos_y*MAX_RAND)/(float)MAX_RAND;

      float nearest_square_x = x_corr-fmod(x_corr, square_size);
      float nearest_square_y = y_corr-fmod(y_corr, square_size);
      float nearest_square_z = z_corr-fmod(z_corr, square_size);

      centre_pos[0] = nearest_square_x+pos_x*square_size;
      centre_pos[1] = nearest_square_y+pos_y*square_size;
      centre_pos[2] = nearest_square_z+pos_z*square_size;
   }

   //find random values for corners of the square that are consistent and repeteable
   void find_rand_vals_2d(float x_corr, float y_corr, float square_size, float *rand_00, float *rand_01, float *rand_10, float *rand_11){

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

   //find random values on the corners of the cube of dimm square_size³ around the x,y,z corr point
   //values for the cube in rand_vals
   void find_rand_vals_3d(float x_corr, float y_corr, float z_corr, float square_size, float rand_vals[8]){

      float nearest_square_x = x_corr-fmod(x_corr, square_size);
      float nearest_square_y = y_corr-fmod(y_corr, square_size);
      float nearest_square_z = z_corr-fmod(z_corr, square_size);
      float nearest_square_x_2 = x_corr-fmod(x_corr, square_size) + square_size;
      float nearest_square_y_2 = y_corr-fmod(y_corr, square_size) + square_size;
      float nearest_square_z_2 = z_corr-fmod(z_corr, square_size) + square_size;

      unsigned int seed_x = nearest_square_x*1013904223*11*7*1023;
      unsigned int seed_y = nearest_square_y*1013904223*11*7*1023;
      unsigned int seed_z = nearest_square_z*1013904223*11*7*1023;
      unsigned int seed_x_2 = nearest_square_x_2*1013904223*11*7*1023;
      unsigned int seed_y_2 = nearest_square_y_2*1013904223*11*7*1023;
      unsigned int seed_z_2 = nearest_square_z_2*1013904223*11*7*1023;
      // unsigned int seed_x = nearest_square_x*1024*11*7;
      // unsigned int seed_y = nearest_square_y*1024*11*7;
      // unsigned int seed_z = nearest_square_z*1024*11*7;
      // unsigned int seed_x_2 = nearest_square_x_2*1024*11*7;
      // unsigned int seed_y_2 = nearest_square_y_2*1024*11*7;
      // unsigned int seed_z_2 = nearest_square_z_2*1024*11*7;

      rand_vals[0] = rand3d(seed_x, seed_y, seed_z)/(float)MAX_RAND;
      rand_vals[1] = rand3d(seed_x_2, seed_y, seed_z)/(float)MAX_RAND;
      rand_vals[2] = rand3d(seed_x, seed_y_2, seed_z)/(float)MAX_RAND;
      rand_vals[3] = rand3d(seed_x_2, seed_y_2, seed_z)/(float)MAX_RAND;
      rand_vals[4] = rand3d(seed_x, seed_y, seed_z_2)/(float)MAX_RAND;
      rand_vals[5] = rand3d(seed_x_2, seed_y, seed_z_2)/(float)MAX_RAND;
      rand_vals[6] = rand3d(seed_x, seed_y_2, seed_z_2)/(float)MAX_RAND;
      rand_vals[7] = rand3d(seed_x_2, seed_y_2, seed_z_2)/(float)MAX_RAND;
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

   unsigned int rand2d(uint x, uint y){
      return rand(y+rand(x));
   }

   unsigned int rand3d(uint x, uint y, uint z){
      return rand(z+rand(y+rand(x)));
      // return rand2d(rand2d(x,y),rand2d(x,z));
      // return rand2d(x,y);
   }
};

#endif
