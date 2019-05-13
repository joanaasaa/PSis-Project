#include "libraries.h"
#include "board_library.h"
#include "UI_library.h"

int main(int argc, char const *argv[])
{
    int done = 0;
	int board_x, board_y; // Para guardar o lugar no tabuleiro da carta escolhida.
	SDL_Event event;

    // ------- ARGUMENT CONTROL -------
    if(argc != 2) {
        printf("Unsupported number of arguments!\n");
        return -1;
    }
    int dim_board = atoi(argv[1]);
	if((!(dim_board & 1)) == 0) // Se o número é ímpar.
	{
		printf("You have to choose an even number!\n");
		return -1;
	}
	if(dim_board > 26)
	{
		printf("Board size can't be greater than 26!\n");
		return -1;
	}
    printf("Board size: %dx%d\n", dim_board, dim_board);

    // ------- START GRAPHICS -------
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		return -1;
	}

	create_board_window(1000, 1000,  dim_board); // Cria a parte gráfica do tabuleiro (SDL).
	
    init_board(dim_board); // Cria o conteúdo do tabuleiro (as strings para as cartas). Função apenas lógica (não lida com a biblioteca gráfica).

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
<<<<<<< HEAD
=======
	printf("fim\n");
	close_board_windows();
>>>>>>> abdb96f431636ae4d4f484b9b43f55028e871070




    return 0;
}