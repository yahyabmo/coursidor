

#ifndef _HELPER_H_
#define _HELPER_H_

#include "commun.h"
#include "move.h"
#include <stddef.h>
void mark_path_objectives(struct player_info *p, vertex_t from, vertex_t to);

unsigned int count_path_objectives(const struct player_info *p, vertex_t from, vertex_t to);

unsigned int bfs_distance(const struct graph_t *g, vertex_t from, vertex_t to);

double astar_distance(const struct graph_t *g, vertex_t from, vertex_t to);

size_t astar_path(const struct graph_t *g, vertex_t from, vertex_t to, vertex_t path[],
                  size_t path_size);

typedef struct {
    struct edge_t e[2];
} wall_pair_t;

size_t enumerate_candidate_walls(const struct graph_t *g, const vertex_t opp_path[], size_t k,
                                 wall_pair_t out[], size_t max_out);

int wall_preserves_connectivity(const struct graph_t *g, const struct edge_t wall[2],
                                const struct player_info *me, const struct player_info *opp);

#endif
