#ifndef LOAD_PLAYER
#define LOAD_PLAYER
#include "graph.h"
#include "player.h"

struct player_t {
    void *handle;
    void (*initialize)(unsigned int, struct graph_t *);
    struct move_t (*play)(const struct move_t previous_move);
    void (*finalize)();
};

struct player_t load_player(char *filename);
void close_player(struct player_t *p);

#endif
