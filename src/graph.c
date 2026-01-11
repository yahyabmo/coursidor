#include "graph.h"
#include <stdlib.h>
#include <string.h>

#define MAX3(a, b, c) (((a) > (b)) ? (((a) > (c)) ? (a) : (c)) : (((b) > (c)) ? (b) : (c)))
// Macro pour calculer le nombre de sommets dans un graph triangulaire
// hexagonale de cote m
#define TRIANGULAR_NUM_VERTICES(m) 3 * m *m - 3 * m + 1

typedef struct {
    int dq;
    int dr;
} hex_dir_t;

typedef struct {
    int q;
    int r;
} coord_t;

// tableau des vecteurs ajoutés pour les 6 directions
static const hex_dir_t hex_dirs[NUM_DIRS + 1] = {
    [NW] = {0, -1}, [NE] = {1, -1}, [E] = {1, 0}, [SE] = {0, 1}, [SW] = {-1, 1}, [W] = {-1, 0}};

// Fonction helper pour trouver l'indice d'un sommet avec des coordonnées (q, r)
static int find_vertex_id(coord_t *coords, unsigned int num_vertices, int q, int r) {
    for (unsigned int i = 0; i < num_vertices; i++) {
        if (coords[i].q == q && coords[i].r == r)
            return i;
    }
    return -1;
}

gsl_spmatrix_uint *gsl_spmatrix_uint_alloc_copy_fallback(const gsl_spmatrix_uint *src) {
    if (!src)
        return NULL;

    gsl_spmatrix_uint *copy =
        gsl_spmatrix_uint_alloc_nzmax(src->size1, src->size2, src->nzmax, src->sptype);
    if (!copy)
        return NULL;

    copy->nz = src->nz;
    for (size_t i = 0; i < src->nz; ++i) {
        copy->data[i] = src->data[i];
        copy->i[i] = src->i[i];
    }

    size_t p_size;
    if (src->sptype == GSL_SPMATRIX_COO) {
        p_size = src->nz;
    } else if (src->sptype == GSL_SPMATRIX_CSR) {
        p_size = src->size1 + 1;
    } else {
        // GSL_SPMATRIX_CSC
        p_size = src->size2 + 1;
    }

    for (size_t k = 0; k < p_size; ++k) {
        copy->p[k] = src->p[k];
    }

    return copy;
}

struct graph_t *graph_create(enum graph_type_t type, unsigned int m) {
    struct graph_t *g = malloc(sizeof(struct graph_t));
    if (!g) {
        return NULL;
    }

    g->type = type;

    switch (type) {
    case TRIANGULAR:
        g->num_vertices = 3 * m * m - 3 * m + 1;
        g->num_edges = 9 * m * m - 15 * m + 6;
        break;

    case CYCLIC:
        g->num_vertices = 12 * m - 18;
        g->num_edges = 24 * m - 36;
        printf("CYCLIC with m=%u: num_vertices = %u\n", m, g->num_vertices);
        break;

    case HOLEY:
        g->num_vertices = (2 * m * m) / 3 + 18 * m - 48;
        g->num_edges = 2 * m * m + 34 * m - 78;
        break;
    }
    // Appeler la fonction d'initialisation selon le type de graphe
    // allocation de la matrice d adjacance
    g->t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
    g->start[0] = 0; // Example valid start for BLACK
    g->start[1] = g->num_vertices - 1;
    // Appeler la fonction d'initialisation selon le type de graphe
    switch (type) {
    case TRIANGULAR:
        graph_initialize_triangular(g, m);
        break;
    case CYCLIC:
        graph_initialize_cyclic(g, m);
        break;
    case HOLEY:
        graph_initialize_holed(g, m);
        break;
    }

