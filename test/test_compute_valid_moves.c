/* test/test_compute_valid_moves.c
 * ----------------------------------------------------------
 * Vérifie que compute_valid_moves() respecte les règles :
 *  – distance 1 quand last_dir = NO_EDGE ;
 *  – distance 3 dans la même direction que last_dir ;
 *  – distance 2 dans une direction à ±30° de last_dir.
 * ---------------------------------------------------------- */
#include <assert.h>
#include <stdio.h>

#include "commun.h"
#include "graph.h"

/* ------------------------------------------------------------------ */
/* 1)  ré‑expose la variable globale déjà définie dans commun.c        */
/* ------------------------------------------------------------------ */
extern struct player_info me; /* <‑‑ évite le “multiple definition” */

/* ------------------------------------------------------------------ */
/* 2)  petit utilitaire : direction −30 ° (si non fourni ailleurs)     */
/* ------------------------------------------------------------------ */
#ifndef HAVE_PREV_DIR
static inline enum dir_t prev_dir(enum dir_t d) /* 0..5 -> 5,0,1,2,3,4   */
{
    return (d == 0) ? 5 : (enum dir_t)(d - 1);
}
#endif

/* ------------------------------------------------------------------ */
/* 3)  cherche un sommet bien “au milieu” pour que les 6 voisins       */
/*     existent (ça évite les faux négatifs quand on teste les 3 pas)  */
/* ------------------------------------------------------------------ */
static vertex_t central_vertex(const struct graph_t *g) {
    struct pos_dir_t nbr[NUM_DIRS];
    vertex_t best = 0;
    unsigned best_deg = 0;

    for (vertex_t v = 0; v < g->num_vertices; ++v) {
        unsigned d = graph_get_neighbors(g, v, nbr);
        if (d > best_deg) {
            best = v;
            best_deg = d;
        }
        if (best_deg == NUM_DIRS)
            break; /* sommet “plein” trouvé */
    }
    return best;
}

/* ------------------------------------------------------------------ */
void test_compute_valid_moves_on_triangle_graph(void) {
    printf("Running test_compute_valid_moves_on_triangle_graph...\n");

    struct graph_t *g = graph_create(TRIANGULAR, 6);
    vertex_t pos = central_vertex(g); /* sommet de départ      */

    struct pos_dir_t moves[64];

    /* === (1) distance 1 quand on n’a pas encore bougé ================ */
    me.last_dir = NO_EDGE;
    size_t c1 = compute_valid_moves(g, pos, me.last_dir, (vertex_t)-1, moves);
    assert(c1 > 0); /* au moins ses voisins  */
    for (size_t i = 0; i < c1; ++i)
        assert(graph_get_direction(g, pos, moves[i].pos) != NO_EDGE);

    /* Pré‑calcul des voisins de pos                                     */
    struct pos_dir_t neigh[NUM_DIRS];
    unsigned n = graph_get_neighbors(g, pos, neigh);

    /* === (2) distance 3 dans la même direction que last_dir ========= */
    int ok_3 = 0;
    for (unsigned i = 0; i < n && !ok_3; ++i) {
        enum dir_t dir0 = neigh[i].dir;
        vertex_t s1 = neigh[i].pos;

        struct pos_dir_t n2[NUM_DIRS];
        unsigned nn2 = graph_get_neighbors(g, s1, n2);
        vertex_t s2 = (vertex_t)-1;
        for (unsigned j = 0; j < nn2; ++j)
            if (n2[j].dir == dir0) {
                s2 = n2[j].pos;
                break;
            }
        if (s2 == (vertex_t)-1)
            continue;

        struct pos_dir_t n3[NUM_DIRS];
        unsigned nn3 = graph_get_neighbors(g, s2, n3);
        vertex_t s3 = (vertex_t)-1;
        for (unsigned k = 0; k < nn3; ++k)
            if (n3[k].dir == dir0) {
                s3 = n3[k].pos;
                break;
            }
        if (s3 == (vertex_t)-1)
            continue;

        me.last_dir = dir0;
        size_t c3 = compute_valid_moves(g, pos, me.last_dir, (vertex_t)-1, moves);
        for (size_t t = 0; t < c3; ++t)
            if (moves[t].pos == s3) {
                ok_3 = 1;
                break;
            }
    }
    assert(ok_3 && "déplacement 3 cases dans la même direction non trouvé");

    /* === (3) distance 2 à ±30° du last_dir =========================== */
    int ok_2_30 = 0;
    for (unsigned i = 0; i < n && !ok_2_30; ++i) {
        enum dir_t last_dir = neigh[i].dir;
        enum dir_t d30[2] = {next_dir(last_dir), prev_dir(last_dir)};

        for (int s = 0; s < 2 && !ok_2_30; ++s) {
            enum dir_t dir30 = d30[s];

            /* première case dans la direction dir30                     */
            vertex_t p1 = (vertex_t)-1;
            for (unsigned j = 0; j < n; ++j)
                if (neigh[j].dir == dir30) {
                    p1 = neigh[j].pos;
                    break;
                }
            if (p1 == (vertex_t)-1)
                continue;

            /* seconde case dans la même direction                       */
            struct pos_dir_t n2[NUM_DIRS];
            unsigned nn2 = graph_get_neighbors(g, p1, n2);
            vertex_t p2 = (vertex_t)-1;
            for (unsigned k = 0; k < nn2; ++k)
                if (n2[k].dir == dir30) {
                    p2 = n2[k].pos;
                    break;
                }
            if (p2 == (vertex_t)-1)
                continue;

            me.last_dir = last_dir;
            size_t c = compute_valid_moves(g, pos, me.last_dir, (vertex_t)-1, moves);
            for (size_t t = 0; t < c; ++t)
                if (moves[t].pos == p2) {
                    ok_2_30 = 1;
                    break;
                }
        }
    }
    assert(ok_2_30 && "déplacement 2 cases à 30° non trouvé");

    graph_free(g);
    printf("test_compute_valid_moves_on_triangle_graph passed.\n");
}
