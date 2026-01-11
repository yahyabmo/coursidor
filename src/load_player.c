#include "load_player.h"
#include <dlfcn.h>

struct player_t load_player(char *filename) {
    struct player_t p;

    p.handle = dlopen(filename, RTLD_LAZY);
    if (!p.handle) {
        fputs(dlerror(), stderr);
        exit(1);
    }

    p.initialize = dlsym(p.handle, "initialize");
    if (!p.initialize) {
        fputs(dlerror(), stderr);
        exit(1);
    }
    p.play = dlsym(p.handle, "play");
    if (!p.play) {
        fputs(dlerror(), stderr);
        exit(1);
    }
    p.finalize = dlsym(p.handle, "finalize");
    if (!p.finalize) {
        fputs(dlerror(), stderr);
        exit(1);
    }
    printf("Player '%s' loaded successfully!\n", filename);
    return p;
}

void close_player(struct player_t *p) {
    dlclose(p->handle);
    p->handle = NULL;
    p->initialize = NULL;
}
