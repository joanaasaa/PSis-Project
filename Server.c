#include "libraries.h"
#include "board_library.h"
#include "UI_library.h"

// card **board;

// typedef struct card {
//     char status; // u - up; d - down; 
//     char string[3];
// } card;

int main(int argc, char const *argv[])
{
    int done = 0;
	int board_x, board_y; // Para guardar o lugar na matriz de cartas da carta escolhida.
	SDL_Event event;

    // int board_size = argumentControl(argc, argv);

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		return -1;
	}

	create_board_window(1000, 1000,  26); // Cria a parte gráfica do tabuleiro (SDL).
	init_board(26); // Cria o conteúdo do tabuleiro (as strings para as cartas). Função apenas lógica (não lida com a biblioteca gráfica).

	while(!done) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case(SDL_QUIT): {
					done = SDL_TRUE;
					break;
				}
				case(SDL_MOUSEBUTTONDOWN): {
					printf("Click!\n");
				}
			}
		}
	}




    return 0;
}