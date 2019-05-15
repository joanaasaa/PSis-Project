#include "libraries.h"

// ------- STRUCTS -------
typedef struct card {
	char status; // Indica o status da carta: u - up; d - down; l - locked.
	char v[3];
} card; // Para guardar a informação de cada carta.

typedef struct play_response {
	int code; // Código que reprensenta a fase da jogada e do jogo.
		// 0 - Impossibilidade de escolher a carta seleccionada.
		// 1 - Foi escolhida a 1ª carta da jogada.
		// 2 - Foi feita a 2ª escolha, as cartas escolhidas são iguais e o jogo continua.
		// 3 - Foi feita a 2ª escolha, as cartas escolhidas são iguais e o jogo terminou.
		// -2 - Foi feita a 2ª escolha mas as cartas escolhidas são diferentes.
	int play1[2], play2[2]; // Índice da carta escolhida da 1ª e 2ª vez.
	char str_play1[3], str_play2[3]; // String correspondente às cartas escolhidas da 1ª e 2ª vez.
} play_response;

// ------- FUNCTIONS -------
int linear_conv(int i, int j); // Devolve um valor linear, consoante coordenadas dadas.

void init_board(int dim); // Para atribuir as strings aos lugares do tabuleiro.

play_response board_play (int x, int y);

int get_dim_board();

char *get_str_send(int i, int j, int r, int g, int b); // Devolve, segundo coordenadas dadas, a string de um dado lugar do tabuleiro.