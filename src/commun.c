#include "commun.h"
#include "graph.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
struct player_info player_infos[2];

struct player_info me;
int is_almost_done(struct player_info *p) {
    int remaining = 0;
    for (size_t i = 0; i < p->num_objectives; ++i)
        if (p->objectives[i] != (vertex_t)-1)
            remaining++;
    return remaining <= 3;
}
int has_path_to_objectives(struct graph_t *g, vertex_t start, vertex_t *objectives,
                           size_t num_obj) {
    printf("[DEBUG] Vérification de chemin depuis %u vers objectifs...\n", start);

    bool visited[g->num_vertices];
    memset(visited, 0, sizeof(visited));
    vertex_t queue[g->num_vertices];
    int front = 0, rear = 0;

    queue[rear++] = start;
    visited[start] = true;

    while (front < rear) {
        vertex_t current = queue[front++];

        for (size_t i = 0; i < num_obj; ++i) {
            if (objectives[i] == current)
                return 1;
        }

        struct pos_dir_t neighbors[6];
        unsigned int count = graph_get_neighbors(g, current, neighbors);
        for (unsigned int i = 0; i < count; ++i) {
            vertex_t neighbor = neighbors[i].pos;
            if (!visited[neighbor] && neighbors[i].dir != WALL_DIR) {
                visited[neighbor] = true;
                queue[rear++] = neighbor;
            }
        }
    }
    return 0;
}
int next_objective(struct player_info *p) {
    for (size_t i = 0; i < p->num_objectives; ++i) {
        if (p->objectives[i] != (vertex_t)-1) {
            return i;
        }
    }
    return -1;
}

/* size_t compute_valid_moves(struct graph_t *g, vertex_t pos, enum dir_t last_dir, vertex_t
 * opponent, */
/*                            struct pos_dir_t out_neighbors[]) { */
/*     size_t move_count = 0; */
/*     struct pos_dir_t neighbors[NUM_DIRS]; */
/*     unsigned int n = graph_get_neighbors(g, pos, neighbors); */

/*     for (unsigned int i = 0; i < n; ++i) { */
/*         vertex_t next = neighbors[i].pos; */
/*         enum dir_t d = neighbors[i].dir; */

/*         if (next == opponent) { */
/*             vertex_t jump = jump_over(g, pos, opponent); */
/*             if (jump != (vertex_t)-1) { */
/*                 out_neighbors[move_count++] = (struct pos_dir_t){jump, me.last_dir}; */
/*             } */
/*             continue; */
/*         } */

/*         if (!graph_has_edge(g, pos, next) || next == opponent) */
/*             continue; */

/*         out_neighbors[move_count++] = (struct pos_dir_t){next, d}; */

/* 	int max_steps = 1; */
/* if (last_dir != NO_EDGE && last_dir != WALL_DIR) { */
/*     if (d == last_dir) */
/*         max_steps = 3; */
/*     else if ((d == next_dir(last_dir)) || (last_dir == next_dir(d))) */
/*         max_steps = 2; */
/* } */

/*         vertex_t current = next; */
/*         for (int step = 1; step < max_steps; ++step) { */
/*             struct pos_dir_t nexts[NUM_DIRS]; */
/*             unsigned int next_n = graph_get_neighbors(g, current, nexts); */
/*             int found = 0; */

/*             for (unsigned int j = 0; j < next_n; ++j) { */
/*                 if (nexts[j].dir == d && graph_has_edge(g, current, nexts[j].pos) */
/*                     && nexts[j].pos != opponent) { */
/*                     current = nexts[j].pos; */
/*                     out_neighbors[move_count++] = (struct pos_dir_t){current, d}; */
/*                     found = 1; */
/*                     break; */
/*                 } */
/*             } */

/*             if (!found) */
/*                 break; */
/*         } */
/*     } */

/*     return move_count; */
/* } */

/* size_t compute_valid_moves(struct graph_t *g, */
/*                            vertex_t pos, */
/*                            enum dir_t last_dir, */
/*                            vertex_t opponent, */
/*                            struct pos_dir_t out_neighbors[]) */
/* { */
/*     size_t move_count = 0; */

/*     // 1) Récupère d’abord tous les voisins directs (sans mur) de pos */
/*     struct pos_dir_t neighbors[NUM_DIRS]; */
/*     unsigned int n = graph_get_neighbors(g, pos, neighbors); */

/*     // 2) Vérifie si l’adversaire est adjacent, une seule fois */
/*     bool opponent_adjacent = false; */
/*     for (unsigned int i = 0; i < n; ++i) { */
/*         if (neighbors[i].pos == opponent) { */
/*             opponent_adjacent = true; */
/*             break; */
/*         } */
/*     } */

