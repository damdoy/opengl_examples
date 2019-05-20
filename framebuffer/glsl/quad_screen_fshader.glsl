#version 330 core
uniform sampler2D tex;
uniform float tex_width;
uniform float tex_height;
in vec2 uv;
out vec3 color;

uniform uint effect_select;

const int blurring_mat_small_size = 3;
float blurring_mat_small[blurring_mat_small_size] = float[](1, 2, 1);
float blurring_mat_small_coef = 1.0/16.0;

const int blurring_mat_med_size = 5;
float blurring_mat_med[blurring_mat_med_size] = float[](1, 4, 6, 4, 1);
float blurring_mat_med_coef = 1.0/256.0;


float average(vec3 vec){
   return (vec.x+vec.y+vec.z)/3;
}

float col_to_gs(vec3 vec){
   return 0.21*vec.x + 0.72*vec.y + 0.07*vec.z;
}

void main() {

   vec3 color_out = vec3(0.5, 1.0, 0.0);

   if(effect_select == 0u){ //normal
      color_out = texture(tex, uv).rgb;
   }
   else if(effect_select == 1u){ //inverted colours
      color_out = 1.0-texture(tex, uv).rgb;
   }
   else if(effect_select == 2u){ //small gaussian approx
      color_out = vec3(0.0, 0.0, 0.0);
      for(int i = 0; i < blurring_mat_small_size; i++){
         int rel_i = i-(blurring_mat_small_size-1)/2;
         float sum = 0.0;
         for(int j = 0; j < blurring_mat_small_size; j++){
            int rel_j = j-(blurring_mat_small_size-1)/2;
            float matrix_val = blurring_mat_small[i]*blurring_mat_small[j];
            color_out += matrix_val*texture(tex, uv+vec2(rel_i/tex_width,rel_j/tex_height)).rgb;
         }
      }
      color_out = color_out*1/16.0;
   }
   else if(effect_select == 3u){ //medium gaussian approx
      color_out = vec3(0.0, 0.0, 0.0);
      for(int i = 0; i < blurring_mat_med_size; i++){
         int rel_i = i-(blurring_mat_med_size-1)/2;
         float sum = 0.0;
         for(int j = 0; j < blurring_mat_med_size; j++){
            int rel_j = j-(blurring_mat_med_size-1)/2;
            float matrix_val = blurring_mat_med[i]*blurring_mat_med[j];
            color_out += matrix_val*texture(tex, uv+vec2(rel_i/tex_width,rel_j/tex_height)).rgb;
         }
      }

      color_out = color_out*blurring_mat_med_coef;
   }
   else if(effect_select == 4u){ //big generated gaussian approx
      color_out = vec3(0.0, 0.0, 0.0);

      const int mat_size = 12;
      float variance = 10.0;
      float mean = mat_size/2.0;
      float blurring_mat_cust[mat_size];
      float coef = 0.0;

      //init the blurring mat
      for(int i = 0; i < mat_size; i++){
         float val = 1.0/sqrt(2*3.1415*variance);
         val = val*pow(2.7182818, -pow((i-mean), 2)/(2*variance));
         blurring_mat_cust[i] = val;
         coef += val;
      }

      coef = 1.0/(coef*coef);

      for(int i = 0; i < mat_size; i++){
         float rel_i = i-(mat_size-1.0)/2.0;
         float sum = 0.0;
         for(int j = 0; j < mat_size; j++){
            float rel_j = j-(mat_size-1.0)/2.0;
            float matrix_val = blurring_mat_cust[i]*blurring_mat_cust[j];
            color_out += matrix_val*texture(tex, uv+vec2(rel_i/tex_width,rel_j/tex_height)).rgb;
         }
      }

      color_out = color_out*coef;
   }
   else if(effect_select == 5u){ //sobel edge detection

      const int sobel_x_size = 3;
      float sobel_x[sobel_x_size*sobel_x_size] = float[](1, 0, -1, 2, 0, -2, 1, 0, -1);

      const int sobel_y_size = 3;
      float sobel_y[sobel_y_size*sobel_y_size] = float[](1, 2, 1, 0, 0, 0, -1, -2, -1);


      float edge_x = 0.0;
      float edge_y = 0.0;

      for(int i = 0; i < sobel_x_size; i++){
         float rel_i = i-(sobel_x_size-1.0)/2.0;

         for(int j = 0; j < sobel_y_size; j++){
            float rel_j = j-(sobel_y_size-1.0)/2.0;
            float grayscale_pixel = col_to_gs(texture(tex, uv+vec2(rel_i/tex_width,rel_j/tex_height)).rgb);
            edge_x += sobel_x[j*3+i]*grayscale_pixel;
            edge_y += sobel_y[j*3+i]*grayscale_pixel;
         }
      }

      color_out = vec3(sqrt(edge_x*edge_x+edge_y*edge_y));
   }
   else if(effect_select == 6u){ //uneven glass

      float move_x = sin(uv.x*500)*10;
      float move_y = sin(uv.y*500)*10;

      color_out = texture(tex, uv+vec2(move_x/tex_width,move_y/tex_height)).rgb;
   }

   color = color_out;
}
