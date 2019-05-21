#ifndef QTREE_H
#define QTREE_H

// 01
// 23

//avoid having a too precise noise gen level compared to the resolution of the terrain
//otherwise it will mess up the normals
#define OFFSET_NOISE_LEVEL 2

//#define MAX_LEVELS 4

//handle the roughness of the subterrains
//when two subterrains of different level are next to one another
#define LOD_GESTION 1

bool rough_array_init[4] = {false, false, false, false};

enum QSide{QSIDE_NORTH = 0, QSIDE_EAST = 1, QSIDE_SOUTH = 2, QSIDE_WEST = 3};

class Quad_tree_node{
public:
   Quad_tree_node(Quad_tree_node *parent, unsigned int subdivisions, unsigned int max_levels, unsigned int noise_generator_max_level){
      is_tail = true;
      is_root = false;
      this->parent = parent;
      rough_edges[0] = false;
      rough_edges[1] = false;
      rough_edges[2] = false;
      rough_edges[3] = false;
      this->subdivisions = subdivisions;
      this->max_levels = max_levels;
      this->noise_generator_max_level = noise_generator_max_level;
   }

   void set_as_root(Noise_generator noise_gen, GLuint _pid){
      this->parent = NULL;
      is_tail = true;
      is_root = true;
      rough_edges[0] = false;
      rough_edges[1] = false;
      rough_edges[2] = false;
      rough_edges[3] = false;
      this->start_x = -1.0;
      this->start_y = -1.0;
      this->end_x = 1.0;
      this->end_y = 1.0;
      this->position_id = 0;
      for (unsigned int j = 0; j <= 3; j++){
         this->children[j] = NULL;
      }
      this->terrain = new Terrain();
      this->level = 0;
      this->shader_pid = _pid;
      this->noise_gen = noise_gen;

      this->terrain->init(subdivisions, subdivisions, this->start_x, this->start_y, this->end_x, this->end_y, &this->noise_gen, this->shader_pid, rough_array_init);
   }

   void delete_root(){
      assert(is_root);

      this->merge_qtree();

      delete terrain;
   }

   //find neighbour_dir
   //returns the neigbour at the same level (or lower), NULL otherwise but never a higher level neighbour since which one should it returns?