/*     for (unsigned int i = 0; i < n; ++i) { */
/*         vertex_t next = neighbors[i].pos; */
/*         enum dir_t d  = neighbors[i].dir; */

/*         // --- saut par-dessus l’adversaire --- */
/*         if (next == opponent) { */
/*             if (!opponent_adjacent) { */
/*                 // on ne devrait jamais arriver ici puisque next vient de neighbors[], */
/*                 // mais on double‐vérifie pour la sécurité */
/*                 continue; */
/*             } */
/*             vertex_t jump = jump_over(g, pos, opponent); */
/*             // n’accepte que si jump est valide et n’atterrit ni sur pos ni sur opponent */
/*             if (jump != (vertex_t)-1 */
/*                 && jump != pos */
/*                 && jump != opponent) */
/*             { */
/*                 out_neighbors[move_count++] = (struct pos_dir_t){ jump, d }; */
/*             } */
/*             // après un jump, on ne tente pas de pas normaux depuis cette direction */
/*             continue; */
/*         } */

/*         // --- déplacement “classique” vers next --- */
/*         // (i) on ne peut pas “tomber” sur l’adversaire */
/*         // (ii) on s’assure que l’arête existe (non‐murée) */
/*         if (next == opponent || !graph_has_edge(g, pos, next)) */
/*             continue; */

/*         // 1er pas */
/*         out_neighbors[move_count++] = (struct pos_dir_t){ next, d }; */

/*         // puis éventuellement 2e et 3e pas si on reste dans la même direction */
/*         int max_steps = 1; */
/*         if (last_dir != NO_EDGE && last_dir != WALL_DIR) { */
/*             if (d == last_dir) { */
/*                 max_steps = 3; */
/*             } else if (d == next_dir(last_dir) */
/*                        || last_dir == next_dir(d)) { */
/*                 max_steps = 2; */
/*             } */
/*         } */

/*         vertex_t current = next; */
/*         for (int step = 1; step < max_steps; ++step) { */
/*             struct pos_dir_t nexts[NUM_DIRS]; */
/*             unsigned int next_n = graph_get_neighbors(g, current, nexts); */
/*             bool found = false; */
/*             for (unsigned int j = 0; j < next_n; ++j) { */
/*                 if (nexts[j].dir == d */
/*                     && graph_has_edge(g, current, nexts[j].pos) */
/*                     && nexts[j].pos != opponent) */
/*                 { */
/*                     current = nexts[j].pos; */
/*                     out_neighbors[move_count++] = */
/*                         (struct pos_dir_t){ current, d }; */
/*                     found = true; */
/*                     break; */
/*                 } */
/*             } */
/*             if (!found) break; */
/*         } */
/*     } */

/*     return move_count; */
/* } */

// Renvoie toutes les positions accessibles (mouvements + sauts) depuis `pos`.
// out_neighbors doit avoir une capacité >= NUM_DIRS * 3 pour être sûr de ne pas déborder.
size_t compute_valid_moves(struct graph_t *g, vertex_t pos, enum dir_t last_dir, vertex_t opponent,
                           struct pos_dir_t out_neighbors[]) {
    size_t move_count = 0;

    // 1) récupère les voisins directs
    struct pos_dir_t neighbors[NUM_DIRS];
    unsigned int n = graph_get_neighbors(g, pos, neighbors);

    // 2) vérifie si l'adversaire est adjacent
    bool opp_adj = false;
    for (unsigned int i = 0; i < n; ++i) {
        if (neighbors[i].pos == opponent) {
            opp_adj = true;
            break;
        }
    }

    for (unsigned int i = 0; i < n; ++i) {
        vertex_t next = neighbors[i].pos;
        enum dir_t d = neighbors[i].dir;

        // Saut par-dessus si l'adversaire est adjacent
        if (next == opponent) {
            if (!opp_adj)
                continue;
            vertex_t jump = jump_over(g, pos, opponent);
            if (jump != (vertex_t)-1 && jump != pos && jump != opponent) {
                out_neighbors[move_count++] = (struct pos_dir_t){jump, d};
            }
            continue;
        }

        // Mouvement classique
        if (!graph_has_edge(g, pos, next) || next == opponent)
            continue;
        out_neighbors[move_count++] = (struct pos_dir_t){next, d};

        // mouvements multiples dans la même direction
        int max_steps = 1;
        if (last_dir != NO_EDGE && last_dir != WALL_DIR) {
            if (d == last_dir)
                max_steps = 3;
            else if (d == next_dir(last_dir) || last_dir == next_dir(d))
                max_steps = 2;
        }
        vertex_t cur = next;
        for (int step = 1; step < max_steps; ++step) {
            struct pos_dir_t nbrs[NUM_DIRS];
            unsigned int nn = graph_get_neighbors(g, cur, nbrs);
            bool found = false;
            for (unsigned int j = 0; j < nn; ++j) {
                if (nbrs[j].dir == d && graph_has_edge(g, cur, nbrs[j].pos) &&
                    nbrs[j].pos != opponent) {
                    cur = nbrs[j].pos;
                    out_neighbors[move_count++] = (struct pos_dir_t){cur, d};
                    found = true;
                    break;
                }
            }
            if (!found)
                break;
        }
    }
    return move_count;
}

