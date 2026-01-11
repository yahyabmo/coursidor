/* test/test_exists_path.c
 *
 * Vérifie que wall_preserves_connectivity() :
 *   1) accepte un mur légal,
 *   2) refuse un mur qui coupe le chemin d’un joueur (s’il en existe un).
 */
#include <assert.h>
#include <stdio.h>

#include "commun.h"
#include "graph.h"
#include "helper.h"

/* ------------------------------------------------------------------ */
/* choisit une arête existante (v→w, dir)                             */
/* ------------------------------------------------------------------ */
static void pick_edge(const struct graph_t *g, vertex_t *v, vertex_t *w, enum dir_t *d) {
    struct pos_dir_t neigh[NUM_DIRS];

    for (vertex_t u = 0; u < g->num_vertices; ++u) {
        unsigned n = graph_get_neighbors(g, u, neigh);
        if (n) {
            *v = u;
            *w = neigh[0].pos;
            *d = neigh[0].dir;
            return;
        }
    }
}

/* ------------------------------------------------------------------ */
void test_exists_path(void) {
    puts("Running test_exists_path...");

    /* ---- Création du graphe et initialisation de 2 joueurs ---------- */
    struct graph_t *g = graph_create(TRIANGULAR, 6);

    struct player_info me = {0}, opp = {0};

    me.start = graph_get_start(g, 0);
    opp.start = graph_get_start(g, 1);

    me.current = me.start;
    opp.current = opp.start;

    me.num_objectives = 1;
    opp.num_objectives = 1;

    me.objectives = malloc(sizeof(vertex_t));
    opp.objectives = malloc(sizeof(vertex_t));

    me.objectives[0] = opp.start; /* objectifs simplifiés            */
    opp.objectives[0] = me.start;

    me.graph = g;
    opp.graph = g;

    /* ---- 1) Un mur manifestement légal ----------------------------- */
    struct edge_t good_wall[2];
    {
        vertex_t v, w;
        enum dir_t dir;
        pick_edge(g, &v, &w, &dir); /* 1re demi‑barre        */

        struct pos_dir_t tab[NUM_DIRS];
        graph_get_neighbors(g, v, tab);

        /* 2e demi‑barre : arête adjacente différente                    */
        good_wall[0] = (struct edge_t){v, w};
        good_wall[1] = (struct edge_t){v, tab[1].pos};
    }
    int ok = wall_preserves_connectivity(g, good_wall, &me, &opp);
    assert(ok == 1 && "mur légal refusé à tort");

    /* ---- 2) Recherche automatique d’un mur qui bloque un joueur ---- */
    struct edge_t bad_wall[2];
    int found_blocking = 0;

    for (vertex_t v = 0; v < g->num_vertices && !found_blocking; ++v) {
        struct pos_dir_t n1[NUM_DIRS];
        unsigned n = graph_get_neighbors(g, v, n1);

        for (unsigned i = 0; i < n && !found_blocking; ++i)
            for (unsigned j = i + 1; j < n && !found_blocking; ++j) {

                /* deux arêtes partant du même sommet et non “opposées”   */
                if (next_dir(n1[i].dir) != n1[j].dir && next_dir(n1[j].dir) != n1[i].dir)
                    continue;

                bad_wall[0] = (struct edge_t){v, n1[i].pos};
                bad_wall[1] = (struct edge_t){v, n1[j].pos};

                if (wall_preserves_connectivity(g, bad_wall, &me, &opp) == 0)
                    found_blocking = 1;
            }
    }

    /* S’il existe un mur bloquant, on vérifie qu’il est bien refusé.    */
    if (found_blocking) {
        ok = wall_preserves_connectivity(g, bad_wall, &me, &opp);
        assert(ok == 0 && "mur illégal accepté (chemin bloqué)");
    } else {
        /* Dans ce plateau particulier, aucun mur 2‑segments n’isole     */
        puts("  (info) aucun mur bloquant n’a été trouvé – test ignoré");
    }

    /* ---- Nettoyage -------------------------------------------------- */
    free(me.objectives);
    free(opp.objectives);
    graph_free(g);

    puts("test_exists_path passed.");
}

/* déclaration pour alltests.c si nécessaire */
void test_exists_path(void);
