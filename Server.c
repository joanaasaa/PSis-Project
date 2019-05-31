#include "server_library.h"

int main(int argc, char const *argv[])
{
    int n; // Aid variable.
	int dim_board; // Board dimension (in cards).
	pthread_t listenSocketID, stdinSocketID, endGameID;

	dim_board = argumentControl(argc, argv);

	// Graphics' initialization
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}
	create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board, 1);

	// Board content initialization.
	init_board(dim_board);

	// Read/write lock for players' list's initialization.
	init_lock();

	// Thread initialization.
	pthread_create(&listenSocketID, NULL, listenSocket_thread, NULL); // Server can listen for new players.
	pthread_create(&stdinSocketID, NULL, stdinSocket_thread, NULL); // Server waits for commands from keyboard.

	// Server application can be terminated either by the "exit" command from stdin or if terminate = 1.
	while(1) {
		if(get_terminate() == 1)
			break;

		else if(get_terminate() == 2) {
			pthread_create(&endGameID, NULL, endGame_thread, NULL);
			pthread_join(endGameID, NULL);
		}
	}

	pthread_join(stdinSocketID, NULL);
	pthread_join(listenSocketID, NULL);
	pthread_join(endGameID, NULL);

	close_board_windows(); // Ends graphics for server.

	destroy_lock();

    return 0;
}