#include "board.h"
#include "commun.h"
#include "move.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct board_t {
    struct graph_t *graphs[2];
    struct graph_t *server_graph;
    vertex_t positions[2];
    vertex_t start[2];
    vertex_t *objectives[2];      // utilisés pour vérification victoire
    vertex_t *display_objectives; // objectifs à afficher (jamais modifiés)
    unsigned int num_objectives;
};

struct board_t *board_create(enum graph_type_t type, unsigned int m, int first_player) {
    struct board_t *board = malloc(sizeof(struct board_t));
    if (!board)
        return NULL;

    struct graph_t *base = graph_create(type, m);

    board->server_graph = base; // <--- garder la version centrale pour le serveur !

    for (int i = 0; i < 2; ++i) {
        board->graphs[i] = graph_copy(base); // <--- chaque joueur a sa propre copie
        unsigned int color = (i == first_player) ? BLACK : WHITE;
        board->start[i] = graph_get_start(base, color);
        board->positions[i] = board->start[i];
    }

    board->num_objectives = base->num_objectives;
    board->objectives[0] = malloc(sizeof(vertex_t) * base->num_objectives);
    board->objectives[1] = malloc(sizeof(vertex_t) * base->num_objectives);
    memcpy(board->objectives[0], base->objectives, sizeof(vertex_t) * base->num_objectives);
    memcpy(board->objectives[1], base->objectives, sizeof(vertex_t) * base->num_objectives);

    board->display_objectives = malloc(sizeof(vertex_t) * base->num_objectives);
    memcpy(board->display_objectives, base->objectives, sizeof(vertex_t) * base->num_objectives);

    return board;
}

struct graph_t *board_get_graph(const struct board_t *board, int player_id) {
    return board->graphs[player_id];
}

vertex_t board_get_position(const struct board_t *board, int player_id) {
    return board->positions[player_id];
}

void board_set_position(struct board_t *board, int player_id, vertex_t new_pos) {
    board->positions[player_id] = new_pos;
}

vertex_t board_get_start(const struct board_t *board, int player_id) {
    return board->start[player_id];
}

unsigned int board_get_objective_count(const struct board_t *board) {
    return board->num_objectives;
}

void board_destroy(struct board_t *board) {
    for (int i = 0; i < 2; ++i) {
        free(board->objectives[i]);
    }
    graph_free(board->server_graph);
    free(board->display_objectives);
    free(board);
}

void board_mark_objective_reached(struct board_t *board, int player_id, vertex_t v) {
    for (unsigned int i = 0; i < board->num_objectives; ++i) {
        if (board->objectives[player_id][i] == v) {
            board->objectives[player_id][i] = (vertex_t)-1;
            printf(" OBJECTIF atteint : joueur %d a atteint %u\n", player_id, v);
        }
    }
}
vertex_t board_get_objective(const struct board_t *board, int player_id, unsigned int i) {
    return (i < board->num_objectives) ? board->objectives[player_id][i] : (vertex_t)-1;
}

vertex_t board_get_display_objective(const struct board_t *board, unsigned int i) {
    return (i < board->num_objectives) ? board->display_objectives[i] : (vertex_t)-1;
}

void board_add_walls(struct board_t *board, int player_id, struct move_t move) {
    struct graph_t *g = board->graphs[player_id];
    struct graph_t *g_opponent = board->graphs[1 - player_id];
    struct graph_t *server_g = board->server_graph;

    for (int i = 0; i < 2; ++i) {
        vertex_t from = move.e[i].fr;
        vertex_t to = move.e[i].to;

        if (from < g->num_vertices && to < g->num_vertices) {
            if (graph_has_edge(g, from, to)) {
                graph_remove_edge(g, from, to);
                graph_remove_edge(g, to, from);
            }
            if (graph_has_edge(g_opponent, from, to)) {
                graph_remove_edge(g_opponent, from, to);
                graph_remove_edge(g_opponent, to, from);
            }
            if (graph_has_edge(server_g, from, to)) {
                graph_remove_edge(server_g, from, to);
                graph_remove_edge(server_g, to, from);
            }
        }
    }
}

struct graph_t *board_get_server_graph(const struct board_t *board) { return board->server_graph; }

/* ------------------------------------------------------------------ */
/*  Marquer tous les objectifs traversés entre from et to             */
/* ------------------------------------------------------------------ */
void board_mark_path_objectives(struct board_t *board, int player_id, vertex_t from, vertex_t to) {
    vertex_t step = from;
    while (step != to) {
        /* Avance d’une case “en ligne droite” (même fonction que helper) */
        step = shortest_path(board->server_graph, step, to);
        if (step == (vertex_t)-1) /* Chemin impossible ?  */
            break;

        /* Si on tombe sur un objectif, on l’enlève */
        for (unsigned int i = 0; i < board->num_objectives; ++i) {
            if (board->objectives[player_id][i] == step) {
                board->objectives[player_id][i] = (vertex_t)-1;
                printf(" OBJECTIF atteint (path) : joueur %d a atteint %u\n", player_id, step);
            }
        }
    }
}
