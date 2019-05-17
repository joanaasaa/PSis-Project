#include "libraries.h"

typedef struct player {
    pthread_t threadID;
    int socket;
    int play1[2], play2[2];
    int rgb_R;
    int rgb_G;
    int rgb_B;
    struct player *next;
} player;

void addPlayer(int newfd);

void *player_thread(void *arg);