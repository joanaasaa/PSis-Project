#include "libraries.h"
#include "board_library.h"
#include "graphics_library.h"

typedef struct player {
    pthread_t threadID;
    int socket;
    int score; // Number of pairs in current game.
    int rgb_R;
    int rgb_G;
    int rgb_B;
    int card1_x;
    int card1_y;
    int card2_x;
    int card2_y;
    struct player *next;
} player;

int get_terminate();

void set_terminate();

int argumentControl(int argc, char const *argv[]);

void init_lock();

void destroy_lock();

void addPlayer(int newfd);

void removePlayer(player *toRemove);

void *stdinSocket_thread(void *arg);

void *listenSocket_thread(void *arg);

void write2all(player *me, char str[]);

void interpret_final_msg(char final_msg[], player *me);

void *player_thread(void *arg);

void *endGame_thread(void *arg);