#ifndef TRUNK_H
#define TRUNK_H

#include "drawable.h"
#include "transform.h"

//a cylinder with recursive sticks branching out
class Trunk : public Drawable{
public:

   virtual ~Trunk(){
      glDeleteBuffers(1, &_vbo_pos);
      glDeleteBuffers(1, &_vbo_sur_norm);
      glDeleteBuffers(1, &_vbo_idx);
      glDeleteBuffers(1, &_vbo_tex);
      delete[] positions;
   }

   virtual void init(){
      _pid = load_shaders("trunk_vshader.glsl", "trunk_fshader.glsl");
      if(_pid == 0) exit(-1);
      init(_pid);
   }

   void init(GLuint pid){
      glGenVertexArrays(1, &_vao);
      init(pid, _vao);
   }

   void create_positions(){

      const uint nb_vertices = 6;

      nb_positions = 3*nb_vertices*2+2*3;
      positions = new GLfloat[nb_positions];
      uint index = 0;

      positions[index] = 0;
      positions[index+1] = -1.0f/2.0f;
      positions[index+2] = 0;
      index+=3;
      positions[index] = 0;
      positions[index+1] = 1.0f/2.0f;
      positions[index+2] = 0;
      index+=3;

      // const float bigger_base = 1.33; //to make base of trunk slightly larger
      //TODO: test this
      const float bigger_base = 1.1;

      for (size_t i = 0; i < nb_vertices; i++) {
         float angle_val = 3.1415f*2.0f*(1.0f/nb_vertices)*i;
         positions[index] = (sin(angle_val)/2.0f)*bigger_base;
         positions[index+1] = -1.0f/2.0f;
         positions[index+2] = (cos(angle_val)/2.0f)*bigger_base;
         index+=3;
         positions[index] = (sin(angle_val)/2.0f)*(1/bigger_base);
         positions[index+1] = 1.0f/2.0f;
         positions[index+2] = (cos(angle_val)/2.0f)*(1/bigger_base);
         index+=3;
      }

      nb_indices = 3*nb_vertices*2+3*nb_vertices*2;
      indices = new GLuint[nb_indices]; //3*2 per face, plus 3 per face for top and bottom

      index = 0;
      for (size_t i = 0; i < nb_vertices; i++) {
         uint start_idx_face = 2+2*i; // 12 points per face
         uint up_left = start_idx_face+1; //may be another one
         uint up_right = start_idx_face+3;
         if(up_right >= 2+2*nb_vertices){
            up_right = 2+1;
         }
         uint down_left = start_idx_face+0;
         uint down_right = start_idx_face+2;
         if(down_right >= 2+2*nb_vertices){
            down_right = 2;
         }

         //top
         indices[index] = 1;
         indices[index+1] = up_left;
         indices[index+2] = up_right;
         index+=3;
         //side
         indices[index] = up_left;
         indices[index+1] = down_left;
         indices[index+2] = down_right;
         index+=3;
         indices[index] = up_right;
         indices[index+1] = up_left;
         indices[index+2] = down_right;
         index+=3;
         //bottom
         indices[index] = 0;
         indices[index+1] = down_right;
         indices[index+2] = down_left;
         index+=3;
      }

      //per vertex textcoord
      nb_text_coord = 2*nb_vertices*2+2*2;
      text_coord = new GLfloat[nb_text_coord];

      index = 0;

      //dont care the top and bottom?
      text_coord[index] = 1;
      text_coord[index+1] = 1;
      index+=2;
      text_coord[index] = 0;
      text_coord[index+1] = 0;
      index+=2;

      for (size_t i = 0; i < nb_vertices; i++) {
         // i: 0 -> 0.5 == 0 -> 1
         // i: 0.5 -> 1.0 == 1 -> 0

         float relative_pos = float(i)/float(nb_vertices);
         if ( relative_pos < 0.5f){
            relative_pos = relative_pos*2;
         }
         else{ //relative_pos > 0.5f (0.5=1, 1.0=0)
            relative_pos = relative_pos*(-2)+2;
         }

         text_coord[index] = relative_pos; //bottom
         text_coord[index+1] = 1.0f;
         index+=2;
         text_coord[index] = relative_pos; //top
         text_coord[index+1] = 0.0f;
         index+=2;
      }

      //per vertex normal
      nb_normals = 3*nb_vertices*2+2*3;
      normals = new GLfloat[nb_normals];

      index = 0;

      normals[index] = 0.0f;
      normals[index+1] = -1.0f;
      normals[index+2] = 0.0f;
      index+=3;
      normals[index] = 0.0f;
      normals[index+1] = 1.0f;
      normals[index+2] = 0.0f;
      index+=3;

      for (size_t i = 0; i < nb_vertices; i++) {
         float angle_val = 3.1415f*2.0f*(1.0f/nb_vertices)*i;
         normals[index] = sin(angle_val);
         normals[index+1] = 0.0f;
         normals[index+2] = cos(angle_val);
         index+=3;
         normals[index] = sin(angle_val);
         normals[index+1] = 0.0f;
         normals[index+2] = cos(angle_val);
         index+=3;
      }
   }

