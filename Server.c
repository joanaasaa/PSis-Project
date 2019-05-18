#include "libraries.h"
#include "server_library.h"
#include "board_library.h"
#include "graphics_library.h"

int main(int argc, char const *argv[])
{
    int n; // Aid variable. 
	int dim_board; // Board dimension (in cards).
	int fd_stdin = 0; // Keyboard's file descriptor.
	char str[20]; // String for commands from keyboard.
	struct sockaddr_in server_addr, client_addr;
	unsigned int client_addrlen;
	pthread_t listenSocketID;
	player *players_aux; 

    dim_board = argumentControl(argc, argv);
	init_board(dim_board);

	pthread_create(&listenSocketID, NULL, listenSocket_thread, NULL); // Prepares server to listen for new players.
	//pthread_join(listenSocketID, NULL);

	// Server application can be terminated at any time with the command "exit".
	while(1) {	
		memset(str, 0, sizeof(str));
		read(fd_stdin, &str, sizeof(str));
		if(strcmp(str, "exit\n") == 0) {
			
			// TAKE NECESSARY ACTIONS TO TERMINATE APPLICATION...

		}
		else printf("Unsupported order!\n");
	}

    return 0;
}