    gsl_spmatrix_uint *compressed = gsl_spmatrix_uint_compress(g->t, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(g->t);
    g->t = compressed;
    /* g->start[0] = 0; // Example valid start for BLACK */
    /* g->start[1] = g->num_vertices - 1; */

    return g;
}

void graph_initialize_triangular(struct graph_t *g, unsigned int m) {
    coord_t *coords = malloc(g->num_vertices * sizeof(coord_t));
    if (!coords) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    /* La condition max(|q|, |r|, |s|) < m avec s = -q - r est une manière
  standard de définir un plateau hexagonal dans un système de coordonnées
  axiales.

  rq:
  La distance d'une cellule au centre (q = 0,r = 0,s = 0) se mesure par :
  distance=max⁡(∣q∣,∣r∣,∣s∣).
     */
    unsigned int vertex_id = 0;
    for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) {
        for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) {
            int s = -q - r;
            if (MAX3(abs(q), abs(r), abs(s)) < (int)m) {
                coords[vertex_id].q = q;
                coords[vertex_id].r = r;
                vertex_id++;
            }
        }
    }

    for (int i = 0; i < (int)g->num_vertices; i++) {
        for (enum dir_t d = NW; d <= W; d++) {
            int nq = coords[i].q + hex_dirs[d].dq;
            int nr = coords[i].r + hex_dirs[d].dr;

            int neighbor_id = find_vertex_id(coords, g->num_vertices, nq, nr);
            if (neighbor_id != -1) {
                graph_add_edge(g, i, neighbor_id, d);
                graph_add_edge(g, neighbor_id, i, opposite_dir(d));
            }
        }
    }

    g->num_objectives = 4;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    graph_initialize_objectives(g);
    free(coords);
}

void graph_initialize_cyclic(struct graph_t *g, unsigned int m) {
    coord_t *coords = malloc(g->num_vertices * sizeof(coord_t));
    if (!coords) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    unsigned int vertex_id = 0;
    for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) {
        for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) {
            int s = -q - r;
            int dist = MAX3(abs(q), abs(r), abs(s));
            if (dist >= (int)(m - 2) && dist < (int)m) {
                coords[vertex_id].q = q;
                coords[vertex_id].r = r;
                vertex_id++;
            }
        }
    }

    for (int i = 0; i < (int)g->num_vertices; i++) {
        for (enum dir_t d = NW; d <= W; d++) {
            int nq = coords[i].q + hex_dirs[d].dq;
            int nr = coords[i].r + hex_dirs[d].dr;

            int ns = -nq - nr;
            int dist = MAX3(abs(nq), abs(nr), abs(ns));
            if (dist >= (int)(m - 2) && dist < (int)m) {
                int neighbor_id = find_vertex_id(coords, g->num_vertices, nq, nr);
                if (neighbor_id != -1) {
                    graph_add_edge(g, i, neighbor_id, d);
                    graph_add_edge(g, neighbor_id, i, opposite_dir(d));
                }
            }
        }
    }

    g->num_objectives = 4;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    graph_initialize_objectives(g);

    free(coords);
}

