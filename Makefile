GSL_PATH ?= /usr/local

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g3 -fPIC -I$(GSL_PATH)/include -I./src -fprofile-arcs -ftest-coverage
LDFLAGS = -L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 -Wl,--rpath=$(GSL_PATH)/lib
LIBS = -lgsl -lgslcblas -lm -ldl

SRC_DIR = src
BUILD_DIR = build
INSTALL_DIR = install

# Fichiers source du serveur (ajout de board.c ici)
SERVER_SRC = $(SRC_DIR)/server.c $(SRC_DIR)/load_player.c $(SRC_DIR)/graph.c $(SRC_DIR)/commun.c $(SRC_DIR)/display.c $(SRC_DIR)/board.c $(SRC_DIR)/helper.c 

SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Joueurs + les dépendances communes
PLAYER1_SRC = $(SRC_DIR)/player1.c
PLAYER2_SRC = $(SRC_DIR)/player2.c

COMMON_SRC = $(SRC_DIR)/commun.c $(SRC_DIR)/graph.c $(SRC_DIR)/helper.c

PLAYER1_OBJ = $(PLAYER1_SRC:.c=.o) $(COMMON_SRC:.c=.o)
PLAYER2_OBJ = $(PLAYER2_SRC:.c=.o) $(COMMON_SRC:.c=.o)


PLAYER1_SO = $(INSTALL_DIR)/player1.so
PLAYER2_SO = $(INSTALL_DIR)/player2.so

SERVER_BIN = $(BUILD_DIR)/server

TEST_SRC = \
    test/alltests.c \
    test/test_add_edge.c \
    test/test_remove_edge.c \
    test/test_compute_valid_moves.c \
    test/test_neighbors_vertex.c \
    test/test_exists_path.c \
    $(SRC_DIR)/graph.c \
    $(SRC_DIR)/commun.c \
    $(SRC_DIR)/helper.c

TEST_BIN = $(INSTALL_DIR)/alltests

all: build $(SERVER_BIN) $(PLAYER1_SO) $(PLAYER2_SO) $(TEST_BIN)

build:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INSTALL_DIR)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) --coverage

$(PLAYER1_SO): $(PLAYER1_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS) $(LIBS) --coverage

$(PLAYER2_SO): $(PLAYER2_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS) $(LIBS) --coverage


$(TEST_BIN): $(TEST_SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

install: all
	cp $(SERVER_BIN) $(INSTALL_DIR)/server

clean:
	rm -rf $(BUILD_DIR) $(INSTALL_DIR) *.so src/*.o src/*.gcno src/*.gcda *~ src/*~ test/*~


test: $(TEST_BIN)
	./$(TEST_BIN)

.PHONY: all build install clean
