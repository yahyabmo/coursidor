
#include "commun.h"
#include "helper.h"
#include "player.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char const *get_player_name() { return "sohayb ajaji"; }

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
        printf("  - objective[%u] = %u\n", i, graph->objectives[i]);

    me.num_walls = (me.graph->num_edges + 15) / 16;
}

#include <float.h>

struct move_t play(const struct move_t prev) {

    if (prev.t != NO_TYPE && prev.c != me.color) {
        me.last_opponent = prev.m;
        if (prev.t == MOVE)
            me.last_opponent = prev.m;
        if (prev.t == WALL) {
            for (int i = 0; i < 2; ++i)
                graph_remove_edge(me.graph, prev.e[i].fr, prev.e[i].to);
        }
    }

    struct move_t m = {.c = me.color, .t = MOVE};

    /* if (me.num_walls > 0 && me.last_opponent != (vertex_t)-1) { */

    /*     vertex_t opp_target; */
    /*     int next_my = next_objective(&me); */
    /*     if (next_my != -1) */
    /*         opp_target = me.graph->num_vertices - me.objectives[next_my] - 1; */
    /*     else */
    /*         opp_target = graph_get_start(me.graph, (me.color == BLACK) ? WHITE : BLACK); */

    /*     unsigned old_dist = bfs_distance(me.graph, me.last_opponent, opp_target); */

    /*     if (old_dist != UINT_MAX && old_dist <= 6) { */

    /*         struct pos_dir_t neigh[NUM_DIRS]; */
    /*         unsigned n = graph_get_neighbors(me.graph, me.last_opponent, neigh); */

    /*         struct edge_t best_wall[2] = {{0, 0}, {0, 0}}; */
    /*         int best_delta = 0; */

    /*         for (unsigned i = 0; i < n; ++i) */
    /*             for (unsigned j = 0; j < n; ++j) { */
    /*                 if (i == j) */
    /*                     continue; */
    /*                 if (next_dir(neigh[i].dir) != neigh[j].dir) */
    /*                     continue; */

    /*                 struct edge_t wall[2] = {{me.last_opponent, neigh[i].pos}, */
    /*                                          {me.last_opponent, neigh[j].pos}}; */
    /*                 if (wall_conflicts(me.graph, wall)) */
    /*                     continue; */

    /*                 struct graph_t *tmp = graph_copy(me.graph); */
    /*                 graph_remove_edge(tmp, wall[0].fr, wall[0].to); */
    /*                 graph_remove_edge(tmp, wall[1].fr, wall[1].to); */

    /*                 unsigned new_dist = bfs_distance(tmp, me.last_opponent, opp_target); */

    /*                 vertex_t my_target = */
    /*                     find_closest_objective(tmp, me.current, me.objectives,
     * me.num_objectives); */
    /*                 if (my_target == (vertex_t)-1) */
    /*                     my_target = me.start; */

    /*                 unsigned my_dist = bfs_distance(tmp, me.current, my_target); */

    /*                 if (new_dist != UINT_MAX && my_dist != UINT_MAX) { */
    /*                     int delta = (int)new_dist - (int)old_dist; */

    /*                     if (delta > best_delta) { */
    /*                         best_delta = delta; */
    /*                         best_wall[0] = wall[0]; */
    /*                         best_wall[1] = wall[1]; */
    /*                     } */
    /*                 } */
    /*                 graph_free(tmp); */
    /*             } */

    /*         if (best_delta >= 2) { */
    /*             for (int k = 0; k < 2; ++k) */
    /*                 graph_remove_edge(me.graph, best_wall[k].fr, best_wall[k].to); */

    /*             m.t = WALL; */
    /*             m.e[0] = best_wall[0]; */
    /*             m.e[1] = best_wall[1]; */
    /*             --me.num_walls; */
    /*             return m; */
    /*         } */
    /*     } */
    /* } */

