#include "server_library.h"
#include "board_library.h"

int dim_board;
int nr_players = 0;
player *players_head = NULL; // List of in-game players.

int argumentControl(int argc, char const *argv[]) {
    
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

	return dim_board;
}

void addPlayer(int newfd)
{
	player *players_aux; 
	
	nr_players++;

	players_aux = (player*) malloc(sizeof(player));
	if(players_aux == NULL) {
		perror("malloc: ");
		pthread_exit(NULL);
	}

	players_aux->socket = newfd;
	players_aux->rgb_R = rand() % 255 + 1;
	players_aux->rgb_G = rand() % 255 + 1;
	players_aux->rgb_B = rand() % 127 + 128;
	players_aux->score = 0;
	
	if(players_head == NULL) { // If the list is empty, the head is created.
		players_aux->next = NULL;
		players_head = players_aux;
	}
	else { // If the list isn't empty, the new player is inserted at the head of the list.
		players_aux->next = players_head;
		players_head = players_aux;
	}

	printf("A new player is now connected.\n");

	pthread_create(&(players_aux->threadID), NULL, player_thread, players_aux);

	return;
}

void removePlayer(player *toRemove) {
	
	player *players_aux, *players_prev;

	printf("A player disconnected.\n");
			
	close(toRemove->socket); // Closing player's socket.
	
	// Removing the player from the list.
	players_prev = NULL;
	for(players_aux = players_head; toRemove != players_aux; players_aux=players_aux->next) // Searches the list for the player.
		players_prev = players_aux;

	if(players_prev == NULL) { // If the player was the head of the list.
		players_head = players_head->next;
		free(players_aux);
	}
	else { // If the player wasn't at the head of the list of players.
		players_prev->next = players_aux->next;
		free(players_aux);
	}

	nr_players--;

	printf("There are now %d players in-game.\n", nr_players);

	return;
}

void *listenSocket_thread(void *arg)
{
	int n; // Aid variable. 
	int fd; // Listen socket's file descriptor.
	char str[20]; // String for messages.
	struct sockaddr_in server_addr, client_addr;
	unsigned int client_addrlen;
	player *players_aux; // Auxiliary pointer to deal with players' list handling.

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

	while(1) {	
		int newfd = accept(fd, (struct sockaddr *) &client_addr, &client_addrlen);
		addPlayer(newfd);
	}
}

void *player_thread(void *arg)
{
	int n; // Aid variable.
	int game = 0; // Registers if the game has started (1), or not (0).
	char str[25]; // String for messages.
	player *me = (player*) arg;

	sprintf(str, "%d-%d-%d-%d-%d\n", dim_board, game, me->rgb_R, me->rgb_G, me->rgb_B);
	printf("str: %s\n", str);

	write(me->socket, str, strlen(str));

	// Server sends the state of the board to new players.
	if(nr_players > 2) { // This is only necessary if there are more than 2 players connected. Before that, changes to the board aren't possible.
		for(int i=0; i<dim_board; i++){
			for(int j=0; j<dim_board; j++){

				memset(str, 0, sizeof(str));

				if(get_str2send(i, j, me->rgb_R, me->rgb_G, me->rgb_B) != NULL) { // If the card is visible to the player (up or locked).
					strcpy(str, get_str2send(i, j, me->rgb_R, me->rgb_G, me->rgb_B));
					printf("str: %s\n", str);
					
					write(me->socket, str, strlen(str));
				}
			}
		}
	}

	if((nr_players==2) && (game==0)) { // If there are 2 players connect but the game hasn't yet started ...
		game = 1; // ... the game starts.

		memset(str, 0, sizeof(str));
		strcpy(str, "start\n");
		write(me->next->socket, str, strlen(str)); // Tells the player who first connected to start playing.
		write(me->next->socket, str, strlen(str)); // Tells the second player to start playing.
	}

	while(1) {// Server waits for player's decesion.
		memset(str, 0, sizeof(str));
		
		n = read(me->socket, str, sizeof(str));
		if(n <= 0) { // If the player disconnected (purposefully or otherwise) ...
			removePlayer(me); // ... the player from the list of connected players ...
			if(nr_players <= 1) {

				// NOTIFY WINNER (THE LAST CONNECTED PLAYER, IF nr_players=1), TERMINATE GAME AND START ANOTHER ONE.

			}
			
			pthread_exit(NULL); // ... and the thread is terminated.
		}

		// INTERPRETS MESSAGE FROM PLAYER...

	}
}