#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"
#include <stdbool.h>
#define MAX3(a, b, c) (((a) > (b)) ? (((a) > (c)) ? (a) : (c)) : (((b) > (c)) ? (b) : (c)))
typedef struct {
    int q;
    int r;
} coord_t;

#define GRID_W 70
#define GRID_H 25

const char *get_cell_symbol(vertex_t id, vertex_t p0, vertex_t p1, vertex_t *objectives,
                            size_t num_obj) {
    if (id == p0)
        return RED "B" RESET;
    if (id == p1)
        return BLUE "W" RESET;
    for (size_t i = 0; i < num_obj; ++i) {
        if (objectives[i] == id)
            return GREEN "*" RESET;
    }
    return ".";
}

/* void print_game_triangular(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1, */
/*                            vertex_t *objectives, size_t num_obj) { */
/*     const char *grid[GRID_H][GRID_W]; */

/*     for (int y = 0; y < GRID_H; y++) */
/*         for (int x = 0; x < GRID_W; x++) */
/*             grid[y][x] = " "; */

/*     coord_t *coords = malloc(sizeof(coord_t) * g->num_vertices); */
/*     if (!coords) { */
/*         fprintf(stderr, "Erreur d'allocation mémoire\n"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     int id = 0; */
/*     for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) { */
/*         for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) { */
/*             int s = -q - r; */
/*             int dist = MAX3(abs(q), abs(r), abs(s)); */
/*             if (dist < (int)m) { */
/*                 coords[id].q = q; */
/*                 coords[id].r = r; */
/*                 id++; */
/*             } */
/*         } */
/*     } */

/*     for (vertex_t id = 0; id < g->num_vertices; id++) { */
/*         int q = coords[id].q; */
/*         int r = coords[id].r; */

/*         int x = 6 * q + 3 * r + GRID_W / 2; */
/*         int y = 2 * r + GRID_H / 2; */

/*         if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H) */
/*             grid[y][x] = get_cell_symbol(id, p0, p1, objectives, num_obj); */

/*         struct pos_dir_t neighbors[NUM_DIRS]; */
/*         unsigned int n = graph_get_neighbors(g, id, neighbors); */
/*         for (unsigned int k = 0; k < n; k++) { */
/* 	  if (neighbors[k].dir == WALL_DIR) */
/*                  continue; // <- ignorer les murs (pas affichés du tout) */

/*             int q2 = coords[neighbors[k].pos].q; */
/*             int r2 = coords[neighbors[k].pos].r; */

/*             int x2 = 6 * q2 + 3 * r2 + GRID_W / 2; */
/*             int y2 = 2 * r2 + GRID_H / 2; */

/*             int xm = (x + x2) / 2; */
/*             int ym = (y + y2) / 2; */

/*             if (xm >= 0 && xm < GRID_W && ym >= 0 && ym < GRID_H) { */
/*                 if (x == x2) */
/*                     grid[ym][xm] = "|"; */
/*                 else if (y == y2) */
/*                     grid[ym][xm] = "─"; */
/*                 else if ((x < x2 && y < y2) || (x > x2 && y > y2)) */
/*                     grid[ym][xm] = "\\"; */
/*                 else */
/*                     grid[ym][xm] = "/"; */
/*             } */
/*         } */
/*     } */

/*     printf("\n\033[1mÉtat du jeu TRIANGULAR (m = %u) :\033[0m\n\n", m); */
/*     for (int y = 0; y < GRID_H; y++) { */
/*         bool has_content = false; */
/*         for (int x = 0; x < GRID_W; x++) { */
/*             if (grid[y][x][0] != ' ') { */
/*                 has_content = true; */
/*                 break; */
/*             } */
/*         } */
/*         if (has_content) { */
/*             for (int x = 0; x < GRID_W; x++) */
/*                 printf("%s", grid[y][x]); */
/*             printf("\n"); */
/*         } */
/*     } */

/*     free(coords); */
/* } */

