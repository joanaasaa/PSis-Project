#include "libraries.h"
#include "Server_library.h"
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
	create_board_window(300, 300,  4); // Cria a parte gráfica do tabuleiro (SDL).
	
    init_board(4); // Cria o conteúdo do tabuleiro (as strings para as cartas). Função apenas lógica (não lida com a biblioteca gráfica).





    return 0;
}