   Quad_tree_node *find_neighbour_north(){
      if(position_id == 2){
         if(!is_root){
            return parent->children[0];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 3){
         if(!is_root){
            return parent->children[1];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 0){
         if(!is_root){
            Quad_tree_node *parent_north = parent->find_neighbour_north();
            if(parent_north != NULL && !parent_north->is_tail){
               return parent_north->children[2];
            }
            else{
               return parent_north;
            }
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 1){
         if(!is_root){
            Quad_tree_node *parent_north = parent->find_neighbour_north();
            if(parent_north != NULL && !parent_north->is_tail){
               return parent_north->children[3];
            }
            else{
               return parent_north;
            }
         }
         else{
            return NULL;
         }
      }
      return NULL;
   }

   Quad_tree_node *find_neighbour_east(){
      if(position_id == 2){
         if(!is_root){
            return parent->children[3];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 0){
         if(!is_root){
            return parent->children[1];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 1){
         if(!is_root){
            Quad_tree_node *parent_east = parent->find_neighbour_east();
            if(parent_east != NULL && !parent_east->is_tail){
               return parent_east->children[0];
            }
            else{
               return parent_east;
            }
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 3){
         if(!is_root){
            Quad_tree_node *parent_east = parent->find_neighbour_east();
            if(parent_east != NULL && !parent_east->is_tail){
               return parent_east->children[2];
            }
            else{
               return parent_east;
            }
         }
         else{
            return NULL;
         }
      }
      return NULL;
   }

   Quad_tree_node *find_neighbour_south(){
      if(position_id == 0){
         if(!is_root){
            return parent->children[2];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 1){
         if(!is_root){
            return parent->children[3];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 2){
         if(!is_root){
            Quad_tree_node *parent_south = parent->find_neighbour_south();
            if(parent_south != NULL && !parent_south->is_tail){
               return parent_south->children[0];
            }
            else{
               return parent_south;
            }
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 3){
         if(!is_root){
            Quad_tree_node *parent_south = parent->find_neighbour_south();
            if(parent_south != NULL && !parent_south->is_tail){
               return parent_south->children[1];
            }
            else{
               return parent_south;
            }
         }
         else{
            return NULL;
         }
      }
      return NULL;
   }

   Quad_tree_node *find_neighbour_west(){
      if(position_id == 3){
         if(!is_root){
            return parent->children[2];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 1){
         if(!is_root){
            return parent->children[0];
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 0){
         if(!is_root){
            Quad_tree_node *parent_west = parent->find_neighbour_west();
            if(parent_west != NULL && !parent_west->is_tail){
               return parent_west->children[1];
            }
            else{
               return parent_west;
            }
         }
         else{
            return NULL;
         }
      }
      else if(position_id == 2){
         if(!is_root){
            Quad_tree_node *parent_west = parent->find_neighbour_west();
            if(parent_west != NULL && !parent_west->is_tail){
               return parent_west->children[3];
            }
            else{
               return parent_west;
            }
         }
         else{
            return NULL;
         }
      }
      return NULL;
   }

   //break a subterrain into 4 subterrains
   void break_qtree(){

      if(this->level > this->max_levels){
         return;
      }

      this->is_tail = false;
      this->terrain->cleanup();
      delete this->terrain;
      this->terrain = NULL;

      for (unsigned int i = 0; i <= 3; i++){
         Quad_tree_node *cur_node = new Quad_tree_node(this, subdivisions, max_levels, noise_generator_max_level);
         cur_node->is_tail = true;
         cur_node->position_id = i;

         if(i == 0 || i == 2){
            cur_node->start_x = this->start_x;
            cur_node->end_x = (this->start_x+ this->end_x)/2.0f;
         }
         else if(i == 1 || i == 3){
            cur_node->start_x = (this->start_x+ this->end_x)/2.0f;
            cur_node->end_x = this->end_x;
         }

         if(i == 0 || i == 1){
            cur_node->start_y = this->start_y;
            cur_node->end_y = (this->start_y+ this->end_y)/2.0f;
         }
         else if(i == 2 || i == 3){
            cur_node->start_y = (this->start_y+ this->end_y)/2.0f;
            cur_node->end_y = this->end_y;
         }

         //printf("sub %u: %f, %f, %f, %f\n", i, cur_node->start_x, cur_node->start_y, cur_node->end_x, cur_node->end_y);

         cur_node->level = this->level+1;

         uint noise_level = this->log2(subdivisions)+cur_node->level-OFFSET_NOISE_LEVEL;
         if (noise_level > noise_generator_max_level)
            noise_level = noise_generator_max_level;

         cur_node->terrain = new Terrain();
         cur_node->noise_gen = this->noise_gen;
         cur_node->noise_gen.set_noise_level(noise_level);
         cur_node->shader_pid = this->shader_pid;
         cur_node->terrain->init(subdivisions, subdivisions, cur_node->start_x, cur_node->start_y, cur_node->end_x, cur_node->end_y, &cur_node->noise_gen, this->shader_pid, rough_array_init);
         for (unsigned int j = 0; j <= 3; j++){
            cur_node->children[j] = NULL;
         }
         this->children[i] = cur_node;
      }

#if LOD_GESTION == 1
      Quad_tree_node *north_neighbour = this->find_neighbour_north();
      Quad_tree_node *east_neighbour = this->find_neighbour_east();
      Quad_tree_node *south_neighbour = this->find_neighbour_south();
      Quad_tree_node *west_neighbour = this->find_neighbour_west();


      //if north is larger, need to roughen the north side
      if(north_neighbour != NULL && north_neighbour->is_tail && north_neighbour->level == this->level){
         this->children[0]->roughen_side(QSIDE_NORTH);
         this->children[1]->roughen_side(QSIDE_NORTH);
      }

      if(east_neighbour != NULL && east_neighbour->is_tail && east_neighbour->level == this->level){
         this->children[1]->roughen_side(QSIDE_EAST);
         this->children[3]->roughen_side(QSIDE_EAST);
      }

      // if south neighbour
      if(south_neighbour != NULL && south_neighbour->is_tail && south_neighbour->level == this->level){
         this->children[2]->roughen_side(QSIDE_SOUTH);
         this->children[3]->roughen_side(QSIDE_SOUTH);
      }

      if(west_neighbour != NULL && west_neighbour->is_tail && west_neighbour->level == this->level){
         this->children[0]->roughen_side(QSIDE_WEST);
         this->children[2]->roughen_side(QSIDE_WEST);
      }


      //TODO apparently problem here
      //neighbour is now on the same level as children but is rough ==> need to remove that
      Quad_tree_node *north_neighbour_sub_0 = this->children[0]->find_neighbour_north();
      Quad_tree_node *north_neighbour_sub_1 = this->children[1]->find_neighbour_north();
      Quad_tree_node *east_neighbour_sub_0 = this->children[1]->find_neighbour_east();
      Quad_tree_node *east_neighbour_sub_1 = this->children[3]->find_neighbour_east();
      Quad_tree_node *south_neighbour_sub_0 = this->children[2]->find_neighbour_south();
      Quad_tree_node *south_neighbour_sub_1 = this->children[3]->find_neighbour_south();
      Quad_tree_node *west_neighbour_sub_0 = this->children[0]->find_neighbour_west();
      Quad_tree_node *west_neighbour_sub_1 = this->children[2]->find_neighbour_west();

      // if(south_neighbour_sub_0 != NULL && south_neighbour_sub_0->is_tail){
      //    printf("south: NOT NULL AND IS TAIL neigh level: %d, child level: %d\n", south_neighbour_sub_0->level, this->children[2]->level);
      // }

      if(north_neighbour_sub_0 != NULL && north_neighbour_sub_0->is_tail && north_neighbour_sub_0->level == this->children[0]->level && north_neighbour_sub_0->rough_edges[QSIDE_SOUTH] == true){
         north_neighbour_sub_0->unroughen_side(QSIDE_SOUTH);
      }

      if(north_neighbour_sub_1 != NULL && north_neighbour_sub_1->is_tail && north_neighbour_sub_1->level == this->children[1]->level && north_neighbour_sub_1->rough_edges[QSIDE_SOUTH] == true){
         north_neighbour_sub_1->unroughen_side(QSIDE_SOUTH);
      }

      if(east_neighbour_sub_0 != NULL && east_neighbour_sub_0->is_tail && east_neighbour_sub_0->level == this->children[1]->level && east_neighbour_sub_0->rough_edges[QSIDE_WEST] == true){
         east_neighbour_sub_0->unroughen_side(QSIDE_WEST);
      }

      if(east_neighbour_sub_1 != NULL && east_neighbour_sub_1->is_tail && east_neighbour_sub_1->level == this->children[3]->level && east_neighbour_sub_1->rough_edges[QSIDE_WEST] == true){
         east_neighbour_sub_1->unroughen_side(QSIDE_WEST);
      }

      if(south_neighbour_sub_0 != NULL && south_neighbour_sub_0->is_tail && south_neighbour_sub_0->level == this->children[2]->level && south_neighbour_sub_0->rough_edges[QSIDE_NORTH] == true){
        south_neighbour_sub_0->unroughen_side(QSIDE_NORTH);
      }

      if(south_neighbour_sub_1 != NULL && south_neighbour_sub_1->is_tail && south_neighbour_sub_1->level == this->children[3]->level && south_neighbour_sub_1->rough_edges[QSIDE_NORTH] == true){
        south_neighbour_sub_1->unroughen_side(QSIDE_NORTH);
      }

      if(west_neighbour_sub_0 != NULL && west_neighbour_sub_0->is_tail && west_neighbour_sub_0->level == this->children[0]->level && west_neighbour_sub_0->rough_edges[QSIDE_EAST] == true){
         west_neighbour_sub_0->unroughen_side(QSIDE_EAST);
      }

      if(west_neighbour_sub_1 != NULL && west_neighbour_sub_1->is_tail && west_neighbour_sub_1->level == this->children[2]->level && west_neighbour_sub_1->rough_edges[QSIDE_EAST] == true){
         west_neighbour_sub_1->unroughen_side(QSIDE_EAST);
      }

#endif
      //tells neighbours of parent to activate rough_edges
   }

   //from a parent, merge its children to form a new subterrain of lower level
   void merge_qtree(){
      if(this->is_tail){
         return;
      }

#if LOD_GESTION == 1
      Quad_tree_node *north_neighbour_sub_0 = this->children[0]->find_neighbour_north();
      Quad_tree_node *north_neighbour_sub_1 = this->children[1]->find_neighbour_north();

      Quad_tree_node *east_neighbour_sub_0 = this->children[1]->find_neighbour_east();
      Quad_tree_node *east_neighbour_sub_1 = this->children[3]->find_neighbour_east();

      Quad_tree_node *south_neighbour_sub_0 = this->children[2]->find_neighbour_south();
      Quad_tree_node *south_neighbour_sub_1 = this->children[3]->find_neighbour_south();

      Quad_tree_node *west_neighbour_sub_0 = this->children[0]->find_neighbour_west();
      Quad_tree_node *west_neighbour_sub_1 = this->children[2]->find_neighbour_west();

      //two cases: a noighbour is of same size and we merge current node ---> rough the neighbour up
      //       we merge it and a rough neighbour is at the same level ==> remove rough

      if(north_neighbour_sub_0 != NULL && north_neighbour_sub_0->is_tail && north_neighbour_sub_0->level == this->children[0]->level){
         north_neighbour_sub_0->roughen_side(QSIDE_SOUTH);
      }
      if(north_neighbour_sub_1 != NULL && north_neighbour_sub_1->is_tail && north_neighbour_sub_1->level == this->children[1]->level){
         north_neighbour_sub_1->roughen_side(QSIDE_SOUTH);
      }

      if(east_neighbour_sub_0 != NULL && east_neighbour_sub_0->is_tail && east_neighbour_sub_0->level == this->children[1]->level){
         east_neighbour_sub_0->roughen_side(QSIDE_WEST);
      }
      if(east_neighbour_sub_1 != NULL && east_neighbour_sub_1->is_tail && east_neighbour_sub_1->level == this->children[3]->level){
         east_neighbour_sub_1->roughen_side(QSIDE_WEST);
      }

      if(south_neighbour_sub_0 != NULL && south_neighbour_sub_0->is_tail && south_neighbour_sub_0->level == this->children[2]->level){
         south_neighbour_sub_0->roughen_side(QSIDE_NORTH);
      }
      if(south_neighbour_sub_1 != NULL && south_neighbour_sub_1->is_tail && south_neighbour_sub_1->level == this->children[3]->level){
         south_neighbour_sub_1->roughen_side(QSIDE_NORTH);
      }

      if(west_neighbour_sub_0 != NULL && west_neighbour_sub_0->is_tail && west_neighbour_sub_0->level == this->children[0]->level){
         west_neighbour_sub_0->roughen_side(QSIDE_EAST);
      }
      if(west_neighbour_sub_1 != NULL && west_neighbour_sub_1->is_tail && west_neighbour_sub_1->level == this->children[2]->level){
         west_neighbour_sub_1->roughen_side(QSIDE_EAST);
      }
#endif

      for (unsigned int i = 0; i <= 3; i++){
         this->children[i]->merge_qtree();
         this->children[i]->terrain->cleanup();
         delete this->children[i]->terrain;
         delete this->children[i];
         this->children[i] = NULL;
      }

      this->terrain = new Terrain();
      this->terrain->init(subdivisions, subdivisions, this->start_x, this->start_y, this->end_x, this->end_y, &this->noise_gen, this->shader_pid, rough_array_init);
      this->is_tail = true;

      //  1121
      //  1221
      //   |is rough

#if LOD_GESTION == 1

      Quad_tree_node *north_neighbour = this->find_neighbour_north();
      Quad_tree_node *east_neighbour = this->find_neighbour_east();
      Quad_tree_node *south_neighbour = this->find_neighbour_south();
      Quad_tree_node *west_neighbour = this->find_neighbour_west();

      if(north_neighbour != NULL && north_neighbour->is_tail && north_neighbour->level == this->level && north_neighbour->rough_edges[QSIDE_SOUTH] == true){
         north_neighbour->unroughen_side(QSIDE_SOUTH);
      }

      if(east_neighbour != NULL && east_neighbour->is_tail && east_neighbour->level == this->level && east_neighbour->rough_edges[QSIDE_WEST] == true){
         east_neighbour->unroughen_side(QSIDE_WEST);
      }

      if(south_neighbour != NULL && south_neighbour->is_tail && south_neighbour->level == this->level && south_neighbour->rough_edges[QSIDE_NORTH] == true){
         south_neighbour->unroughen_side(QSIDE_NORTH);
      }

      if(west_neighbour != NULL && west_neighbour->is_tail && west_neighbour->level == this->level && west_neighbour->rough_edges[QSIDE_EAST] == true){
         west_neighbour->unroughen_side(QSIDE_EAST);
      }
#endif
   }

   //tells which side of the subterrain should be rough, and recreate the terrain to avoid having to modify the opengl arrays
   void roughen_side(QSide side){
      assert(this->is_tail);
      this->rough_edges[side] = true;
      this->terrain->cleanup();
      delete this->terrain;
      this->terrain = new Terrain();
      this->terrain->init(subdivisions, subdivisions, this->start_x, this->start_y, this->end_x, this->end_y, &this->noise_gen, this->shader_pid, this->rough_edges);
   }

   void unroughen_side(QSide side){
      assert(this->is_tail);
      this->rough_edges[side] = false;
      this->terrain->cleanup();
      delete this->terrain;
      this->terrain = new Terrain();
      this->terrain->init(subdivisions, subdivisions, this->start_x, this->start_y, this->end_x, this->end_y, &this->noise_gen, this->shader_pid, this->rough_edges);
   }

   unsigned int subdivisions;
   unsigned int max_levels;
   unsigned int noise_generator_max_level;
   bool is_tail;
   bool is_root;
   bool rough_edges[4]; // 0=top, 1=right, 2=bottom, 3=left
   unsigned int position_id;
   float start_x;
   float start_y;
   float end_x;
   float end_y;
   unsigned int level;
   Terrain *terrain;
   Quad_tree_node *children[4];
   Quad_tree_node *parent;
   GLuint shader_pid;
   Noise_generator noise_gen;

protected:
   unsigned int log2(unsigned int val){
      unsigned int ret_val = 0;

      while (val >>=1 != 0){
         ret_val++;
      }

      return ret_val;
   }

};

#endif
