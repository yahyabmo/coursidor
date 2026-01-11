
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "helper.h"

void mark_path_objectives(struct player_info *p, vertex_t from, vertex_t to) {
    vertex_t step = from;
    while (step != to) {
        step = shortest_path(p->graph, step, to);
        if (step == (vertex_t)-1)
            break;

        for (size_t k = 0; k < p->num_objectives; ++k)
            if (p->objectives[k] == step)
                p->objectives[k] = (vertex_t)-1;
    }
}

unsigned int count_path_objectives(const struct player_info *p, vertex_t from, vertex_t to) {
    unsigned int n = 0;
    vertex_t step = from;
    while (step != to) {
        step = shortest_path(p->graph, step, to);
        if (step == (vertex_t)-1)
            break;

        for (size_t k = 0; k < p->num_objectives; ++k)
            if (p->objectives[k] == step)
                ++n;
    }
    return n;
}

unsigned int bfs_distance(const struct graph_t *g, vertex_t from, vertex_t to) {
    if (from == to)
        return 0;

    bool *vis = calloc(g->num_vertices, sizeof(bool));
    vertex_t *q = malloc(sizeof(vertex_t) * g->num_vertices);
    unsigned *d = malloc(sizeof(unsigned) * g->num_vertices);

    unsigned front = 0, back = 0;
    q[back] = from;
    d[back++] = 0;
    vis[from] = true;

    while (front < back) {
        vertex_t v = q[front];
        unsigned di = d[front++];
        struct pos_dir_t neigh[NUM_DIRS];
        unsigned n = graph_get_neighbors(g, v, neigh);

        for (unsigned i = 0; i < n; ++i) {
            vertex_t u = neigh[i].pos;
            if (neigh[i].dir == WALL_DIR || vis[u])
                continue;
            if (u == to) {
                free(vis);
                free(q);
                free(d);
                return di + 1;
            }
            vis[u] = true;
            q[back] = u;
            d[back++] = di + 1;
        }
    }
    free(vis);
    free(q);
    free(d);
    return UINT_MAX;
}

typedef struct {
    vertex_t v;
    double f, g;
} open_node_t;

static double hex_heuristic(vertex_t a, vertex_t b) { return fabs((double)a - (double)b); }

static size_t find_min_f(const open_node_t *open, size_t sz) {
    size_t best = 0;
    double best_f = open[0].f;
    for (size_t i = 1; i < sz; ++i)
        if (open[i].f < best_f) {
            best_f = open[i].f;
            best = i;
        }
    return best;
}

size_t enumerate_candidate_walls(const struct graph_t *g, const vertex_t opp_path[], size_t k,
                                 wall_pair_t out[], size_t max_out) {
    size_t out_sz = 0;
    size_t depth = 0;

    while (opp_path[depth] != (vertex_t)-1 && depth < k && out_sz < max_out) {
        vertex_t v = opp_path[depth];
        struct pos_dir_t neigh[NUM_DIRS];
        unsigned n = graph_get_neighbors(g, v, neigh);

        for (unsigned i = 0; i < n && out_sz < max_out; ++i) {
            for (unsigned j = 0; j < n && out_sz < max_out; ++j) {
                if (i == j)
                    continue;
                if (next_dir(neigh[i].dir) != neigh[j].dir)
                    continue;
                if (neigh[i].dir == WALL_DIR || neigh[j].dir == WALL_DIR)
                    continue;

                wall_pair_t w = {{{v, neigh[i].pos}, {v, neigh[j].pos}}};
                if (wall_conflicts((struct graph_t *)g, w.e))
                    continue;

                int duplicate = 0;
                for (size_t t = 0; t < out_sz; ++t)
                    if ((out[t].e[0].fr == w.e[0].fr && out[t].e[0].to == w.e[0].to) ||
                        (out[t].e[0].fr == w.e[1].to && out[t].e[0].to == w.e[1].fr))
                        duplicate = 1;
                if (!duplicate)
                    out[out_sz++] = w;
            }
        }
        ++depth;
    }
    return out_sz;
}