    if (me.num_walls > 0 && me.last_opponent != (vertex_t)-1) {
        vertex_t opp_target;
        int next_my = next_objective(&me);
        if (next_my != -1)
            opp_target = me.graph->num_vertices - me.objectives[next_my] - 1;
        else
            opp_target = graph_get_start(me.graph, (me.color == BLACK) ? WHITE : BLACK);

        unsigned old_dist = bfs_distance(me.graph, me.last_opponent, opp_target);

        if (old_dist != UINT_MAX && old_dist <= 6) {
            struct pos_dir_t neigh[NUM_DIRS];
            unsigned n = graph_get_neighbors(me.graph, me.last_opponent, neigh);

            struct edge_t best_wall[2] = {{0, 0}, {0, 0}};
            int best_delta = 0;

            for (unsigned i = 0; i < n; ++i) {
                for (unsigned j = 0; j < n; ++j) {
                    if (i == j)
                        continue;
                    if (next_dir(neigh[i].dir) != neigh[j].dir)
                        continue;

                    struct edge_t wall[2] = {{me.last_opponent, neigh[i].pos},
                                             {me.last_opponent, neigh[j].pos}};

                    if (wall_conflicts(me.graph, wall))
                        continue;

                    struct graph_t *tmp = graph_copy(me.graph);
                    graph_remove_edge(tmp, wall[0].fr, wall[0].to);
                    graph_remove_edge(tmp, wall[1].fr, wall[1].to);

                    vertex_t opp_obj_arr[1] = {opp_target};
                    if (!has_path_to_objectives(tmp, me.last_opponent, opp_obj_arr, 1)) {
                        graph_free(tmp);
                        continue;
                    }

                    if (!wall_is_legal(tmp, wall, me.current, me.last_opponent, me.objectives,
                                       me.num_objectives, opp_obj_arr, 1)) {
                        graph_free(tmp);
                        continue;
                    }

                    unsigned new_dist = bfs_distance(tmp, me.last_opponent, opp_target);
                    unsigned my_dist = bfs_distance(
                        tmp, me.current,
                        find_closest_objective(tmp, me.current, me.objectives, me.num_objectives));

                    if (new_dist != UINT_MAX && my_dist != UINT_MAX) {
                        int delta = (int)new_dist - (int)old_dist;
                        if (delta > best_delta) {
                            best_delta = delta;
                            best_wall[0] = wall[0];
                            best_wall[1] = wall[1];
                        }
                    }

                    graph_free(tmp);
                }
            }

            if (best_delta >= 2) {
                for (int k = 0; k < 2; ++k)
                    graph_remove_edge(me.graph, best_wall[k].fr, best_wall[k].to);

                m.t = WALL;
                m.e[0] = best_wall[0];
                m.e[1] = best_wall[1];
                --me.num_walls;
                return m;
            }
        }
    }

    vertex_t target =
        find_closest_objective(me.graph, me.current, me.objectives, me.num_objectives);
    if (target == (vertex_t)-1)
        target = me.start;

    struct pos_dir_t moves[32];
    size_t nm = compute_valid_moves(me.graph, me.current, me.last_dir, me.last_opponent, moves);

    vertex_t best = me.current;
    enum dir_t best_dir = me.last_dir;
    unsigned best_gain = 0;
    double best_dist = DBL_MAX;
    int best_safe = -1;

    for (size_t i = 0; i < nm; ++i) {
        vertex_t cand = moves[i].pos;
        enum dir_t cdir = moves[i].dir;
        if (cand == me.current || cand == me.last_opponent)
            continue;

        unsigned gain = count_path_objectives(&me, me.current, cand);
        double dist = astar_distance(me.graph, cand, target);

        double gap = (me.last_opponent != (vertex_t)-1)
                         ? astar_distance(me.graph, cand, me.last_opponent)
                         : 10.0;
        int safe = (gap >= 2.0);

        int better = 0;
        if (gain > best_gain)
            better = 1;
        else if (gain == best_gain && dist < best_dist)
            better = 1;
        else if (gain == best_gain && fabs(dist - best_dist) < 1e-6 && safe > best_safe)
            better = 1;

        if (better) {
            best_gain = gain;
            best_dist = dist;
            best_safe = safe;
            best = cand;
            best_dir = cdir;
        }
    }

    mark_path_objectives(&me, me.current, best);
    me.current = best;
    me.last_dir = best_dir;

    m.m = best;
    return m;
}

void finalize() {
    free(me.objectives);
    graph_free(me.graph);
}
