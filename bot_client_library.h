#include "libraries.h"

typedef struct bot_player_self {
    int rgb_R;
    int rgb_G;
    int rgb_B;
    int score;
} bot_player_self;

void argumentControl(int argc, char const *argv[]);

void create_socket(const char *server_ip);

void interpret_final_msg(char final_msg[]);

void *thread_read(void *arg);

void *thread_write(void *arg);