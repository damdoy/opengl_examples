#version 330 core
uniform sampler2D tex;
uniform sampler2D ao_tex;
//uniform sampler2DMS tex;
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

float get_ao_blur(vec2 center){
   float ao_out = 0.0;
   for(int i = 0; i < blurring_mat_med_size; i++){
      int rel_i = i-(blurring_mat_med_size-1)/2;
      float sum = 0.0;
      for(int j = 0; j < blurring_mat_med_size; j++){
         int rel_j = j-(blurring_mat_med_size-1)/2;
         float matrix_val = blurring_mat_med[i]*blurring_mat_med[j];
         ao_out += matrix_val*texture(ao_tex, uv+vec2(rel_i/tex_width,rel_j/tex_height)).r;
      }
   }
   return ao_out*blurring_mat_med_coef;
}

void main() {

   vec3 color_out = vec3(0.5, 1.0, 0.0);

   if(effect_select == 0u){ //normal
      color_out = texture(tex, uv).rgb*get_ao_blur(uv);
   }

   color = color_out;
}
