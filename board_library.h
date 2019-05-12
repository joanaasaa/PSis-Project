#include "libraries.h"

// ------- ESTRUTURAS -------
typedef struct board_place{
	char v[3];
} board_place; // Para guardar a string de cada lugar no tabuleiro.

typedef struct play_response{
	int code; 	// 0 - filled (impossibilidade de escolher a carta seleccionada).
				// 1 - 1st play
				// 2 2nd - same plays
				// 3 - END (o jogo termina).
				// -2 2nd - diffrent
	int play1[2], play2[2]; // Índice da carta escolhida da 1ª e 2ª vez.
	char str_play1[3], str_play2[3]; // String correspondente às cartas escolhidas da 1ª e 2ª vez.
} play_response;

// ------- FUNÇÕES -------
int linear_conv(int i, int j); // Devolve um valor linear, consoante coordenadas dadas.

char *get_board_place_str(int i, int j); // Devolve, segundo coordenadas dadas, a string de um dado lugar do tabuleiro.

void init_board(int dim); // Para atribuir as strings aos lugares do tabuleiro.

play_response board_play (int x, int y);