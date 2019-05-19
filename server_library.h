#include "libraries.h"

typedef struct player {
    pthread_t threadID;
    int socket;
    int score;
    int play1[2], play2[2];
    int rgb_R;
    int rgb_G;
    int rgb_B;
    struct player *next;
} player;

int check_terminate();

int argumentControl(int argc, char const *argv[]);

void addPlayer(int newfd);

void removePlayer(player *toRemove);

void *stdinSocket_thread(void *arg);

void *checkTimer_thread(void *arg);

void *listenSocket_thread(void *arg);

void *player_thread(void *arg);

void read_message(char str[]);