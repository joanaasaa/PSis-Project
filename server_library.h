#include "libraries.h"

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

int check_terminate();

int argumentControl(int argc, char const *argv[]);

void addPlayer(int newfd);

void removePlayer(player *toRemove);

void interpret_final_msg(char final_msg[], player *me);

void *stdinSocket_thread(void *arg);

void *checkTimer_thread(void *arg);

void *listenSocket_thread(void *arg);

void *player_thread(void *arg);
