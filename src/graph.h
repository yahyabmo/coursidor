#ifndef _CORS_GRAPH_H_
#define _CORS_GRAPH_H_

#include <gsl/gsl_spblas.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <stddef.h> // Necessary for gsl
#include <time.h>

#include "move.h"

/* Possible directions of the graph */
enum dir_t {
    NO_EDGE = 0,
    NW = 1,
    NE = 2,
    E = 3,
    SE = 4,
    SW = 5,
    W = 6,
    FIRST_DIR = NW,
    LAST_DIR = W,
    NUM_DIRS = 6,
    WALL_DIR = 7,
};

/* A function to determine the opposite direction of a direction `d` */
static inline enum dir_t opposite_dir(enum dir_t d) {
    return (d == 0) ? 0 : (d >= 4) ? (d - 3) : (d + 3);
}

/* A function to determine the next dir in the counterclockwise order */
static inline enum dir_t next_dir(enum dir_t d) {
    return (d == 0) ? 0 : (d == 1) ? LAST_DIR : (d - 1);
}

/* The different types of graphs on which the game is played */
enum graph_type_t { TRIANGULAR = 0, CYCLIC = 1, HOLEY = 2 };

/* The representation of a graph */
struct graph_t {
    enum graph_type_t type;      // The type of the graph
    unsigned int num_vertices;   // Number of vertices in the graph
    unsigned int num_edges;      // Number of edges in the graph
    gsl_spmatrix_uint *t;        // Sparse matrix of size num_vertices*num_vertices,
                                 // t[i][j] > 0 means there is an edge from i to j
                                 // t[i][j] == E means that j is EAST of i
                                 // t[i][j] == W means that j is WEST of i
                                 // and so on
    vertex_t start[NUM_PLAYERS]; // Starting vertices of both players
    unsigned int num_objectives; // Number of objectives in the graph
    vertex_t *objectives;        // Objectives of the graph
};

struct pos_dir_t {
    vertex_t pos;
    enum dir_t dir;
};

struct graph_t *graph_create(enum graph_type_t type, unsigned int m);
void graph_initialize_triangular(struct graph_t *g, unsigned int m);
void graph_initialize_cyclic(struct graph_t *g, unsigned int m);
void graph_initialize_holed(struct graph_t *g, unsigned int m);
void graph_add_edge(struct graph_t *g, vertex_t i, vertex_t j, enum dir_t dir);
void graph_remove_edge(struct graph_t *g, vertex_t i, vertex_t j);
void graph_free(struct graph_t *g);
unsigned int graph_num_vertices(const struct graph_t *g);

unsigned int graph_num_objectives(const struct graph_t *g);

vertex_t graph_get_objective(const struct graph_t *g, unsigned int i);

vertex_t graph_get_start(const struct graph_t *g, unsigned int player_id);

unsigned int graph_get_neighbors(const struct graph_t *g, vertex_t vertex,
                                 struct pos_dir_t neighbors[]);

int graph_has_edge(const struct graph_t *g, vertex_t from, vertex_t to);

enum dir_t graph_get_direction(const struct graph_t *g, vertex_t from, vertex_t to);

struct graph_t *graph_copy(const struct graph_t *src);

void graph_initialize_objectives(const struct graph_t *src);

void graph_remove_edge(struct graph_t *g, vertex_t i, vertex_t j);

#endif // _CORS_GRAPH_H_