void graph_initialize_holed(struct graph_t *g, unsigned int m) {
    // Allocation généreuse (sur base triangulaire max)
    coord_t *coords = malloc((3 * m * m - 3 * m + 1) * sizeof(coord_t));
    if (!coords) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    // Taille des trous
    int hole_size = m / 3 - 1;

    // Définir les centres des 7 trous
    int centers[7][2] = {
        {-2 * hole_size + 2 * (hole_size), -(3 * (hole_size) - (hole_size - 1))}, // haut gauche
        {2 * hole_size + 1, -(3 * (hole_size) - (hole_size - 1))},                // haut droite
        {-2 * hole_size - 1, 0},                                                  // milieu gauche
        {0, 0},                                                                   // centre
        {2 * hole_size + 1, 0},                                                   // milieu droite
        {-2 * hole_size - 1, (3 * (hole_size) - (hole_size - 1))},                // bas gauche
        {2 * hole_size - 2 * (hole_size), (3 * (hole_size) - (hole_size - 1))}    // bas droite
    };

    // Fonction helper pour savoir si un sommet est dans un trou
    int is_in_cyclic_hole(int q, int r) {
        int s = -q - r;
        for (int i = 0; i < 7; i++) {
            int cq = centers[i][0];
            int cr = centers[i][1];
            int cs = -cq - cr;
            int dist = MAX3(abs(q - cq), abs(r - cr), abs(s - cs));
            if (dist < hole_size)
                return 1;
        }
        return 0;
    }

    // Remplir les sommets valides (hors des 7 trous)
    unsigned int vertex_id = 0;
    for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) {
        for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) {
            int s = -q - r;
            int dist = MAX3(abs(q), abs(r), abs(s));
            if (dist < (int)m && !is_in_cyclic_hole(q, r)) {
                coords[vertex_id].q = q;
                coords[vertex_id].r = r;
                vertex_id++;
            }
        }
    }

    g->num_vertices = vertex_id;

    // Création de la matrice (attention : libérer celle de graph_create)
    gsl_spmatrix_uint_free(g->t);
    g->t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);

    // Ajouter les arêtes
    for (unsigned int i = 0; i < g->num_vertices; i++) {
        for (enum dir_t d = NW; d <= W; d++) {
            int nq = coords[i].q + hex_dirs[d].dq;
            int nr = coords[i].r + hex_dirs[d].dr;
            int ns = -nq - nr;

            int dist = MAX3(abs(nq), abs(nr), abs(ns));
            if (dist < (int)m && !is_in_cyclic_hole(nq, nr)) {
                int neighbor_id = find_vertex_id(coords, g->num_vertices, nq, nr);
                if (neighbor_id != -1) {
                    graph_add_edge(g, i, neighbor_id, d);
                    graph_add_edge(g, neighbor_id, i, opposite_dir(d));
                }
            }
        }
    }

    // Objectifs aléatoires
    g->num_objectives = 4;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    graph_initialize_objectives(g);

    free(coords);
}

void graph_add_edge(struct graph_t *g, vertex_t i, vertex_t j, enum dir_t dir) {
    if (i >= g->num_vertices || j >= g->num_vertices) {
        fprintf(stderr, "ERROR: Edge out of bounds (%u → %u)\n", i, j);
        exit(1);
    }

    gsl_spmatrix_uint_set(g->t, i, j, dir);
}

void graph_free(struct graph_t *g) {
    if (!g)
        return;
    gsl_spmatrix_uint_free(g->t);
    free(g->objectives);
    free(g);
}

unsigned int graph_num_vertices(const struct graph_t *g) { return g->num_vertices; }

unsigned int graph_num_objectives(const struct graph_t *g) { return g->num_objectives; }

vertex_t graph_get_objective(const struct graph_t *g, unsigned int i) { return g->objectives[i]; }

vertex_t graph_get_start(const struct graph_t *g, unsigned int player_id) {
    return g->start[player_id];
}

unsigned int graph_get_neighbors(const struct graph_t *g, vertex_t vertex,
                                 struct pos_dir_t neighbors[]) {
    unsigned int count = 0;

    for (int k = g->t->p[vertex]; k < g->t->p[vertex + 1]; ++k) {
        if (g->t->data[k] != WALL_DIR) {
            neighbors[count].pos = g->t->i[k];
            neighbors[count].dir = g->t->data[k];
            count++;
        }
    }

    return count;
}

int graph_has_edge(const struct graph_t *g, vertex_t from, vertex_t to) {

    for (int k = g->t->p[from]; k < g->t->p[from + 1]; ++k) {
        if ((vertex_t)g->t->i[k] == to && g->t->data[k] != WALL_DIR)
            return 1;
    }
    return 0;
}

