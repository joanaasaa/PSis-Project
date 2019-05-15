#include "libraries.h"

typedef struct player_self {
    int play1[2], play2[2];
    int rgb_R;
    int rgb_G;
    int rgb_B;
} player_self;

void write_card(int  board_x, int board_y, char * text, int r, int g, int b);
void paint_card(int  board_x, int board_y , int r, int g, int b);
void clear_card(int  board_x, int board_y);
void get_board_card(int mouse_x, int mouse_y, int * board_x, int *board_y);
void create_board_window(int width, int height,  int dim);
void close_board_windows();