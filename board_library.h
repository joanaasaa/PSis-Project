#include "libraries.h"

// ------- STRUCTS -------
typedef struct card {
	char v[3];
	char status;
	int rgb_R;
    int rgb_G;
    int rgb_B;
} card; // Para guardar a informação de cada carta.

typedef struct play_response {
	int code; // Código que reprensenta a fase da jogada e do jogo.
	int play1[2], play2[2]; // Índice da carta escolhida da 1ª e 2ª vez.
	char str_play1[3], str_play2[3]; // String correspondente às cartas escolhidas da 1ª e 2ª vez.
} play_response;

// ------- FUNCTIONS -------
void init_mutex();

int linear_conv(int i, int j); // Devolve um valor linear, consoante coordenadas dadas.

void set_card_traits(int i, int j, int status, int r, int g, int b);

void set_pair_traits(int i1, int j1, int i2, int j2, int status, int r, int g, int b);

char *get_card_str(int i, int j);

char get_card_status(int i, int j);

void clear_board();

void init_board(int dim); // Para atribuir as strings aos lugares do tabuleiro.

play_response board_play (int x, int y);

char *get_str2send(int i, int j); // Devolve, segundo coordenadas dadas, a string de um dado lugar do tabuleiro.

void destroy_mutex();