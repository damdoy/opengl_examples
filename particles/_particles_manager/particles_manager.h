#ifndef PARTICLES_MANAGER
#define PARTICLES_MANAGER

#include "drawable.h"

class Particles_manager : public Drawable{

public:
   Particles_manager(){

   }

   ~Particles_manager(){

   }

   void init(uint nb_particles, GLuint pid){

      srand(time(0));

      _pid = pid;

      this->nb_particles = nb_particles;
      life_sec_min = 1;
      life_sec_max = 2;

      vao_particles = 0;

      wind_func = Particles_manager::default_wind;

      glGenVertexArrays(1, &vao_particles);
      glBindVertexArray(vao_particles);

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
         part.life_remaining = 0;
         part.age = 0;
         part.inital_speed_x = 0;
         part.inital_speed_y = 0;
         part.inital_speed_z = 0;
         lst_particles.push_back(part);
      }

      glBufferData(GL_ARRAY_BUFFER, nb_particles*3*sizeof(GLfloat), particles_positions, GL_DYNAMIC_DRAW);

      //contains 3d position of the particle
      GLuint pos_id = glGetAttribLocation(_pid, "pos_point");

      glEnableVertexAttribArray(pos_id);
      glVertexAttribPointer(pos_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glVertexAttribDivisor(pos_id, 1);

      //particle life buffer
      glGenBuffers(1, &_vbo_points_life);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_points_life);

      particles_life = new GLfloat[nb_particles*2];

      for (size_t i = 0; i < nb_particles; i++) {
         particles_life[i*2] = 0.0f; //life remaining of the particle
         particles_life[i*2+1] = 0.0f; //age of the particle
         lst_particles[i].life_remaining = &(particles_life[i*2]);
         lst_particles[i].age = &(particles_life[i*2+1]);
      }

      glBufferData(GL_ARRAY_BUFFER, nb_particles*2*sizeof(GLfloat), particles_life, GL_DYNAMIC_DRAW);

      GLuint age_id = glGetAttribLocation(_pid, "life_age");

      glEnableVertexAttribArray(age_id);
      glVertexAttribPointer(age_id, 2, GL_FLOAT, GL_FALSE, 0, NULL);

      glVertexAttribDivisor(age_id, 1);

      glBindVertexArray(0);

   }

   void set_emiter_boundary(GLfloat minx, GLfloat maxx, GLfloat miny, GLfloat maxy, GLfloat minz, GLfloat maxz){
      emiter_boudary_min_x = minx;
      emiter_boudary_max_x = maxx;
      emiter_boudary_min_y = miny;
      emiter_boudary_max_y = maxy;
      emiter_boudary_min_z = minz;
      emiter_boudary_max_z = maxz;
   }

   //will be rand between the two durations
   void set_life_duration_sec(float life_sec_min, float life_sec_max){
      this->life_sec_min = life_sec_min;
      this->life_sec_max = life_sec_max;
   }

   //must be called every draw loop
   void set_time(float time){
      this->current_time = time;
   }

   //variation to set init_vel_x (+- variation)
   void set_initial_velocity(float init_vel_x, float init_vel_y, float init_vel_z, float variation_x, float variation_y, float variation_z){
      this->init_vel_x = init_vel_x;
      this->init_vel_y = init_vel_y;
      this->init_vel_z = init_vel_z;

      for (size_t i = 0; i < nb_particles; i++) {
         float rand_val = (rand()%1000)/1000.0f;
         lst_particles[i].inital_speed_x = mix(init_vel_x-variation_x, init_vel_x+variation_x, rand_val);
         rand_val = (rand()%1000)/1000.0f;
         lst_particles[i].inital_speed_y = mix(init_vel_y-variation_y, init_vel_y+variation_y, rand_val);
         rand_val = (rand()%1000)/1000.0f;
         lst_particles[i].inital_speed_z = mix(init_vel_z-variation_z, init_vel_z+variation_z, rand_val);
      }
   }

   void set_wind_func( void (*wind_func)(float[3], float[3], float)){
      this->wind_func = wind_func;
   }

   void clean(){
      delete[] particles_positions;
   }


   void draw(){

      handle_particles();

      //sends the buffer to the gpu again
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_points_pos);
      glBufferData(GL_ARRAY_BUFFER, nb_particles*3*sizeof(GLfloat), particles_positions, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindBuffer(GL_ARRAY_BUFFER, _vbo_points_life);
      glBufferData(GL_ARRAY_BUFFER, nb_particles*2*sizeof(GLfloat), particles_life, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glUseProgram(_pid);
      glBindVertexArray(vao_particles);

      //these uniform were called for every plane
      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDisable(GL_CULL_FACE);

      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, nb_particles);

      glEnable(GL_CULL_FACE);
      glBindVertexArray(0);
      glUseProgram(0);

   }

protected:

   struct Particle {
      float *position;
      float inital_speed_x;
      float inital_speed_y;
      float inital_speed_z;
      float *life_remaining;
      float *age;
   };

   std::vector<Particle> lst_particles;

   uint nb_particles;

   float life_sec_min;
   float life_sec_max;
   float current_time;
   float prev_time;

   GLfloat *particles_positions;
   GLfloat *particles_life;

   GLuint _pid;
   GLuint vao_particles;

   GLuint _vbo;
   GLuint _vbo_tex;
   GLuint _vbo_points_pos;
   GLuint _vbo_points_life;

   GLfloat emiter_boudary_min_x;
   GLfloat emiter_boudary_max_x;
   GLfloat emiter_boudary_min_y;
   GLfloat emiter_boudary_max_y;
   GLfloat emiter_boudary_min_z;
   GLfloat emiter_boudary_max_z;

   //initial velocities
   GLfloat init_vel_x;
   GLfloat init_vel_y;
   GLfloat init_vel_z;

   void (*wind_func)(float pos[3], float ret[3], float time);

   void handle_particles(){
      float time_diff = current_time-prev_time;
      for (size_t i = 0; i < nb_particles; i++) {

         if(*lst_particles[i].life_remaining <= 0){ //create new particle
            float rand_val = (rand()%1000)/1000.0f;
            lst_particles[i].position[0] = mix(emiter_boudary_min_x, emiter_boudary_max_x, rand_val);
            rand_val = (rand()%1000)/1000.0f;
            lst_particles[i].position[1] = mix(emiter_boudary_min_y, emiter_boudary_max_y, rand_val);
            rand_val = (rand()%1000)/1000.0f;
            lst_particles[i].position[2] = mix(emiter_boudary_min_z, emiter_boudary_max_z, rand_val);
            rand_val = (rand()%1000)/1000.0f;
            *lst_particles[i].life_remaining = mix(life_sec_min, life_sec_max, rand_val);
            *lst_particles[i].age = 0.0f;
         }
         else{ //update particle
            float wind[3];
            (*wind_func)(lst_particles[i].position, wind, this->current_time);
            lst_particles[i].position[0] += time_diff*(lst_particles[i].inital_speed_x+wind[0]);
            lst_particles[i].position[1] += time_diff*(lst_particles[i].inital_speed_y+wind[1]);
            lst_particles[i].position[2] += time_diff*(lst_particles[i].inital_speed_z+wind[2]);
            *lst_particles[i].life_remaining -= time_diff;
            *lst_particles[i].age += time_diff;

         }
      }
      prev_time = current_time;
   }

   float mix(float min, float max, float ratio){
      return min*(1.0f-ratio)+max*ratio;
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
