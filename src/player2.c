
#include "commun.h"
#include "player.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char const *get_player_name() { return "yahya"; }

void initialize(unsigned int player_id, struct graph_t *graph) {
    me.id = player_id;
    me.color = (player_id == 0) ? BLACK : WHITE;
    me.graph = graph;
    me.start = graph_get_start(graph, player_id);
    me.current = me.start;
    me.last_opponent = (vertex_t)-1;
    me.last_dir = NO_EDGE;
    me.num_objectives = graph_num_objectives(graph);
    me.objectives = malloc(me.num_objectives * sizeof(vertex_t));
    for (unsigned int i = 0; i < me.num_objectives; ++i)
        me.objectives[i] = graph_get_objective(graph, i);
    printf("Objectifs initiaux :\n");
    for (unsigned int i = 0; i < graph->num_objectives; ++i)
        printf("- objective[%u] = %u\n", i, graph->objectives[i]);
    me.num_walls = (me.graph->num_edges + 15) / 16;
}

struct move_t play(const struct move_t previous_move) {
    // Met à jour le graphe local si l'adversaire a posé un mur
    if (previous_move.t != NO_TYPE && previous_move.c != me.color) {
        me.last_opponent = previous_move.m;

        if (previous_move.t == WALL) {
            for (int i = 0; i < 2; ++i) {
                struct edge_t e = previous_move.e[i];
                graph_remove_edge(me.graph, e.fr, e.to); // supprime dans les 2 sens
            }
        }
    }

    struct move_t m = {.c = me.color};

    /* // === 1. Tentative de poser un mur stratégique === */
    /* if (me.num_walls > 0 && me.last_opponent != (vertex_t)-1) { */
    /*     int next_obj_idx = next_objective(&me); */
    /*     if (next_obj_idx != -1) { */
    /*         vertex_t my_obj = me.objectives[next_obj_idx]; */
    /*         vertex_t opponent_obj = me.graph->num_vertices - my_obj - 1; */

    /*         int opponent_dist = 0; */
    /*         vertex_t tmp = me.last_opponent; */
    /*         while (tmp != opponent_obj && opponent_dist < 4) { */
    /*             tmp = shortest_path(me.graph, tmp, opponent_obj); */
    /*             if (tmp == (vertex_t)-1) break; */
    /*             opponent_dist++; */
    /*         } */

    /*         if (opponent_dist <= 2) { */
    /*             int dir = direction_between(me.graph, me.last_opponent, opponent_obj); */
    /*             if (dir != -1) { */
    /*                 enum dir_t target_dir = dir; */
    /*                 enum dir_t next_target_dir = next_dir(target_dir); */

    /*                 struct pos_dir_t neighbors[NUM_DIRS]; */
    /*                 unsigned int n = graph_get_neighbors(me.graph, me.last_opponent, neighbors);
     */

    /*                 struct edge_t walls[2] = {{-1, -1}, {-1, -1}}; */
    /*                 for (unsigned int i = 0; i < n; ++i) { */
    /*                     if (neighbors[i].dir == target_dir || neighbors[i].dir ==
     * next_target_dir) { */
    /*                         if ((int)walls[0].fr == -1) */
    /*                             walls[0] = (struct edge_t){me.last_opponent, neighbors[i].pos};
     */
    /*                         else { */
    /*                             walls[1] = (struct edge_t){me.last_opponent, neighbors[i].pos};
     */
    /*                             break; */
    /*                         } */
    /*                     } */
    /*                 } */

    /*                 if ((int)walls[0].fr != -1 && (int)walls[1].fr != -1 && */
    /*                     !wall_conflicts(me.graph, walls)) { */
    /*                     struct graph_t *test_graph = graph_copy(me.graph); */
    /*                     for (int i = 0; i < 2; ++i) */
    /*                         graph_remove_edge(test_graph, walls[i].fr, walls[i].to); */

    /*                     vertex_t *opponent_obj_ptr = &opponent_obj; */
    /*                     if (has_path_to_objectives(test_graph, me.last_opponent,
     * opponent_obj_ptr, 1) && */
    /*                         wall_is_legal(me.graph, walls, me.current, me.last_opponent, */
    /*                                       me.objectives, me.num_objectives, opponent_obj_ptr, 1))
     * { */

    /*                         // Mise à jour du graphe local avec le mur qu'on pose soi-même */
    /*                         for (int i = 0; i < 2; ++i) */
    /*                             graph_remove_edge(me.graph, walls[i].fr, walls[i].to); */

    /*                         m.t = WALL; */
    /*                         m.e[0] = walls[0]; */
    /*                         m.e[1] = walls[1]; */
    /*                         me.num_walls--; */
    /*                         graph_free(test_graph); */
    /*                         return m; */
    /*                     } */
    /*                     graph_free(test_graph); */
    /*                 } */
    /*             } */
    /*         } */
    /*     } */
    /* } */

    // === 2. Déplacement vers l’objectif ou retour à la base ===
    m.t = MOVE;
    vertex_t target =
        find_closest_objective(me.graph, me.current, me.objectives, me.num_objectives);
    if (target == (vertex_t)-1)
        target = me.start;

    struct pos_dir_t valid_moves[16];
    size_t num_moves =
        compute_valid_moves(me.graph, me.current, me.last_dir, me.last_opponent, valid_moves);

    vertex_t best = me.current;
    unsigned int min_dist = (unsigned int)-1;

    for (size_t i = 0; i < num_moves; ++i) {
        vertex_t candidate = valid_moves[i].pos;

        if (candidate == me.current || candidate == me.last_opponent)
            continue;

        // Estimer la distance à l’objectif depuis cette case
        vertex_t tmp = candidate;
        unsigned int d = 0;
        while (tmp != target) {
            tmp = shortest_path(me.graph, tmp, target);
            if (tmp == (vertex_t)-1)
                break;
            d++;
        }

        if (d < min_dist) {
            min_dist = d;
            best = candidate;
            me.last_dir = valid_moves[i].dir;
        }
    }

    m.m = best;
    me.current = best;

    for (unsigned int j = 0; j < me.num_objectives; ++j) {
        if (me.objectives[j] == m.m)
            me.objectives[j] = (vertex_t)-1;
    }

    return m;
}

void finalize() {
    free(me.objectives);
    graph_free(me.graph);
}
