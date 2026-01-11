#ifndef _COMMUN_H_
#define _COMMUN_H_

#include "graph.h"
#include "move.h"

struct player_info {
    vertex_t start;
    vertex_t current;
    vertex_t last_opponent;
    enum player_color_t color;
    struct graph_t *graph;
    vertex_t *objectives;
    size_t num_objectives;
    enum dir_t last_dir;
    unsigned int id;
    unsigned int num_walls;
};

// retourne l'indice de l'objectif suivant -1 si tous atteint
// tableau objectives est alloué dans initialize (player[1-2].c)
int next_objective(struct player_info *p);

// vertex_t shortest_path(struct graph_t* g, vertex_t from, vertex_t to);
size_t compute_valid_moves(struct graph_t *g, vertex_t pos, enum dir_t last_dir, vertex_t opponent,
                           struct pos_dir_t out_neighbors[]);
size_t compute_valid_moves2(struct graph_t *g, vertex_t pos, enum dir_t last_dir, vertex_t opponent,
                            struct pos_dir_t out_neighbors[]);

int direction_between(struct graph_t *g, vertex_t from, vertex_t to);

vertex_t jump_over(struct graph_t *g, vertex_t me, vertex_t opponent);
vertex_t jump_over2(struct graph_t *g, vertex_t me, vertex_t opponent);

// Retourne 1 si le joueur a gagné (tous les objectifs visités + retour au
// départ)
int has_won_info(struct player_info *p);

// coord_t *get_vertex_coords(const struct graph_t *g);
// int hex_distance(coord_t a, coord_t b);

// vertex_t a_star_pathfinding(struct graph_t *graph, vertex_t start, vertex_t *goals,
//                             size_t num_goals);

vertex_t find_closest_objective(struct graph_t *g, vertex_t start, vertex_t *objectives,
                                size_t num_obj);
int is_opponent_near_last_objective(struct player_info *opponent);
// Permet à server.c de dire à player.c qui il est (BLACK = 0 ou WHITE = 1)
void set_player_id_from_server(int id);
extern struct player_info me;

// Vérifie si un chemin existe entre 'start' et les objectifs
int has_path_to_objectives(struct graph_t *g, vertex_t start, vertex_t *objectives, size_t num_obj);
int opponent_almost_done(struct player_info *opponent);
extern struct player_info player_infos[2]; // permet à player1.c d'y accéder
int is_almost_done(struct player_info *p); // signature de la fonction
int wall_conflicts(struct graph_t *g, struct edge_t wall[2]);

int wall_is_legal(struct graph_t *graph, struct edge_t wall[2], vertex_t pos_self, vertex_t pos_opp,
                  vertex_t *self_obj, size_t self_n, vertex_t *opp_obj, size_t opp_n);

// Ajouter cette déclaration
vertex_t shortest_path(struct graph_t *g, vertex_t from, vertex_t to);

#endif // _COMMUN_H_
