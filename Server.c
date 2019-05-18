#include "libraries.h"
#include "server_library.h"
#include "board_library.h"
#include "graphics_library.h"

int main(int argc, char const *argv[])
{
    int n; // Aid variable. 
	int dim_board; // Board dimension (in cards).
	int server_duration = 1200;
	pthread_t listenSocketID, stdinSocketID;
	time_t start_time, aux_time; 

    dim_board = argumentControl(argc, argv);
	init_board(dim_board);

	pthread_create(&listenSocketID, NULL, listenSocket_thread, NULL); // Prepares server to listen for new players.
	pthread_create(&stdinSocketID, NULL, stdinSocket_thread, NULL); // Prepares server to wait for commands from keyboard.
	
	// Server application can be terminated either by the "exit" command from stdin or when final instant is reached.
	start_time = time(NULL);
	while(1) {	
		aux_time = time(NULL);

		if( (pthread_join(stdinSocketID, NULL) == 0) || (aux_time - start_time >= server_duration) ) {
			set_terminate();
			break;
		}
	}

	pthread_join(stdinSocketID, NULL);
	pthread_join(listenSocketID, NULL);

    return 0;
}