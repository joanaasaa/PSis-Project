#include "libraries.h"

typedef struct player {
    char ip[20];
    char port[6];
    int play1[2], play2[2]; 
} player;

int argumentControl(int argc, char const *argv[]);

void TCPConnection();