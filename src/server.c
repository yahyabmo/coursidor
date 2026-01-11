#include <dlfcn.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "board.h"
#include "commun.h"
#include "display.h"
#include "graph.h"
#include "load_player.h"
#include "move.h"
#include "player.h"

int compute_next_player(int p) { return 1 - p; }

int is_invalid(struct move_t move) { return move.t == NO_TYPE; }

void print_usage(char *progname) {
    fprintf(stderr, "Usage: %s [-m size] [-t type] [-M max_turns] player1.so player2.so\n",
            progname);
}

int parse_arguments(int argc, char **argv, char *progname, enum graph_type_t *type, unsigned int *m,
                    unsigned int *max_turns, char **player1, char **player2) {
    *type = TRIANGULAR;
    *m = 6;
    *max_turns = 0;

    int opt;
    while ((opt = getopt(argc, argv, "m:t:M:")) != -1) {
        switch (opt) {
        case 'm':
            *m = atoi(optarg);
            break;
        case 't':
            if (strcmp(optarg, "T") == 0)
                *type = TRIANGULAR;
            else if (strcmp(optarg, "C") == 0)
                *type = CYCLIC;
            else if (strcmp(optarg, "H") == 0)
                *type = HOLEY;
            else {
                fprintf(stderr, "Invalid graph type: %s\n", optarg);
                return 0;
            }
            break;
        case 'M':
            *max_turns = atoi(optarg);
            break;
        default:
            print_usage(progname);
            return 0;
        }
    }

    if (argc - optind != 2) {
        print_usage(progname);
        return 0;
    }

    *player1 = argv[optind];
    *player2 = argv[optind + 1];
    return 1;
}

