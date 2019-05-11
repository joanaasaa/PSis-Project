#include "libraries.h"

// ------- ESTRUTURAS -------
typedef struct board_place{
	char v[3];
} board_place; // Para guardar a string de cada lugar no tabuleiro.

typedef struct play_response{
	int code; 	// 0 - filled
				// 1 - 1st play
				// 2 2nd - same plays
				// 3 END
				// -2 2nd - diffrent
	int play1[2];
	int play2[2];
	char str_play1[3], str_play2[3];
} play_response;

// ------- FUNÇÕES -------
int linear_conv(int i, int j); // Devolve um valor linear, consoante coordenadas dadas.

char *get_board_place_str(int i, int j); // Devolve, segundo coordenadas dadas, a string de um dado lugar do tabuleiro.

void init_board(int dim); // Para atribuir as strings aos lugares do tabuleiro.

play_response board_play (int x, int y);