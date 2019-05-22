#include "libraries.h"

typedef struct player {
    pthread_t threadID;
    int socket;
    int score; // Number of pairs in current game.
    int total_score; // Number of games that this player won.
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

void read_message(char str[], char final_msg[]);