int main(int argc, char **argv) {
    enum graph_type_t type;
    unsigned int m, max_turns;
    char *player1_path, *player2_path;

    if (!parse_arguments(argc, argv, argv[0], &type, &m, &max_turns, &player1_path, &player2_path))
        return EXIT_FAILURE;

    struct player_t players[2];
    players[0] = load_player(player1_path);
    players[1] = load_player(player2_path);

    int first_player = rand() % 2;
    struct board_t *board = board_create(type, m, first_player);
    if (!board) {
        fprintf(stderr, "Erreur lors de la création du plateau\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 2; ++i) {
        unsigned int color = (i == first_player) ? BLACK : WHITE;
        set_player_id_from_server(i);
        players[i].initialize(color, board_get_graph(board, i));
    }

    struct move_t current_move = {.t = NO_TYPE, .c = NO_COLOR};
    int current = first_player;
    unsigned int turn_count = 0;

    while (1) {
        if (max_turns > 0 && turn_count >= max_turns) {
            printf("Nombre maximal de tours atteint (%u).\n", max_turns);
            break;
        }

        current_move = players[current].play(current_move);

        if (current_move.t == MOVE) {
            printf("[SERVER] Tour %u : Joueur %d a joué MOVE vers %u\n", turn_count, current,
                   current_move.m);
        } else if (current_move.t == WALL) {
            printf("[SERVER] Tour %u : Joueur %d a joué WALL entre [%u-%u] et [%u-%u]\n",
                   turn_count, current, current_move.e[0].fr, current_move.e[0].to,
                   current_move.e[1].fr, current_move.e[1].to);
        } else {
            printf("[SERVER] Tour %u : Joueur %d a passé son tour (NO_TYPE)\n", turn_count,
                   current);
        }

        /* if (current_move.t == WALL) { */
        /*     board_add_walls(board, current, current_move); */
        if (current_move.t == WALL) {
            struct graph_t *server_g = board_get_server_graph(board);

            struct graph_t *test_graph = graph_copy(server_g);
            for (int i = 0; i < 2; ++i) {
                graph_remove_edge(test_graph, current_move.e[i].fr, current_move.e[i].to);
            }

            // Vérifier si les deux joueurs peuvent encore atteindre leurs objectifs
            vertex_t p0 = board_get_position(board, 0);
            vertex_t p1 = board_get_position(board, 1);

            size_t num_obj = board_get_objective_count(board);
            vertex_t *objectives0 = malloc(sizeof(vertex_t) * num_obj);
            vertex_t *objectives1 = malloc(sizeof(vertex_t) * num_obj);

            for (unsigned int i = 0; i < num_obj; ++i) {
                objectives0[i] = board_get_objective(board, 0, i);
                objectives1[i] = board_get_objective(board, 1, i);
            }

            int ok0 = has_path_to_objectives(test_graph, p0, objectives0, num_obj);
            int ok1 = has_path_to_objectives(test_graph, p1, objectives1, num_obj);

            free(objectives0);
            free(objectives1);
            graph_free(test_graph);

            if (ok0 && ok1) {
                printf("[SERVER] Le mur est valide, on l'applique !\n");
                board_add_walls(board, current, current_move);
            } else {
                printf("[SERVER] Mur illégal ! Mur ignoré.\n");
                // Le mur est interdit : on ignore le move
            }
        }

        /*  else if (current_move.t == MOVE) { */
        /*     board_set_position(board, current, current_move.m); */
        /*     board_mark_objective_reached(board, current, current_move.m); */
        /* } */
        else if (current_move.t == MOVE) {
            vertex_t old_pos = board_get_position(board, current); /* position avant */
            board_set_position(board, current, current_move.m);

            /* marquer tous les objectifs rencontrés sur le chemin */
            board_mark_path_objectives(board, current, old_pos, current_move.m);
        }

        // Objectifs pour affichage (non modifiés)
        size_t num_obj = board_get_objective_count(board);
        vertex_t *display_obj = malloc(sizeof(vertex_t) * num_obj);
        for (unsigned int i = 0; i < num_obj; ++i)
            display_obj[i] = board_get_display_objective(board, i);

        vertex_t p0 = board_get_position(board, 0);
        vertex_t p1 = board_get_position(board, 1);
        // print_game(board_get_graph(board, 0), m, p0, p1, display_obj, num_obj);
        struct graph_t *g = board_get_graph(board, 0);
        if (g->type == CYCLIC)
            print_game_cyclic(g, m, p0, p1, display_obj, num_obj);
        else if (g->type == HOLEY)
            print_game_holed(g, m, p0, p1, display_obj, num_obj);
        else
            print_game_triangular(g, m, p0, p1, display_obj, num_obj);
        printf("\nObjectifs restants :\n");
        for (int pid = 0; pid < 2; ++pid) {
            printf("  Joueur %d (%s):", pid, (pid == 0) ? "BLACK" : "WHITE");
            for (unsigned int j = 0; j < num_obj; ++j) {
                vertex_t obj = board_get_objective(board, pid, j);
                if (obj != (vertex_t)-1)
                    printf(" %u", obj);
            }
            printf("\n");
        }

        free(display_obj);

        // Objectifs réels (modifiés) pour la condition de victoire
        // Après avoir traité un mouvement :
        vertex_t *real_obj = malloc(sizeof(vertex_t) * num_obj);
        for (unsigned int i = 0; i < num_obj; ++i)
            real_obj[i] = board_get_objective(board, current, i); // Utiliser l'ID du joueur actuel

        struct player_info info;
        info.current = board_get_position(board, current);
        info.start = board_get_start(board, current);
        info.num_objectives = num_obj;
        info.objectives = real_obj;
        info.graph = board_get_graph(board, current);

        if (is_invalid(current_move) || has_won_info(&info)) {
            printf("Player %d wins!\n", current);
            free(real_obj);
            break;
        }

        free(real_obj);
        current = compute_next_player(current);
        turn_count++;
    }

    for (int i = 0; i < 2; ++i)
        players[i].finalize();

    board_destroy(board);
    close_player(&players[0]);
    close_player(&players[1]);
    printf("Partie terminée.\n");
    return 0;
}