void print_game_triangular(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1,
                           vertex_t *objectives, size_t num_obj) {
    const char *grid[GRID_H][GRID_W];

    // Initialiser la grille avec des espaces
    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++)
            grid[y][x] = " ";

    coord_t *coords = malloc(sizeof(coord_t) * g->num_vertices);
    if (!coords) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    // Générer les coordonnées axiales (q, r) pour chaque sommet
    int id = 0;
    for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) {
        for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) {
            int s = -q - r;
            int dist = MAX3(abs(q), abs(r), abs(s));
            if (dist < (int)m) {
                coords[id].q = q;
                coords[id].r = r;
                id++;
            }
        }
    }

    // Placer les sommets et les arêtes dans la grille
    for (vertex_t id = 0; id < g->num_vertices; id++) {
        int q = coords[id].q;
        int r = coords[id].r;

        // Calculer les coordonnées d'affichage
        int x = 6 * q + 3 * r + GRID_W / 2;
        int y = 2 * r + GRID_H / 2;

        // Vérifier les limites avant d'accéder à la grille

        if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H) {
            // Vérifie que la cellule n'est pas déjà occupée
            if (grid[y][x][0] == ' ')
                grid[y][x] = get_cell_symbol(id, p0, p1, objectives, num_obj);
        }

        // Traiter les arêtes
        struct pos_dir_t neighbors[NUM_DIRS];
        unsigned int n = graph_get_neighbors(g, id, neighbors);
        for (unsigned int k = 0; k < n; k++) {
            if (neighbors[k].dir == WALL_DIR)
                continue;

            vertex_t neighbor_pos = neighbors[k].pos;
            if (neighbor_pos >= g->num_vertices)
                continue; // Éviter les sommets invalides

            int q2 = coords[neighbor_pos].q;
            int r2 = coords[neighbor_pos].r;

            int x2 = 6 * q2 + 3 * r2 + GRID_W / 2;
            int y2 = 2 * r2 + GRID_H / 2;

            // Vérifier les limites du voisin
            if (x2 < 0 || x2 >= GRID_W || y2 < 0 || y2 >= GRID_H)
                continue;

            // Calculer le milieu
            int xm = (x + x2) / 2;
            int ym = (y + y2) / 2;

            // Vérifier les limites du milieu avant d'assigner
            if (xm >= 0 && xm < GRID_W && ym >= 0 && ym < GRID_H) {
                if (x == x2)
                    grid[ym][xm] = "|";
                else if (y == y2)
                    grid[ym][xm] = "─";
                else if ((x < x2 && y < y2) || (x > x2 && y > y2))
                    grid[ym][xm] = "\\";
                else
                    grid[ym][xm] = "/";
            }
        }
    }

    // Afficher la grille
    printf("\n\033[1mÉtat du jeu TRIANGULAR (m = %u) :\033[0m\n\n", m);
    for (int y = 0; y < GRID_H; y++) {
        bool has_content = false;
        for (int x = 0; x < GRID_W; x++) {
            if (grid[y][x][0] != ' ') {
                has_content = true;
                break;
            }
        }
        if (has_content) {
            for (int x = 0; x < GRID_W; x++)
                printf("%s", grid[y][x]);
            printf("\n");
        }
    }

    free(coords);
}
void print_game_cyclic(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1,
                       vertex_t *objectives, size_t num_obj) {
    const char *grid[GRID_H][GRID_W];
    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++)
            grid[y][x] = " ";

    coord_t *coords = malloc(sizeof(coord_t) * g->num_vertices);
    int id = 0;

    // Génération des coordonnées (q, r)
    for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) {
        for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) {
            int s = -q - r;
            int dist = MAX3(abs(q), abs(r), abs(s));
            if (dist >= (int)(m - 2) && dist < (int)m) {
                coords[id].q = q;
                coords[id].r = r;
                id++;
            }
        }
    }

    // Placement des sommets
    for (vertex_t id = 0; id < g->num_vertices; id++) {
        int q = coords[id].q;
        int r = coords[id].r;
        int x = 6 * q + 3 * r + GRID_W / 2;
        int y = 2 * r + GRID_H / 2;

        if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H)
            grid[y][x] = get_cell_symbol(id, p0, p1, objectives, num_obj);

        // Placement des arêtes
        struct pos_dir_t neighbors[NUM_DIRS];
        unsigned int n = graph_get_neighbors(g, id, neighbors);
        for (unsigned int k = 0; k < n; k++) {
            if (neighbors[k].dir == WALL_DIR)
                continue; // <- ignorer les murs (pas affichés du tout)

            int q2 = coords[neighbors[k].pos].q;
            int r2 = coords[neighbors[k].pos].r;
            int x2 = 6 * q2 + 3 * r2 + GRID_W / 2;
            int y2 = 2 * r2 + GRID_H / 2;

            int xm = (x + x2) / 2;
            int ym = (y + y2) / 2;

            if (xm >= 0 && xm < GRID_W && ym >= 0 && ym < GRID_H) {
                if (x == x2)
                    grid[ym][xm] = "|";
                else if (y == y2)
                    grid[ym][xm] = "─";
                else if ((x < x2 && y < y2) || (x > x2 && y > y2))
                    grid[ym][xm] = "\\";
                else
                    grid[ym][xm] = "/";
            }
        }
    }

    printf("\n\033[1mÉtat du jeu CYCLIC (m = %u) :\033[0m\n\n", m);
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++)
            fputs(grid[y][x], stdout);
        putchar('\n');
    }

    free(coords);
}

