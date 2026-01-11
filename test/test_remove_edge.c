#include "graph.h"
#include "move.h" // pour les directions
#include <assert.h>
#include <stdio.h>

void test_remove_edge() {
    struct graph_t *g = graph_create(TRIANGULAR, 3);

    // Choisir deux sommets voisins
    struct pos_dir_t neighbors[NUM_DIRS];
    unsigned int n = graph_get_neighbors(g, 0, neighbors);
    assert(n > 0); // Il faut au moins un voisin

    vertex_t neighbor = neighbors[0].pos;

    // Vérifie que l’arête existe
    assert(graph_has_edge(g, 0, neighbor));
    assert(graph_has_edge(g, neighbor, 0));

    // Supprimer l’arête
    graph_remove_edge(g, 0, neighbor);

    // Vérifier qu’elle n'existe plus dans les deux sens
    assert(!graph_has_edge(g, 0, neighbor));
    assert(!graph_has_edge(g, neighbor, 0));

    printf("test_remove_edge : OK\n");
    graph_free(g);
}
