#include "libraries.h"
#include "player_management.h"
#include "board_library.h"
#include "UI_library.h"

int dim_board;

void argumentControl(int argc, char const *argv[]) {
    
    if(argc != 2) {
        printf("Unsupported number of arguments!\n");
        exit(-1);
    }

    dim_board = atoi(argv[1]);

	if((!(dim_board & 1)) == 0) // Se o número é ímpar.
	{
		printf("You have to choose an even number!\n");
		exit(-1);
	}

	if(dim_board > 26)
	{
		printf("Board size can't be greater than 26!\n");
		exit(-1);
	}
    printf("Board size: %dx%d\n", dim_board, dim_board);

	return;
}

int main(int argc, char const *argv[])
{
	int done = 0;
    int n;
	int fd;
	char str[20];
	struct sockaddr_in server_addr, client_addr;
	unsigned int client_addrlen;
	pthread_t listenSocketID;
	SDL_Event event;
	player *players_aux; 

    argumentControl(argc, argv);
	init_board(4);

    // ------- BOARD INITIALIZATION -------
/*    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	create_board_window(300, 300, dim_board); // Cria a parte gráfica do tabuleiro (SDL).

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
	}*/


	// ------- SOCKET CREATION -------

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	fd = socket(AF_INET, SOCK_STREAM, 0); 	// AF_INET: Socket for communication between diffrent machines;
											// SOCK_STREAM: Stream socket. Connection oriented.
	if(fd == -1) {
		perror("socket: ");
		exit(-1);
  	}
  	 
	n = bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n == -1) {
		perror("bind: ");
		exit(-1);
 	}
  	printf("Socket created and binded!\n");
  	listen(fd, 5);

	// Server is listening for clients.
	while(1)
	{
		int newfd = accept(fd, (struct sockaddr *) &client_addr, &client_addrlen);
		addPlayer(newfd);
	}

	close(fd);

    // // ------- START GRAPHICS -------
	// if(SDL_Init(SDL_INIT_VIDEO) < 0) {
	// 	printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	// 	exit(-1);
	// }
	// if(TTF_Init() == -1) {
	// 	printf("TTF_Init: %s\n", TTF_GetError());
	// 	exit(-1);
	// }


	// create_board_window(1000, 1000,  dim_board); // Cria a parte gráfica do tabuleiro (SDL).
	
    // init_board(dim_board); // Cria o conteúdo do tabuleiro (as strings para as cartas). Função apenas lógica (não lida com a biblioteca gráfica).

    // while(!done) {
	// 	while(SDL_PollEvent(&event)) {
	// 		switch(event.type) {
	// 			case(SDL_QUIT): {
	// 				done = SDL_TRUE;
	// 				break;
	// 			}
	// 			case(SDL_MOUSEBUTTONDOWN): {
	// 				printf("Click!\n");
	// 			}
	// 		}
	// 	}
	// }

	//close_board_windows();

    return 0;
}