#include "server_library.h"

int main(int argc, char const *argv[])
{
    int n; // Aid variable.
	int dim_board; // Board dimension (in cards).
	time_t now, aux_endTime;
	pthread_t listenSocketID, stdinSocketID, endGameID;
	SDL_Event event; // GRAPHICS

	aux_endTime = time(NULL);

	// GRAPHICS
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	// START
	dim_board = argumentControl(argc, argv);
	create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board, 1); // GRAPHICS
	init_board(dim_board);

	init_lock();

	pthread_create(&listenSocketID, NULL, listenSocket_thread, NULL); // Prepares server to listen for new players.
	pthread_create(&stdinSocketID, NULL, stdinSocket_thread, NULL); // Prepares server to wait for commands from keyboard.

	// Server application can be terminated either by the "exit" command from stdin or when final instant is reached.
	while(1) {
		now = time(NULL);
		if(now - aux_endTime >= 1200) {// Server works for 20 minutes.
			set_terminate();
			break;
		}

		if(get_terminate() == 1)
			break;

		else if(get_terminate() == 2) {
			pthread_create(&endGameID, NULL, endGame_thread, NULL);
			pthread_join(endGameID, NULL);
		}
	}

	pthread_join(stdinSocketID, NULL);
	pthread_join(listenSocketID, NULL);

	close_board_windows(); // Ends graphics for server.

	destroy_lock();

    return 0;
}