// Renvoie -1 si pas de saut possible, sinon sommet d'arrivée.
vertex_t jump_over(struct graph_t *g, vertex_t me_pos, vertex_t opponent) {
    // récupère les voisins directs de me_pos
    struct pos_dir_t neigh[NUM_DIRS];
    unsigned int cnt = graph_get_neighbors(g, me_pos, neigh);
    // s'assure que l'adversaire est adjacent
    bool adj = false;
    enum dir_t dir_to_opp = NO_EDGE;
    for (unsigned int i = 0; i < cnt; ++i) {
        if (neigh[i].pos == opponent) {
            adj = true;
            dir_to_opp = neigh[i].dir;
            break;
        }
    }
    if (!adj || !graph_has_edge(g, me_pos, opponent))
        return (vertex_t)-1;

    // récupère voisins de l'adversaire dans la même direction
    struct pos_dir_t opp_nb[NUM_DIRS];
    unsigned int opp_n = graph_get_neighbors(g, opponent, opp_nb);
    for (unsigned int j = 0; j < opp_n; ++j) {
        if (opp_nb[j].dir == dir_to_opp && graph_has_edge(g, opponent, opp_nb[j].pos)) {
            vertex_t jump = opp_nb[j].pos;
            if (jump != me_pos && jump != opponent)
                return jump;
        }
    }
    return (vertex_t)-1;
}

int direction_between(struct graph_t *g, vertex_t from, vertex_t to) {
    enum dir_t d = graph_get_direction(g, from, to);
    return (d != NO_EDGE) ? (int)d : -1;
}

/* vertex_t jump_over(struct graph_t *g, vertex_t me_pos, vertex_t opponent) { */
/*     struct pos_dir_t neighbors[NUM_DIRS]; */
/*     graph_get_neighbors(g, me_pos, neighbors); */

/*     for (unsigned int i = 0; i < NUM_DIRS; ++i) { */
/*         if (neighbors[i].pos == opponent && graph_has_edge(g, me_pos, opponent)) { */
/*             // Cherche les voisins de l’adversaire */
/*             struct pos_dir_t opp_neighbors[NUM_DIRS]; */
/*             graph_get_neighbors(g, opponent, opp_neighbors); */

/*             vertex_t best_jump = (vertex_t)-1; */
/*             unsigned int best_dist = (unsigned int)-1; */

/*             vertex_t target = find_closest_objective(g, opponent, me.objectives,
 * me.num_objectives); */

/*             for (unsigned int j = 0; j < NUM_DIRS; ++j) { */
/*                 vertex_t jump = opp_neighbors[j].pos; */

/*                 // Ignore si on saute sur soi-même ou l’adversaire */
/*                 if (jump == me_pos || jump == opponent) */
/*                     continue; */

/*                 if (graph_has_edge(g, opponent, jump)) { */
/*                     // Estime la distance depuis cette case vers l’objectif */
/*                     vertex_t tmp = jump; */
/*                     unsigned int d = 0; */
/*                     while (tmp != target && d < g->num_vertices) { */
/*                         tmp = shortest_path(g, tmp, target); */
/*                         if (tmp == (vertex_t)-1) break; */
/*                         d++; */
/*                     } */

/*                     if (d < best_dist) { */
/*                         best_dist = d; */
/*                         best_jump = jump; */
/*                         me.last_dir = opp_neighbors[j].dir;   */
/*                     } */
/*                 } */
/*             } */

/*             return best_jump; */
/*         } */
/*     } */

/*     return (vertex_t)-1; */
/* } */

int has_won_info(struct player_info *p) {
    if (p->current != p->start)
        return 0;
    for (size_t i = 0; i < p->num_objectives; ++i) {
        if (p->objectives[i] != (vertex_t)-1)
            return 0;
    }
    return 1;
}

int opponent_almost_done(struct player_info *opponent) {
    int remaining = 0;
    for (size_t i = 0; i < opponent->num_objectives; ++i) {
        if (opponent->objectives[i] != (vertex_t)-1)
            remaining++;
    }
    return remaining <= 1;
}

