#ifndef CLOUD_PARTICLES_MANAGER_HPP
#define CLOUD_PARTICLES_MANAGER_HPP

#include <algorithm>
#include <thread>
#include <mutex>

#include "drawable.h"
#include "noise_generator_3d.hpp"

class Cloud_particles_manager : public Drawable{
public:
   Cloud_particles_manager(){
      clouds_amount_change = true;
   }

   ~Cloud_particles_manager(){
      thread_sort->join();
   }

   void init(uint nb_particles, GLuint pid){

      srand(time(0));

      _pid = pid;
      thread_running = false;

      wind_func = Cloud_particles_manager::default_wind;

      this->nb_particles = nb_particles;

      glGenVertexArrays(1, &vao_particles);
      glBindVertexArray(vao_particles);

      //white clouds with dark shadows
      light_colour[0] = 1.0f;
      light_colour[1] = 1.0f;
      light_colour[2] = 1.0f;

      shadow_colour[0] = 0.0f;
      shadow_colour[1] = 0.0f;
      shadow_colour[2] = 0.0f;

      shadow_factor = 0.3f;

      //index
      // 0 3
      // 1 2

      //particle is just a square
      GLfloat vpoint[] = {
         -0.5f, 0.0f, 0.5f, // 1
         0.5f, 0.0f, 0.5f, // 2
         -0.5f, 0.0f, -0.5f, // 0
         0.5f,  0.0f, -0.5f, // 3
      };

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

      //shape of the particle (square)
      GLuint vpoint_id = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
      glVertexAttribDivisor(vpoint_id, 0);

      //texture positions will allow the fragemnt shader to draw the particle
      const GLfloat vtexcoord[] = {
                                    0.0f, 1.0f, // 1
                                    1.0f, 1.0f, // 2
                                    0.0f, 0.0f, // 0
                                    1.0f, 0.0f, // 3
                                    };

      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);
      glVertexAttribDivisor(vtexcoord_id, 0);