enum dir_t graph_get_direction(const struct graph_t *g, vertex_t from, vertex_t to) {
    for (int k = g->t->p[from]; k < g->t->p[from + 1]; ++k) {
        if ((vertex_t)g->t->i[k] == to && g->t->data[k] != WALL_DIR)
            return g->t->data[k];
    }
    return NO_EDGE;
}

/* struct graph_t *graph_copy(const struct graph_t *src) { */
/*     if (!src) */
/*         return NULL; */

/*     struct graph_t *dst = malloc(sizeof(struct graph_t)); */
/*     if (!dst) */
/*         return NULL; */

/*     dst->type = src->type; */
/*     dst->num_vertices = src->num_vertices; */
/*     dst->num_edges = src->num_edges; */

/*     dst->t = gsl_spmatrix_uint_alloc_copy_fallback(src->t); */
/*     if (!dst->t) { */
/*         free(dst); */
/*         return NULL; */
/*     } */

/*     memcpy(dst->start, src->start, sizeof(vertex_t) * NUM_PLAYERS); */

/*     dst->num_objectives = src->num_objectives; */
/*     dst->objectives = malloc(dst->num_objectives * sizeof(vertex_t)); */
/*     if (!dst->objectives) { */
/*         gsl_spmatrix_uint_free(dst->t); */
/*         free(dst); */
/*         return NULL; */
/*     } */
/*     memcpy(dst->objectives, src->objectives, dst->num_objectives * sizeof(vertex_t)); */

/*     return dst; */
/* } */
struct graph_t *graph_copy(const struct graph_t *g) {
    struct graph_t *copy = malloc(sizeof(struct graph_t));
    if (!copy)
        return NULL;

    copy->type = g->type;
    copy->num_vertices = g->num_vertices;
    copy->num_edges = g->num_edges;

    // Copie des objectifs
    copy->num_objectives = g->num_objectives;
    copy->objectives = malloc(sizeof(vertex_t) * g->num_objectives);
    memcpy(copy->objectives, g->objectives, sizeof(vertex_t) * g->num_objectives);

    // Copie des positions de départ
    memcpy(copy->start, g->start, sizeof(vertex_t) * NUM_PLAYERS);

    // Copie manuelle de la matrice d’adjacence
    gsl_spmatrix_uint *tmp = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
    for (size_t row = 0; row < g->num_vertices; ++row) {
        for (int k = g->t->p[row]; k < g->t->p[row + 1]; ++k) {
            vertex_t col = g->t->i[k];
            unsigned int val = g->t->data[k];
            gsl_spmatrix_uint_set(tmp, row, col, val);
        }
    }

    // Compression en CSR
    copy->t = gsl_spmatrix_uint_compress(tmp, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(tmp);

    return copy;
}

void graph_initialize_objectives(const struct graph_t *src) {
    srand(time(NULL));
    unsigned int num_initialized_objectives = 0;
    for (unsigned int i = 0; i < src->num_vertices / 2; i++) {
        if (i != src->start[0]) {
            if (num_initialized_objectives < (src->num_objectives / 2)) {
                src->objectives[num_initialized_objectives++] = rand() % (src->num_vertices / 2);
            } else {
                break;
            }
        }
    }
    for (unsigned int i = 0; i < src->num_objectives / 2; i++) {
        src->objectives[i + src->num_objectives / 2] = src->num_vertices - (src->objectives[i] + 1);
    }
}

void graph_remove_edge(struct graph_t *g, vertex_t i, vertex_t j) {
    if (i >= g->num_vertices || j >= g->num_vertices || !graph_has_edge(g, i, j))
        return;
    for (int k = g->t->p[i]; k < g->t->p[i + 1]; ++k) {
        if ((vertex_t)g->t->i[k] == j) {
            g->t->data[k] = WALL_DIR;
            break;
        }
    }

    for (int k = g->t->p[j]; k < g->t->p[j + 1]; ++k) {
        if ((vertex_t)g->t->i[k] == i) {
            g->t->data[k] = WALL_DIR;
            break;
        }
    }
}