void print_game_holed(struct graph_t *g, unsigned int m, vertex_t p0, vertex_t p1,
                      vertex_t *objectives, size_t num_obj) {
    const char *grid[GRID_H][GRID_W];

    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++)
            grid[y][x] = " ";

    coord_t *coords = malloc(sizeof(coord_t) * g->num_vertices);
    if (!coords) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    int hole_size = m / 3 - 1;

    // mêmes centres que dans graph_initialize_holed
    int centers[7][2] = {{-2 * hole_size + 2 * (hole_size), -(3 * (hole_size) - (hole_size - 1))},
                         {2 * hole_size + 1, -(3 * (hole_size) - (hole_size - 1))},
                         {-2 * hole_size - 1, 0},
                         {0, 0},
                         {2 * hole_size + 1, 0},
                         {-2 * hole_size - 1, (3 * (hole_size) - (hole_size - 1))},
                         {2 * hole_size - 2 * (hole_size), (3 * (hole_size) - (hole_size - 1))}};

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

    int id = 0;
    for (int q = -(int)(m - 1); q <= (int)(m - 1); q++) {
        for (int r = -(int)(m - 1); r <= (int)(m - 1); r++) {
            int s = -q - r;
            int dist = MAX3(abs(q), abs(r), abs(s));
            if (dist < (int)m && !is_in_cyclic_hole(q, r)) {
                coords[id].q = q;
                coords[id].r = r;
                id++;
            }
        }
    }

    for (vertex_t id = 0; id < g->num_vertices; id++) {
        int q = coords[id].q;
        int r = coords[id].r;

        int x = 6 * q + 3 * r + GRID_W / 2;
        int y = 2 * r + GRID_H / 2;

        if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H)
            grid[y][x] = get_cell_symbol(id, p0, p1, objectives, num_obj);

        struct pos_dir_t neighbors[NUM_DIRS];
        unsigned int n = graph_get_neighbors(g, id, neighbors);
        for (unsigned int k = 0; k < n; k++) {
            if (neighbors[k].dir == WALL_DIR)
                continue;

            int q2 = coords[neighbors[k].pos].q;
            int r2 = coords[neighbors[k].pos].r;

            int x2 = 6 * q2 + 3 * r2 + GRID_W / 2;
            int y2 = 2 * r2 + GRID_H / 2;

            int xm = (x + x2) / 2;
            int ym = (y + y2) / 2;

            if (xm >= 0 && xm < GRID_W && ym >= 0 && ym < GRID_H) {
                if (x == x2)
                    grid[ym][xm] = "|";
                else if (y == y2)
                    grid[ym][xm] = "─";
                else if ((x < x2 && y < y2) || (x > x2 && y > y2))
                    grid[ym][xm] = "\\";
                else
                    grid[ym][xm] = "/";
            }
        }
    }

    printf("\n\033[1mÉtat du jeu HOLEY (m = %u) :\033[0m\n\n", m);
    for (int y = 0; y < GRID_H; y++) {
        bool has_content = false;
        for (int x = 0; x < GRID_W; x++) {
            if (grid[y][x][0] != ' ') {
                has_content = true;
                break;
            }
        }
        if (has_content) {
            for (int x = 0; x < GRID_W; x++)
                printf("%s", grid[y][x]);
            printf("\n");
        }
    }

    free(coords);
}
