#ifndef PLANE_SINE_H
#define PLANE_SINE_H

#include "_plane/plane.h"

class Plane_sine : public Plane{
public:

   Plane_sine(){
      this->nb_vertices_side = 50;//default
   }

   void init(){
      GLuint pid = load_shaders("plane_vshader.glsl", "plane_fshader.glsl");

      init(pid);
   }

   void set_nb_vertices_side(uint val){
      this->nb_vertices_side = val;
   }

   void init(GLuint pid){

      this->_pid = pid;
      if(_pid == 0) exit(-1);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      generate_geometry();

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, nb_vertices*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, nb_normals*sizeof(GLfloat), normals, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "surface_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_indices*sizeof(GLuint), indices, GL_STATIC_DRAW);

      //texture coord definition
      const GLfloat vtexcoord[] = { 0.0f, 1.0f,
                                    1.0f, 1.0f,
                                    1.0f, 0.0f,
                                    0.0f, 0.0f};

      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindVertexArray(0);
   }

   void init(GLuint pid, GLuint vao){
      this->_pid = pid;
      this->_vao = vao;
   }

   void draw(){
      glUseProgram(_pid);
      glBindVertexArray(_vao);

      glDisable(GL_CULL_FACE);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDrawElements(GL_TRIANGLES, nb_vertices_to_draw, GL_UNSIGNED_INT, 0);

      glEnable(GL_CULL_FACE);

      glUseProgram(0);
      glBindVertexArray(0);
   }

   float get_height(float pos_x, float pos_y){
      glm::vec4 vec = glm::vec4(pos_x, 0.0, pos_y, 1.0);
      glm::vec4 vec_inv = glm::inverse(this->model_matrix)*vec;
      float height = get_height_priv(vec_inv[0], vec_inv[2]);
      height = (this->model_matrix*glm::vec4(0.0f, height, 0.0f, 1.0f))[1];
      return height;
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

protected:
   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _vbo_sur_norm;
   GLuint _pid;
   GLuint _tex;
   GLuint _tex1;

   GLfloat *vertices;
   uint nb_vertices;
   GLuint *indices;
   uint nb_indices;
   GLfloat *normals;
   uint nb_normals;
   GLfloat *tex_coord;

   uint nb_vertices_side;
   uint nb_vertices_to_draw;

   float get_height_priv(float x, float z){
      return sin(x*5)+sin(z*5);
   }

   void generate_geometry()
   {
      double increment = 1.0/(nb_vertices_side-1);

      vertices = new GLfloat[nb_vertices_side*nb_vertices_side*3];
      nb_vertices = nb_vertices_side*nb_vertices_side*3;

      for (size_t y = 0; y < nb_vertices_side; y++) {
         for (size_t x = 0; x < nb_vertices_side; x++) {
            vertices[(y*nb_vertices_side+x)*3] = -1.0+2*increment*x;
            vertices[(y*nb_vertices_side+x)*3+1] = get_height_priv(-1.0+2*increment*x, -1.0+2*increment*y);
            vertices[(y*nb_vertices_side+x)*3+2] = -1.0+2*increment*y;
         }
      }

      uint nb_squares = nb_vertices_side-1;
      indices = new GLuint[nb_squares*nb_squares*6];
      nb_vertices_to_draw = nb_squares*nb_squares*6;
      nb_indices = nb_vertices_to_draw;
      uint indices_index = 0;

      //calcukation of the indices;
      //indices: 01234X
      //         56789X
      //take 105', 561, 437, 783

      for (size_t y = 0; y < nb_vertices_side-1; y+=1) {
         for (size_t x = 0; x < nb_vertices_side-1; x+=1) {
            uint this_vertice = y*nb_vertices_side+x;
            //one of the triangle
            indices[indices_index] = this_vertice+1; //top right
            indices[indices_index+1] = this_vertice;
            indices[indices_index+2] = this_vertice+nb_vertices_side;

            indices_index += 3;

            indices[indices_index] = this_vertice+nb_vertices_side;
            indices[indices_index+1] = this_vertice+nb_vertices_side+1;
            indices[indices_index+2] = this_vertice+1;

            indices_index += 3;
         }
      }

      normals = new GLfloat[nb_vertices_side*nb_vertices_side*3];
      nb_normals = nb_vertices_side*nb_vertices_side*3;
      for (size_t x = 0; x < nb_vertices_side; x++) {
         for (size_t y = 0; y < nb_vertices_side; y++) {
            normals[y*nb_vertices_side+x] = 0.0;
            normals[y*nb_vertices_side+x+1] = 1.0;
            normals[y*nb_vertices_side+x+2] = 0.0;
         }
      }

   }

};

#endif
