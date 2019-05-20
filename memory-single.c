#include "libraries.h"
#include "board_library.h"
#include "UI_library.h"

int main() {
	int done = 0;
	int board_x, board_y; // Para guardar o lugar na matriz de cartas da carta escolhida...
	SDL_Event event;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	create_board_window(300, 300, 4); // Cria a parte gráfica do tabuleiro (SDL).
	init_board(4); // Cria o conteúdo do tabuleiro (as strings para as cartas). Função apenas lógica (não lida com a biblioteca gráfica).

	while(!done) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case(SDL_QUIT): {
					done = SDL_TRUE;
					break;
				}
				case(SDL_MOUSEBUTTONDOWN): {
					get_board_card(event.button.x, event.button.y, &board_x, &board_y); // Verifica a que carta correspondem as coordenadas.

					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y); // Imprime na consola as coordenadas do rato e a que carta correspondem.
					play_response resp = board_play(board_x, board_y); // Verifica a jogada.
					switch(resp.code) {
						case(1): // Foi a primeira escolha de uma jogada.
							paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100); // Pinta o fundo da carta de verde.
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200); // Pinta as letras de cinzento.
							break;
						case(3): // O jogo terminou.
							done = 1;
						case(2): // The play's 2nd choice was made. The cards matched but the game still goes on.
							paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100); // Pinta o fundo da carta de verde.
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0); // Pinta as letras a preto.
							paint_card(resp.play2[0], resp.play2[1] , 7, 200, 100); // Pinta o fundo da carta de verde.
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0); // Pinta as letras a preto.
							break;
						case(-2):
							paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100); // Pinta o fundo da carta de verde.
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0); // Pinta as letras a vermelho.
							paint_card(resp.play2[0], resp.play2[1] , 7, 200, 100); // Pinta o fundo da carta de verde.
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0); // Pinta as letras a vermelho.
							
							sleep(2); 

							// Pinta de novo as cartas de branco.
							paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255); 
							paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
							break;
					}
				}
			}
		}
	}
	printf("fim\n");
	close_board_windows();
}