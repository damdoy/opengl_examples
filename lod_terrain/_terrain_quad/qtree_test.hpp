#ifndef QTREE_TEST_H
#define QTREE_TEST_H

#include "qtree.h"
#include "noise_generator.hpp"

#define QTREE_TEST_PID_DEBUG 0

//tests that the qtree is really working

void qtree_test_base(){
   Noise_generator noise_gen;
   Quad_tree_node root(NULL, 2, 5, 0);
   root.set_as_root(noise_gen, QTREE_TEST_PID_DEBUG);
   root.break_qtree();
   if(!root.is_tail && root.level == 0 && root.children[0]->is_tail && root.children[0]->level == 1){
      // printf("TEST PASSED\n");
      root.delete_root();
   }
   else{
      printf("TEST NOT PASSED");
      root.delete_root();
   }
}

void qtree_test_neighbour_simple(){
   Noise_generator noise_gen;
   Quad_tree_node root(NULL, 2, 5, 0);
   root.set_as_root(noise_gen, QTREE_TEST_PID_DEBUG);
   root.break_qtree();

   Quad_tree_node *north_neighbour = root.children[2]->find_neighbour_north();
   Quad_tree_node *south_neighbour = root.children[1]->find_neighbour_south();

   if(north_neighbour == root.children[0] && south_neighbour == root.children[3]){
      // printf("TEST PASSED\n");
      root.delete_root();
   }
   else{
      printf("TEST NOT PASSED\n");
      root.delete_root();
   }
}


//12
//21
//north neighbour of 2 should be the 1 above
void qtree_test_neighbour_of_child(){
   Noise_generator noise_gen;
   Quad_tree_node root(NULL, 2, 5, 0);
   root.set_as_root(noise_gen, QTREE_TEST_PID_DEBUG);
   root.break_qtree();
   root.children[1]->break_qtree();
   root.children[2]->break_qtree();

   Quad_tree_node *north_neighbour_0 = root.children[2]->children[0]->find_neighbour_north();
   Quad_tree_node *north_neighbour_1 = root.children[2]->children[1]->find_neighbour_north();

   Quad_tree_node *south_neighbour_0 = root.children[1]->children[2]->find_neighbour_south();
   Quad_tree_node *south_neighbour_1 = root.children[1]->children[3]->find_neighbour_south();

   Quad_tree_node *south_neighbour_null = root.children[2]->children[2]->find_neighbour_south();

   Quad_tree_node *south_neighbour_sub = root.children[1]->children[1]->find_neighbour_south();

   if(north_neighbour_0 == root.children[0] && north_neighbour_1 == root.children[0]
      && south_neighbour_0 == root.children[3] && south_neighbour_1 == root.children[3]
      && south_neighbour_null == NULL
      && south_neighbour_sub == root.children[1]->children[3]){
      // printf("TEST_PASSED\n");
      root.delete_root();
   }
   else{
      printf("TEST NOT PASSED\n");
      root.delete_root();
   }
}

void qtree_test_neighbour_different_parents(){
   Noise_generator noise_gen;
   Quad_tree_node root(NULL, 2, 5, 0);
   root.set_as_root(noise_gen, QTREE_TEST_PID_DEBUG);
   root.break_qtree();
   root.children[0]->break_qtree();
   root.children[1]->break_qtree();
   root.children[2]->break_qtree();
   root.children[3]->break_qtree();

   Quad_tree_node *north_neighbour = root.children[2]->children[0]->find_neighbour_north();
   Quad_tree_node *south_neighbour = root.children[1]->children[3]->find_neighbour_south();

   if(north_neighbour == root.children[0]->children[2]
   && south_neighbour == root.children[3]->children[1]){

      // printf("TEST PASSED\n");
      root.delete_root();
   }
   else{
      printf("TEST NOT PASSED\n");
      root.delete_root();
   }

}


#endif
