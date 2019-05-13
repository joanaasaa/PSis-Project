#include "libraries.h"

typedef struct player {
    pthread_t threadID;
    int socket;
    char ip[20];
    char port[6];
    int play1[2], play2[2];
    int rgb_R;
    int rgb_G;
    int rgb_B;
} player;

void *listenSocket(void *arg);