double astar_distance(const struct graph_t *g, vertex_t from, vertex_t to) {
    if (from == to)
        return 0.0;

    double *gcost = malloc(sizeof(double) * g->num_vertices);
    for (size_t i = 0; i < g->num_vertices; ++i)
        gcost[i] = DBL_MAX;
    gcost[from] = 0.0;

    open_node_t *open = malloc(sizeof(open_node_t) * g->num_vertices);
    size_t open_sz = 0;
    open[open_sz++] = (open_node_t){from, hex_heuristic(from, to), 0.0};

    while (open_sz) {

        size_t idx = find_min_f(open, open_sz);
        open_node_t cur = open[idx];
        open[idx] = open[--open_sz];

        if (cur.v == to) {
            double res = cur.g;
            free(gcost);
            free(open);
            return res;
        }

        struct pos_dir_t neigh[NUM_DIRS];
        unsigned n = graph_get_neighbors(g, cur.v, neigh);

        for (unsigned i = 0; i < n; ++i) {
            if (neigh[i].dir == WALL_DIR)
                continue;
            vertex_t u = neigh[i].pos;
            double ng = cur.g + 1.0;

            if (ng < gcost[u]) {
                gcost[u] = ng;
                open[open_sz++] = (open_node_t){u, ng + hex_heuristic(u, to), ng};
            }
        }
    }
    free(gcost);
    free(open);
    return DBL_MAX; /* pas de chemin          */
}

size_t astar_path(const struct graph_t *g, vertex_t from, vertex_t to, vertex_t path[],
                  size_t max_sz) {
    if (from == to) {
        if (max_sz)
            path[0] = to;
        return 1;
    }

    vertex_t *parent = malloc(sizeof(vertex_t) * g->num_vertices);
    double *gcost = malloc(sizeof(double) * g->num_vertices);
    for (size_t i = 0; i < g->num_vertices; ++i) {
        parent[i] = (vertex_t)-1;
        gcost[i] = DBL_MAX;
    }
    gcost[from] = 0.0;

    open_node_t *open = malloc(sizeof(open_node_t) * g->num_vertices);
    size_t open_sz = 0;
    open[open_sz++] = (open_node_t){from, hex_heuristic(from, to), 0.0};

    while (open_sz) {
        size_t idx = find_min_f(open, open_sz);
        open_node_t cur = open[idx];
        open[idx] = open[--open_sz];

        if (cur.v == to)
            break;

        struct pos_dir_t neigh[NUM_DIRS];
        unsigned n = graph_get_neighbors(g, cur.v, neigh);

        for (unsigned i = 0; i < n; ++i) {
            if (neigh[i].dir == WALL_DIR)
                continue;
            vertex_t u = neigh[i].pos;
            double ng = cur.g + 1.0;

            if (ng < gcost[u]) {
                gcost[u] = ng;
                parent[u] = cur.v;
                open[open_sz++] = (open_node_t){u, ng + hex_heuristic(u, to), ng};
            }
        }
    }

    size_t len = 0;
    for (vertex_t v = to; v != (vertex_t)-1 && v != from && len < max_sz; v = parent[v])
        path[len++] = v;

    for (size_t i = 0; i < len / 2; ++i) {
        vertex_t tmp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = tmp;
    }

    free(parent);
    free(gcost);
    free(open);
    return len;
}

static int player_has_way(const struct graph_t *g, const struct player_info *pl) {

    vertex_t tgt = find_closest_objective((struct graph_t *)g, pl->current, pl->objectives,
                                          pl->num_objectives);
    if (tgt == (vertex_t)-1)
        tgt = pl->start;

    if (bfs_distance(g, pl->current, tgt) == UINT_MAX)
        return 0;

    if (bfs_distance(g, tgt, pl->start) == UINT_MAX)
        return 0;

    return 1;
}

int wall_preserves_connectivity(const struct graph_t *g, const struct edge_t wall[2],
                                const struct player_info *me, const struct player_info *opp) {

    struct graph_t *tmp = graph_copy(g);
    graph_remove_edge(tmp, wall[0].fr, wall[0].to);
    graph_remove_edge(tmp, wall[1].fr, wall[1].to);

    int ok_me = player_has_way(tmp, me);
    int ok_opp = player_has_way(tmp, opp);

    graph_free(tmp);
    return ok_me && ok_opp;
}