int wall_conflicts(struct graph_t *g, struct edge_t wall[2]) {
    for (int i = 0; i < 2; ++i) {
        vertex_t from = wall[i].fr;
        vertex_t to = wall[i].to;

        // Vérifie si cette arête est déjà bloquée
        if (!graph_has_edge(g, from, to)) {
            return 1;
        }

        struct pos_dir_t neighbors[NUM_DIRS];
        unsigned int n = graph_get_neighbors(g, from, neighbors);
        for (unsigned int j = 0; j < n; ++j) {
            vertex_t neighbor = neighbors[j].pos;

            if (neighbor == to)
                continue;

            if (!graph_has_edge(g, from, neighbor)) {
                return 1;
            }
        }
    }

    return 0;
}

int wall_is_legal(struct graph_t *graph, struct edge_t wall[2], vertex_t pos_self, vertex_t pos_opp,
                  vertex_t *self_obj, size_t self_n, vertex_t *opp_obj, size_t opp_n) {
    struct graph_t *copy = graph_copy(graph);

    for (int i = 0; i < 2; ++i)
        graph_remove_edge(copy, wall[i].fr, wall[i].to);

    // On vérifie que les deux joueurs ont toujours un chemin
    int ok_self = has_path_to_objectives(copy, pos_self, self_obj, self_n);
    int ok_opp = has_path_to_objectives(copy, pos_opp, opp_obj, opp_n);

    graph_free(copy);
    return ok_self && ok_opp;
}

vertex_t shortest_path(struct graph_t *g, vertex_t from, vertex_t to) {
    if (from == to)
        return from;

    bool *visited = calloc(g->num_vertices, sizeof(bool));
    vertex_t *parent = malloc(g->num_vertices * sizeof(vertex_t));
    vertex_t *queue = malloc(g->num_vertices * sizeof(vertex_t));

    int front = 0, rear = 0;
    queue[rear++] = from;
    visited[from] = true;
    parent[from] = (vertex_t)-1;

    while (front < rear) {
        vertex_t current = queue[front++];

        struct pos_dir_t neighbors[NUM_DIRS];
        unsigned int count = graph_get_neighbors(g, current, neighbors);

        for (unsigned i = 0; i < count; ++i) {
            vertex_t neighbor = neighbors[i].pos;
            if (!visited[neighbor] && neighbors[i].dir != WALL_DIR) {
                parent[neighbor] = current;
                visited[neighbor] = true;

                if (neighbor == to) {
                    // Reconstruire le premier pas du chemin
                    vertex_t node = to;
                    while (parent[node] != from && parent[node] != (vertex_t)-1) {
                        node = parent[node];
                    }
                    free(visited);
                    free(parent);
                    free(queue);
                    return node;
                }

                queue[rear++] = neighbor;
            }
        }
    }

    free(visited);
    free(parent);
    free(queue);
    return (vertex_t)-1; // Pas de chemin trouvé
}

// Trouve l'objectif le plus proche avec BFS
vertex_t find_closest_objective(struct graph_t *g, vertex_t start, vertex_t *objectives,
                                size_t num_obj) {
    bool visited[g->num_vertices];
    memset(visited, 0, sizeof(visited));
    vertex_t queue[g->num_vertices];
    int front = 0, rear = 0;

    queue[rear++] = start;
    visited[start] = true;

    while (front < rear) {
        vertex_t current = queue[front++];

        // Vérifier si current est un objectif non atteint
        for (size_t i = 0; i < num_obj; ++i) {
            if (objectives[i] == current)
                return current;
        }

        // Explorer les voisins
        struct pos_dir_t neighbors[NUM_DIRS];
        unsigned int count = graph_get_neighbors(g, current, neighbors);
        for (unsigned int j = 0; j < count; ++j) {
            vertex_t neighbor = neighbors[j].pos;
            if (!visited[neighbor] && neighbors[j].dir != WALL_DIR) {
                visited[neighbor] = true;
                queue[rear++] = neighbor;
            }
        }
    }
    return (vertex_t)-1;
}

int is_opponent_near_last_objective(struct player_info *opponent) {
    for (size_t i = 0; i < opponent->num_objectives; ++i) {
        vertex_t obj = opponent->objectives[i];
        if (obj != (vertex_t)-1) {
            vertex_t tmp = opponent->current;
            int dist = 0;
            while (tmp != obj && dist <= 3) {
                tmp = shortest_path(opponent->graph, tmp, obj);
                if (tmp == (vertex_t)-1)
                    break;
                dist++;
            }
            if (dist <= 3)
                return 1;
        }
    }
    return 0;
}

void set_player_id_from_server(int id) {
    me.id = id;
    me.color = (id == 0) ? BLACK : WHITE;
}
