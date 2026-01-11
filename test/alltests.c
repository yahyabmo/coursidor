
#include <stdio.h>

// Déclarations des fonctions de test
void test_add_edge();
void test_remove_edge();
void test_compute_valid_moves_on_triangle_graph();
extern void test_exists_path(void);

extern void test_neighbors_vertex(void);
int main() {
    printf("=== Running all tests ===\n\n");

    test_add_edge();
    test_remove_edge();
    test_compute_valid_moves_on_triangle_graph();
    test_exists_path();
    test_neighbors_vertex();
    printf("\n All tests passed successfully.\n");
    return 0;
}
