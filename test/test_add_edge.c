#include "graph.h"
#include "move.h" // Pour les directions EAST, WEST...
#include <assert.h>
#include <stdio.h>

void test_add_edge() {
    struct graph_t *g = graph_create(TRIANGULAR, 3);

    // Vérifie qu’il y a bien une arête entre 0 et un de ses voisins connus
    struct pos_dir_t neighbors[NUM_DIRS];
    unsigned int n = graph_get_neighbors(g, 0, neighbors);

    // Vérifie que tous les voisins détectés sont effectivement connectés
    for (unsigned int i = 0; i < n; ++i) {
        vertex_t target = neighbors[i].pos;
        enum dir_t dir = neighbors[i].dir;
        assert(graph_has_edge(g, 0, target));
        assert(graph_get_direction(g, 0, target) == dir);
    }

    printf("test_add_edge : OK\n");
    graph_free(g);
}