   void init(GLuint pid, GLuint _vao){

      create_positions();

      this->_pid = pid;

      glUseProgram(_pid);

      glBindVertexArray(_vao);

      glGenBuffers(1, &_vbo_pos);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
      glBufferData(GL_ARRAY_BUFFER, nb_positions*sizeof(GLfloat), positions, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, nb_normals*sizeof(GLfloat), normals, GL_STATIC_DRAW);

      id_pos = glGetAttribLocation(_pid, "normals");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_indices*sizeof(GLuint), indices, GL_STATIC_DRAW);


      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, nb_text_coord*sizeof(GLfloat), text_coord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindVertexArray(0);

      // //main trunk for the fractal model
      // Transform t;
      // t.translate(0,7,0);
      // t.scale(1.5, 14, 1.5);
      // transf.push_back(t);
      // add_sub_trunks_fractal(t, 0, 4);

      //have fixed random values for each iteration of the trunks
      for (size_t i = 0; i < 32; i++) {
         rand_vals.push_back( (rand()%1000)/1000.0f);
      }

      //first trunk (axiom or root of the lsystem)
      Transform t;
      t.translate(0, 0.5, 0);
      t.scale(1.5/14, 1, 1.5/14);
      // add_sub_trunks_lsystem(t, 0, 5, true);
      add_sub_trunks_lsystem_random(t, 0, 5, true);
   }

   //fixed lsystem trunks
   //for debug/showing purposes
   void add_sub_trunks_lsystem(Transform t, uint level, uint max_level, bool is_end_point){
      if(level >= max_level){
         transf.push_back(t);

         //model matrix for the leaves
         if(is_end_point){
            //take a point at the top of an untransformed trunk, apply the transform on it
            //position of new point is a translation matrix
            glm::vec4 point = glm::vec4(0.0, 0.5, 0.0, 1.0);
            point = t.get_matrix()*point;
            Transform end_point_translation;
            end_point_translation.translate(point[0], point[1], point[2]);
            end_point_matrices.push_back(end_point_translation);
         }
         return;
      }

      Transform t0;
      t0.translate(0, 0, 0);
      t0.scale(1, 1, 1);
      t0.mult(t);
      add_sub_trunks_lsystem(t0, level+1, max_level, false);

      Transform t1;
      t1.translate(0, 1, 0);
      t1.rotate(0.0f, 0.0f, 1.0f, -3.1415f/14.0f);
      t1.scale(0.9, 0.9, 0.9);
      t1.mult(t);
      add_sub_trunks_lsystem(t1, level+1, max_level, true);

      Transform t2;
      t2.translate(0, 1, 0);
      t2.rotate(1.0f, 0.0f, 0.0f, -3.1415f/10.0f);
      t2.rotate(0.0f, 0.0f, 1.0f, 3.1415f/10.0f);
      t2.scale(0.8, 0.8, 0.8);
      t2.mult(t);
      add_sub_trunks_lsystem(t2, level+1, max_level, true);

      Transform b0;
      b0.translate(0, 0.4, 0);
      b0.rotate(0.0f, 0.0f, 1.0f, 3.1415f/4.0f);
      b0.scale(0.6, 0.6, 0.6);
      b0.mult(t);
      add_sub_trunks_lsystem(b0, level+1, max_level, true);

      Transform b1;
      b1.translate(0, 0.7, 0);
      b1.rotate(1.0f, 0.0f, 0.0f, -3.1415f/4.0f);
      b1.scale(0.4, 0.4, 0.4);
      b1.mult(t);
      add_sub_trunks_lsystem(b1, level+1, max_level, true);

      Transform b2;
      b2.translate(0, 0.85, 0);
      b2.rotate(0.0f, 0.0f, 1.0f, -3.1415f/3.0f);
      b2.scale(0.4, 0.4, 0.4);
      b2.mult(t);
      add_sub_trunks_lsystem(b2, level+1, max_level, true);
   }

   float lerp(float val, float min, float max){
      return (1-val)*min+val*max;
   }

   void add_sub_trunks_lsystem_random(Transform t, uint level, uint max_level, bool is_end_point){
      if(level >= max_level){
         //model matrix for the trunk model
         transf.push_back(t);

         //model matrix for the leaves
         if(is_end_point){
            //take a point at the top of an untransformed trunk, apply the transform on it
            //position of new point is a translation matrix
            glm::vec4 point = glm::vec4(0.0, 0.5, 0.0, 1.0);
            point = t.get_matrix()*point;
            Transform end_point_translation;
            end_point_translation.translate(point[0], point[1], point[2]);
            end_point_matrices.push_back(end_point_translation);
         }

         return;
      }

      //base trunk
      Transform t0;
      t0.translate(0, 0, 0);
      t0.scale(1, 1, 1);
      t0.mult(t);
      add_sub_trunks_lsystem_random(t0, level+1000, max_level, false);

      //top trunk
      Transform t1;
      t1.translate(0, 0.98, 0);
      t1.rotate(0.0f, 1.0f, 0.0f, lerp(rand_vals[2], 0, 2*3.1415f));
      t1.rotate(0.0f, 0.0f, 1.0f, lerp(rand_vals[0], 3.1415f/14.0f, 3.1415f/10.0f));
      t1.rotate(1.0f, 0.0f, 0.0f, lerp(rand_vals[1], 3.1415f/14.0f, 3.1415f/10.0f));
      t1.scale(0.8, 0.8, 0.8);
      t1.mult(t);
      add_sub_trunks_lsystem_random(t1, level+1, max_level, true);

      //side trunks / twigs
      Transform b0;
      b0.translate(0, lerp(rand_vals[16], 0.5, 0.9), 0);
      b0.rotate(0.0f, 1.0f, 0.0f, lerp(rand_vals[8], 0, 2*3.1415f));
      b0.rotate(0.0f, 0.0f, 1.0f, lerp(rand_vals[6], 3.1415f/4.0f, 3.1415f/10.0f));
      b0.rotate(1.0f, 0.0f, 0.0f, lerp(rand_vals[7], 3.1415f/4.0f, 3.1415f/10.0f));
      b0.scale(0.6, 0.6, 0.6);
      b0.mult(t);
      add_sub_trunks_lsystem_random(b0, level+1, max_level, true);

      Transform b1;
      b1.translate(0, lerp(rand_vals[17], 0.5, 0.9), 0);
      b1.rotate(0.0f, 1.0f, 0.0f, lerp(rand_vals[11], 0, 2*3.1415f));
      b1.rotate(0.0f, 0.0f, 1.0f, lerp(rand_vals[9], 3.1415f/3.0f, 3.1415f/10.0f));
      b1.rotate(1.0f, 0.0f, 0.0f, lerp(rand_vals[10], 3.1415f/3.0f, 3.1415f/10.0f));
      b1.scale(0.5, 0.5, 0.5);
      b1.mult(t);
      add_sub_trunks_lsystem_random(b1, level+1, max_level, true);

      Transform b2;
      b2.translate(0, lerp(rand_vals[18], 0.5, 0.9), 0);
      b2.rotate(0.0f, 1.0f, 0.0f, lerp(rand_vals[14], 0, 2*3.1415f));
      b2.rotate(0.0f, 0.0f, 1.0f, lerp(rand_vals[12], 3.1415f/2.0f, 3.1415f/10.0f));
      b2.rotate(1.0f, 0.0f, 0.0f, lerp(rand_vals[13], 3.1415f/2.0f, 3.1415f/10.0f));
      b2.scale(0.4, 0.4, 0.4);
      b2.mult(t);
      add_sub_trunks_lsystem_random(b2, level+1, max_level, true);
   }

   //simple trunk model, fractal-like but unrealistic
   //this is not really a l-system
   void add_sub_trunks_fractal(Transform t, uint level, uint max_level){

      if(level >= max_level){
         return;
      }

      Transform up;
      up.translate(0, 14, 0);
      up.scale(0.5, 0.25, 0.5);
      up.mult(t);
      transf.push_back(up);

      Transform s1;
      s1.translate(0.3, 7+3.5, 0);
      s1.rotate(0.0f, 0.0f, 1.0f, -3.1415f/4.0f);
      s1.scale(0.5, 0.5, 0.5);
      s1.mult(t);
      transf.push_back(s1);

      Transform s2;
      s2.translate(-0.3, 7+3.5, 0);
      s2.rotate(0.0f, 0.0f, 1.0f, 3.1415f/4.0f);
      s2.scale(0.5, 0.5, 0.5);
      s2.mult(t);
      transf.push_back(s2);

      Transform s3;
      s3.translate(0, 7+3.5, -0.0);
      s3.rotate(1.0f, 0.0f, 0.0f, 3.1415f/4.0f);
      s3.scale(0.5, 0.5, 0.5);
      s3.mult(t);
      transf.push_back(s3);

      Transform s4;
      s4.translate(0, 7+3.5, 0.0);
      s4.rotate(1.0f, 0.0f, 0.0f, -3.1415f/4.0f);
      s4.scale(0.5, 0.5, 0.5);
      s4.mult(t);
      transf.push_back(s4);

      add_sub_trunks_fractal(up, level+1, max_level);
      add_sub_trunks_fractal(s1, level+1, max_level);
      add_sub_trunks_fractal(s2, level+1, max_level);
      add_sub_trunks_fractal(s3, level+1, max_level);
      add_sub_trunks_fractal(s4, level+1, max_level);
   }

   //to be added to a drawing list by the caller
   std::vector<glm::mat4> get_transf()
   {
      std::vector<glm::mat4> ret;
      for (size_t i = 0; i < transf.size(); i++) {
         ret.push_back(transf[i].get_matrix());
      }
      return ret;
   }

   uint indices_to_draw(){
      return nb_indices;
   }

   std::vector<glm::mat4> get_end_point_matrices()
   {
      std::vector<glm::mat4> ret;
      for (size_t i = 0; i < end_point_matrices.size(); i++) {
         ret.push_back(end_point_matrices[i].get_matrix());
      }
      return ret;
   }

   virtual void draw(){

      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDrawElements(GL_TRIANGLES, nb_indices, GL_UNSIGNED_INT, 0);

      glBindVertexArray(0);
   }

protected:
   GLuint _vao;
   GLuint _vbo_pos;
   GLuint _vbo_sur_norm;
   GLuint _vbo_idx;
   GLuint _vbo_tex;

   std::vector<Transform> transf;
   std::vector<Transform> end_point_matrices; //points where the twigs/trunks stops, to add leaves
   std::vector<float> rand_vals; //list of random values

   GLfloat *positions;
   uint nb_positions;

   GLuint *indices;
   uint nb_indices;

   GLfloat *text_coord;
   uint nb_text_coord;

   GLfloat *normals;
   uint nb_normals;
};

#endif