      glGenBuffers(1, &_vbo_points_pos);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_points_pos);

      particles_positions = new GLfloat[3*nb_particles];

      for (size_t i = 0; i < nb_particles; i++) {
         Particle part;
         part.position = &(particles_positions[i*3]);
         lst_particles.push_back(part);
      }

      glBufferData(GL_ARRAY_BUFFER, nb_particles*3*sizeof(GLfloat), particles_positions, GL_DYNAMIC_DRAW);

      //contains 3d position of the particle
      GLuint pos_id = glGetAttribLocation(_pid, "pos_point");

      glEnableVertexAttribArray(pos_id);
      glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      //means that only one 3d position will be given per square (4 vertices)
      glVertexAttribDivisor(pos_id, 1);

      glGenBuffers(1, &_vbo_colour);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_colour);

      particles_colours = new float[nb_particles];

      for (size_t i = 0; i < nb_particles; i++) {
         lst_particles[i].colour = &(particles_colours[i]);
      }

      glBufferData(GL_ARRAY_BUFFER, nb_particles*sizeof(float), particles_colours, GL_DYNAMIC_DRAW);

      GLuint col_id = glGetAttribLocation(_pid, "particle_colour");

      glEnableVertexAttribArray(col_id);
      glVertexAttribPointer(col_id, 1, GL_FLOAT, GL_FALSE, 0, NULL);

      //means that only one 3d position will be given per square (4 vertices)
      glVertexAttribDivisor(col_id, 1);

      /// active particle: display or not a particle
      glGenBuffers(1, &_vbo_active);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_active);

      particles_active = new uint8_t[nb_particles];

      for (size_t i = 0; i < nb_particles; i++) {
         lst_particles[i].active = &(particles_active[i]);
      }

      glBufferData(GL_ARRAY_BUFFER, nb_particles*sizeof(uint8_t), particles_active, GL_DYNAMIC_DRAW);

      GLuint act_id = glGetAttribLocation(_pid, "particle_active");

      glEnableVertexAttribArray(act_id);
      glVertexAttribIPointer(act_id, 1, GL_UNSIGNED_BYTE, 0, NULL);

      //means that only one 3d position will be given per square (4 vertices)
      glVertexAttribDivisor(act_id, 1);


      ///random vals for particles, give them unique features
      glGenBuffers(1, &_vbo_random);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_random);

      particles_random = new float[nb_particles];

      for (size_t i = 0; i < nb_particles; i++) {
         lst_particles[i].random = &(particles_random[i]);
      }

      glBufferData(GL_ARRAY_BUFFER, nb_particles*sizeof(float), particles_random, GL_DYNAMIC_DRAW);

      GLuint rnd_id = glGetAttribLocation(_pid, "particle_random");

      glEnableVertexAttribArray(rnd_id);
      glVertexAttribPointer(rnd_id, 1, GL_FLOAT, GL_TRUE, 0, NULL);

      //means that only one 3d position will be given per square (4 vertices)
      glVertexAttribDivisor(rnd_id, 1);

      clouds_amount = 0.0f;
      prev_clouds_amount = 0.0f;

      glBindVertexArray(0);

   }

   //TODO must be called every draw loop
   void set_time(float time){
      this->current_time = time;
   }

   //0 = none, 1.0 = max
   void set_clouds_amount(float amount){
      clouds_amount = amount-0.5f;
      if(clouds_amount != prev_clouds_amount){
         clouds_amount_change = true;
         prev_clouds_amount = clouds_amount;
      }
   }

   void set_wind_func( void (*wind_func)(float[3], float[3], float)){
      this->wind_func = wind_func;
   }

   void set_3d_noise_generator(Noise_generator_3d *noise_gen, uint noise_size){
      this->noise_size_3d = noise_size;

      // less height resolution for the noise
      float noise_bound_x = 3.5;
      float noise_bound_y = 2.0;
      float noise_bound_z = 3.5;
      noise_3d = noise_gen->get_3D_noise(noise_size_3d, noise_size_3d, noise_size_3d, 0, noise_bound_x, 0, noise_bound_y, 0, noise_bound_z);

      create_particles();
   }

   //2d noise gen for the alpha texture for the particles
   void set_2d_noise_generator(Noise_generator *noise_gen, uint noise_size){
      this->noise_size_2d = noise_size;

      noise_2d = noise_gen->get_2D_noise(noise_size_2d, noise_size_2d, 0, 2, 0, 2);

      noise_raw = new float[noise_size_2d*noise_size_2d];

      uint count = 0;
      for (size_t i = 0; i < noise_size; i++) {
         for (size_t j = 0; j < noise_size; j++) {
            noise_raw[count] = noise_2d[i][j];
            count++;
         }
      }

      glGenTextures(1, &_tex_noise_2d);
      glBindTexture(GL_TEXTURE_2D, _tex_noise_2d);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, noise_size_2d, noise_size_2d, 0, GL_RED, GL_FLOAT, noise_raw);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_noise_2d");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
   }

   void clean(){
      delete[] particles_positions;
   }

   void set_light_colour(float r, float g, float b){
      light_colour[0] = r;
      light_colour[1] = g;
      light_colour[2] = b;
   }

   void set_shadow_colour(float r, float g, float b){
      shadow_colour[0] = r;
      shadow_colour[0] = g;
      shadow_colour[0] = b;
   }

   //fact = 0, no shadow, at 1 very strong shadows (default 0.3)
   void set_shadow_factor(float fact){
      shadow_factor = fact;
   }

   void draw(){

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_noise_2d);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_noise_2d");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

      handle_particles();

      mtx.lock(); //access the particle memory

      //sends the buffer to the gpu again
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_points_pos);
      glBufferData(GL_ARRAY_BUFFER, nb_particles_to_draw*3*sizeof(GLfloat), particles_positions, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_colour);
      glBufferData(GL_ARRAY_BUFFER, nb_particles_to_draw*sizeof(float), particles_colours, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_active);
      glBufferData(GL_ARRAY_BUFFER, nb_particles_to_draw*sizeof(uint8_t), particles_active, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_random);
      glBufferData(GL_ARRAY_BUFFER, nb_particles_to_draw*sizeof(float), particles_random, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      mtx.unlock();

      glUseProgram(_pid);
      glBindVertexArray(vao_particles);

      glUniform3fv( glGetUniformLocation(_pid, "light_colour"), 1, this->light_colour);
      glUniform3fv( glGetUniformLocation(_pid, "shadow_colour"), 1, this->shadow_colour);

      //these uniform were called for every plane
      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDisable(GL_CULL_FACE);
      glEnable(GL_BLEND);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      //remove depth mask will mean clouds will not be drawn in front of one another ==> good with transparency
      glDepthMask(GL_FALSE);

      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, nb_particles_to_draw);

      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);

      glEnable(GL_CULL_FACE);
      glBindVertexArray(0);
      glUseProgram(0);

   }

