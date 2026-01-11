#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "graph.h"
#include "move.h"

// Affiche l'état du jeu :
// - p0 : position du joueur BLACK
// - p1 : position du joueur WHITE
// - objectives : liste des sommets objectifs encore actifs
void print_game_triangular(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1,
                           vertex_t *objectives, size_t num_obj);
void print_game_cyclic(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1,
                       vertex_t *objectives, size_t num_obj);

void print_game_holed(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1,
                      vertex_t *objectives, size_t num_obj);
#endif // _DISPLAY_H_
