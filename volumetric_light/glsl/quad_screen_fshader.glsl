#version 330 core
uniform sampler2D tex;
uniform sampler2D depth_tex;
uniform sampler2D shadow_map_depth_tex;
uniform float tex_width;
uniform float tex_height;
in vec2 uv;
out vec3 color;

uniform uint effect_select;
uniform float fov_angle;

uniform mat4 camera_transform_matrix;
uniform mat4 proj_transform_matrix;
//contains projection + camera transf
uniform mat4 shadow_map_transform_mat;

void main() {
   vec3 color_out = vec3(0.5, 1.0, 0.0);

   //method to find world coord from camera pixel coords: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
   float aspect_ratio = tex_width/tex_height;

   float point_x = (2*uv.x-1)*aspect_ratio*tan(fov_angle/2);
   float point_y = (2*uv.y-1)*tan(fov_angle/2);

   vec3 vector_pixel = vec3(point_x, point_y, -1);

   //find world pos of the pixel and the camera origin, then we have the vector of the ray in world
   vec3 vector_pixel_world = vec3(inverse(camera_transform_matrix)*vec4(vector_pixel, 1.0));
   vec3 vector_origin_world = vec3(inverse(camera_transform_matrix)*vec4(0.0, 0.0, 0.0, 1.0));
   vec3 vector_dir = vector_pixel_world-vector_origin_world;

   if(effect_select == 0u){ //normal
      color_out = texture(tex, uv).rgb;
      vec3 cur_vec = vector_pixel_world;

      mat4 PV_mat = proj_transform_matrix*camera_transform_matrix;

      //this will increase resolution of volumetric light but be computationally heavy
      int max_iterations = 300;

      for(int i = 0; i < max_iterations; i++){
         vec4 end_vec = PV_mat*vec4(cur_vec, 1.0);

         //find corresponding pixel of the 3d world point on the shadow depth map
         vec4 shadow_coord_tex = shadow_map_transform_mat*vec4(cur_vec, 1.0);

         float fog = 0;
         float fog_altitude = 5  ;

         //have some fog effect at low altitude
         float fog_calc = 1;
         if(cur_vec.y > fog_altitude-1 && cur_vec.y < fog_altitude){
            fog_calc = fog_altitude-cur_vec.y;
         }
         else if(cur_vec.y >= fog_altitude){
            fog_calc = 0;
         }

         if(fog_calc > 0){
            //bias for the shadow map, helps against aliasing
            float BIAS = 0.002;
            //bias for the camera depth, avoid seeing lights "through" the objects
            float BIAS_DEPTH = 0.02;
            //check that the end of the vector is not further than the depth map (behind an object)
            if(end_vec.z/end_vec.w+BIAS_DEPTH < texture(depth_tex, uv).r){
               //check that the ray from the end vector to the light has no obstruction with the help of the shadow depth map
               if ( texture( shadow_map_depth_tex, (shadow_coord_tex.xy/shadow_coord_tex.w) ).x > shadow_coord_tex.z/shadow_coord_tex.w-BIAS){
                  //if ray not obstructed from cam to end_vec and then to light, then add a bit of colour (vol light)
                  color_out += fog_calc*vec3(0.016);
               }
            }
         }
         //increment for the ray tracing
         cur_vec = cur_vec + 0.15*vector_dir;
      }
   }

   color = color_out;
}