protected:

   struct Particle {
      float *position;
      float *colour;
      uint8_t *active;
      float *random;
      float dist_to_cam;

      bool operator < (const Particle &other){
         return this->dist_to_cam > other.dist_to_cam;
      }
      //try sort by height => don't give a good result
      // bool operator < (const Particle &other){
      //    return this->position[1] < other.position[1];
      // }
   };

   std::vector<Particle> lst_particles;
   std::thread *thread_sort;
   bool thread_running;
   std::mutex mtx; //mutex for particle memory acces
   uint noise_size_2d;
   uint noise_size_3d;
   std::vector<std::vector<std::vector<float> > > noise_3d;
   std::vector<std::vector<float> > noise_2d;
   float *noise_raw;

   float light_colour[3];
   float shadow_colour[3];
   float shadow_factor;

   uint nb_particles;
   uint nb_particles_to_draw;
   float clouds_amount;
   bool clouds_amount_change;
   float prev_clouds_amount;

   float current_time;
   float prev_time;

   GLfloat *particles_positions;
   float *particles_colours;
   uint8_t *particles_active;
   float *particles_random;

   GLuint _pid;
   GLuint vao_particles;

   GLuint _vbo;
   GLuint _vbo_tex;
   GLuint _vbo_points_pos;
   GLuint _vbo_colour;
   GLuint _vbo_random;
   GLuint _vbo_active;

   GLuint _tex_noise_2d;

   //saves the texture for a single cloud particle
   GLuint _tex_2d_cloud;

   void (*wind_func)(float pos[3], float ret[3], float time);

   void handle_particles(){
      float time_diff = current_time-prev_time;

      mtx.lock(); //simple solution: as we are touching particles

      std::vector<Particle>::iterator it = lst_particles.begin();
      while(it != lst_particles.end()){
         float wind[3];
         (*wind_func)(it->position, wind, this->current_time);
         it->position[0] += time_diff*wind[0];
         it->position[1] += time_diff*wind[1];
         it->position[2] += time_diff*wind[2];


         //activate pparticles and find shadows
         //will be -0.5..0.5
         float pos_x = it->position[0];
         float pos_y = it->position[1];
         float pos_z = it->position[2];

         float pos_x_norm = (pos_x+0.5 < 0)? 0 : pos_x+0.5;
         float pos_y_norm = (pos_y+0.5 < 0)? 0 : pos_y+0.5;
         float pos_z_norm = (pos_z+0.5 < 0)? 0 : pos_z+0.5;

         float noise_val = get_noise_adjusted(pos_x_norm, pos_y_norm, pos_z_norm);

         if(*it->active){
            glm::vec4 particle_pos_transf = glm::vec4(pos_x, pos_y, pos_z, 1.0);
            particle_pos_transf = model_matrix*particle_pos_transf;
            float distance_to_cam = sqrt(pow(particle_pos_transf[0]-camera_position[0], 2)+pow(particle_pos_transf[1]-camera_position[1], 2)+pow(particle_pos_transf[2]-camera_position[2], 2));
            it->dist_to_cam = distance_to_cam;
         }

         if(clouds_amount_change == 1){
            if(noise_val+clouds_amount > 0.0f){

               *it->active = 1;

               ///OBSCURITY as ray casting integral from the particle to the sun
               ///if ray is in clouds, increase the obscurity value
               float obscurity = 0;
               uint total_incr = 20; //more incr = better shadow granularity
               //move along the sun vector
               for (size_t increment = 0; increment < total_incr; increment++) {

                  //cast ray toward the sun, incrementally
                  float new_pos_x = pos_x+0.5 + sun_dir[0]*(float(increment)/total_incr);
                  float new_pos_y = pos_y+0.5 + sun_dir[1]*(float(increment)/total_incr);
                  float new_pos_z = pos_z+0.5 + sun_dir[2]*(float(increment)/total_incr);


                  if(new_pos_x < 0 || new_pos_x > 1){
                     continue;
                  }
                  if(new_pos_y < 0 || new_pos_y > 1){
                     continue;
                  }
                  if(new_pos_y < 0 || new_pos_y > 1){
                     continue;
                  }

                  float noise_val = get_noise_adjusted(new_pos_x, new_pos_y, new_pos_z);

                  //if ray increment intersect cloud, increase obscurity
                  if(noise_val+clouds_amount > 0.0f){
                     obscurity += 1;
                  }

               }
               //1 = clear 0 = completely in shadows
               *it->colour = 1.0f-shadow_factor*(obscurity/(float)total_incr);

               /////////////////////USE NOISE as colour
               // *it->colour = 1.0f-(noise_val+0.5);

            }
            else{
               *it->active = 0;
            }
         }


         it++;
      }
      mtx.unlock();

      prev_time = current_time;
      clouds_amount_change = 0;
      // sort_particles(nb_particles_to_draw); //without thread
      if(!thread_running){
         thread_running = true;
         thread_sort = new std::thread(&Cloud_particles_manager::sort_particles, this, nb_particles_to_draw);
      }
   }

   void create_particles(){

      srand(0);
      uint nb_particles_side = pow(nb_particles, 1.0/3.0); //cube
      uint count = 0;

      //more resolution on the x-z plane than the height (y)
      uint nb_particles_side_x = nb_particles_side*1.82;
      uint nb_particles_side_y = nb_particles_side*0.3;
      uint nb_particles_side_z = nb_particles_side*1.82;

      for (size_t i = 0; i < nb_particles_side_x; i++) {
         for (size_t j = 0; j < nb_particles_side_y; j++) {
            for (size_t k = 0; k < nb_particles_side_z; k++) {
               //will be 0..1
               float pos_x = i*1.0/(nb_particles_side_x);
               float pos_y = j*1.0/(nb_particles_side_y);
               float pos_z = k*1.0/(nb_particles_side_z);

               pos_x -= 0.50+0.02*(rand()/float(RAND_MAX)-1);
               pos_y -= 0.50+0.02*(rand()/float(RAND_MAX)-1);
               pos_z -= 0.50+0.02*(rand()/float(RAND_MAX)-1);
               // pos_x -= 0.5;
               // pos_y -= 0.5;
               // pos_z -= 0.5;


               // *lst_particles[count].active = 0;
               lst_particles[count].position[0] = pos_x;
               lst_particles[count].position[1] = pos_y;
               lst_particles[count].position[2] = pos_z;
               *lst_particles[count].active = 0;
               *lst_particles[count].random = rand()/float(RAND_MAX);
               // *lst_particles[count].random = 0.5; //deterministic

               count++;

            }
         }
      }

      nb_particles_to_draw = count;
      printf("will draw %d particles\n", nb_particles_to_draw);
   }

   void upgrade_particles(){

   }

   //norm height between 0.0 and 1.0
   float get_noise_adjusted(float pos_x, float pos_y, float pos_z){
      //find positions in the 3d array
      float pos_x_size = pos_x*(noise_size_3d-2);
      float pos_y_size = pos_y*(noise_size_3d-2);
      float pos_z_size = pos_z*(noise_size_3d-2);

      float noise_val = get_noise_lerp_3d(pos_x_size, pos_y_size, pos_z_size);

      //avoid abrupt change of clouds val at top and bottom
      if(pos_y < 0.3){
         noise_val -= 1.0-(pos_y/0.3);
      }
      if(pos_y > 0.7){
         noise_val -= 1.0-((1.0f-pos_y)/0.3);
      }

      return noise_val;
   }

   //sort the particles given their distance to cam ==> farther away are first to be drawn
   void sort_particles(uint nb_particles_to_sort){
      std::vector<Particle> local_lst_particles = lst_particles;
      std::sort(local_lst_particles.begin(), local_lst_particles.begin()+nb_particles_to_sort);

      //reorg particles in the array also
      float *new_particles_positions = new float[nb_particles_to_sort*3];
      float *new_particles_colours = new float[nb_particles_to_sort];
      uint8_t *new_particles_active = new uint8_t[nb_particles_to_sort];
      float *new_particles_random = new float[nb_particles_to_sort];
      for (size_t i = 0; i < nb_particles_to_sort; i++) {
         new_particles_positions[i*3+0] = local_lst_particles[i].position[0];
         new_particles_positions[i*3+1] = local_lst_particles[i].position[1];
         new_particles_positions[i*3+2] = local_lst_particles[i].position[2];
         new_particles_colours[i] = *local_lst_particles[i].colour;
         new_particles_active[i] = *local_lst_particles[i].active;
         new_particles_random[i] = *local_lst_particles[i].random;
         // new_particles_positions[i] = 1.0;
      }

      mtx.lock();
      memcpy(particles_positions, new_particles_positions, nb_particles_to_sort*3*sizeof(float));
      memcpy(particles_colours, new_particles_colours, nb_particles_to_sort*sizeof(float));
      memcpy(particles_active, new_particles_active, nb_particles_to_sort*sizeof(uint8_t));
      memcpy(particles_random, new_particles_random, nb_particles_to_sort*sizeof(float));
      mtx.unlock();

      delete[] new_particles_positions;
      delete[] new_particles_colours;
      delete[] new_particles_active;
      delete[] new_particles_random;

      thread_running = false;
   }

   float mix(float min, float max, float ratio){
      return min*(1.0f-ratio)+max*ratio;
   }

   //gets val from the noise array from float values ==> use lerp
   //ex if x = 124.5, will get 0.5*noise[124]+0.5*noise[125]
   float get_noise_lerp_3d(float x, float y, float z){
      uint uint_x = uint(floor(x));
      uint uint_y = uint(floor(y));
      uint uint_z = uint(floor(z));

      float frac_x = x-uint_x;
      float frac_y = y-uint_y;
      float frac_z = z-uint_z;

      float lin_x00 = (1.0-frac_x)*noise_3d[uint_x][uint_y][uint_z]+(frac_x)*noise_3d[uint_x+1][uint_y][uint_z];
      float lin_x10 = (1.0-frac_x)*noise_3d[uint_x][uint_y+1][uint_z]+(frac_x)*noise_3d[uint_x+1][uint_y+1][uint_z];
      float lin_xy0 = (1.0-frac_y)*lin_x00+(frac_y)*lin_x10;

      float lin_x01 = (1.0-frac_x)*noise_3d[uint_x][uint_y][uint_z+1]+(frac_x)*noise_3d[uint_x+1][uint_y][uint_z+1];
      float lin_x11 = (1.0-frac_x)*noise_3d[uint_x][uint_y+1][uint_z+1]+(frac_x)*noise_3d[uint_x+1][uint_y+1][uint_z+1];
      float lin_xy1 = (1.0-frac_y)*lin_x01+(frac_y)*lin_x11;

      float lin_xyz = (1.0-frac_z)*lin_xy0+(frac_z)*lin_xy1;

      return lin_xyz;
   }

   //no wind
   static void default_wind(float[3], float ret[3], float)
   {
      ret[0] = 0;
      ret[1] = 0;
      ret[2] = 0;
   }
};

#endif
