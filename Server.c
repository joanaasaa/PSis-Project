#include "libraries.h"
#include "server_library.h"
#include "board_library.h"
#include "graphics_library.h"

int main(int argc, char const *argv[])
{
    int n; // Aid variable. 
	int dim_board; // Board dimension (in cards).
	pthread_t listenSocketID, stdinSocketID, checkTimerID;
	SDL_Event event; // GRAPHICS

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
	create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board); // GRAPHICS
	init_board(dim_board);

	init_lock();
	init_mutex();

	pthread_create(&listenSocketID, NULL, listenSocket_thread, NULL); // Prepares server to listen for new players.
	pthread_create(&stdinSocketID, NULL, stdinSocket_thread, NULL); // Prepares server to wait for commands from keyboard.
	pthread_create(&checkTimerID, NULL, checkTimer_thread, NULL);

	// Server application can be terminated either by the "exit" command from stdin or when final instant is reached.
	while(1) {
		if(check_terminate() == 1) break;
	}

	pthread_join(checkTimerID, NULL);
	pthread_join(stdinSocketID, NULL);
	pthread_join(listenSocketID, NULL);

	close_board_windows(); // Ends graphics for server.

	destroy_lock();
	destroy_mutex();

    return 0;
}
