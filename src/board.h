#ifndef _CORS_BOARD_H_
#define _CORS_BOARD_H_

#include "graph.h"

struct board_t;

// Crée une structure board contenant deux copies du graphe, les positions, les
// objectifs, etc.
struct board_t *board_create(enum graph_type_t type, unsigned int m, int first_player);

// Donne le graphe d’un joueur (0 ou 1)
struct graph_t *board_get_graph(const struct board_t *board, int player_id);

// Renvoie la position actuelle d’un joueur
vertex_t board_get_position(const struct board_t *board, int player_id);

// Renvoie le sommet de départ d’un joueur
vertex_t board_get_start(const struct board_t *board, int player_id);

// Met à jour la position d’un joueur
void board_set_position(struct board_t *board, int player_id, vertex_t new_pos);

// Nombre d’objectifs
unsigned int board_get_objective_count(const struct board_t *board);

// Récupère un objectif
vertex_t board_get_objective(const struct board_t *board, int player_id, unsigned int i);

// Libère la mémoire du board
void board_destroy(struct board_t *board);

void board_mark_objective_reached(struct board_t *board, int player_id, vertex_t v);

vertex_t board_get_display_objective(const struct board_t *board, unsigned int i);

void board_add_walls(struct board_t *board, int player_id, struct move_t move);

struct graph_t *board_get_server_graph(const struct board_t *board);
void board_mark_path_objectives(struct board_t *board, int player_id, vertex_t from, vertex_t to);

#endif
