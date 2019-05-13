#include "libraries.h"

typedef struct player {
    char ip[20];
    char port[6];
    card play1, play2;
} player;

int argumentControl(int argc, char const *argv[]);