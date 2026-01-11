/* test/test_neighbors_vertex.c
 *
 * Vérifie que graph_get_neighbors() :
 *  - ne renvoie que des arêtes valides (existantes et non mur),
 *  - fournit la bonne direction pour chaque voisin,
 *  - réagit bien à la suppression d’une arête (sur une copie).
 */
#include <assert.h>
#include <stdio.h>

#include "commun.h" // pour graph_has_edge, graph_get_direction, opposite_dir
#include "graph.h"

void test_neighbors_vertex(void) {
    puts("Running test_neighbors_vertex...");

    /* On crée un petit graphe triangulaire de côté 3 */
    struct graph_t *g = graph_create(TRIANGULAR, 3);
    struct pos_dir_t nbr[NUM_DIRS];

    for (vertex_t v = 0; v < g->num_vertices; ++v) {
        /* 1) vérification des voisins “normaux” */
        unsigned n = graph_get_neighbors(g, v, nbr);
        for (unsigned i = 0; i < n; ++i) {
            vertex_t u = nbr[i].pos;
            enum dir_t d = nbr[i].dir;
            assert(graph_has_edge(g, v, u) && "get_neighbors a retourné un voisin invalide");
            assert(graph_get_direction(g, v, u) == d && "direction incorrecte pour un voisin");
        }

        /* 2) test de la suppression d’une arête – sur une copie du graphe */
        if (n > 0) {
            vertex_t u0 = nbr[0].pos;

            /* on copie le graphe, on y enlève l’arête, on recalcule */
            struct graph_t *gc = graph_copy(g);
            graph_remove_edge(gc, v, u0);

            unsigned m = graph_get_neighbors(gc, v, nbr);
            assert(m == n - 1 && "remove_edge n'a pas réduit le nombre de voisins attendu");

            graph_free(gc);
        }
    }

    graph_free(g);
    puts("test_neighbors_vertex passed.");
}

/* Permet à alltests.c de déclarer ce test */
void test_neighbors_vertex(void);
