#include "server_library.h"
#include "board_library.h"
#include "graphics_library.h"

int game = 0; // Registers if the game has started (1), or not (0).
int dim_board;
int found_pairs = 0; 
int nr_players = 0;
int terminate = 0;
player *players_head = NULL; // List of in-game players.

int check_terminate()
{
	return terminate;
}

int argumentControl(int argc, char const *argv[]) {
    
    if(argc != 2) {
        printf("Unsupported number of arguments!\n");
        exit(-1);
    }

    dim_board = atoi(argv[1]);

	if((!(dim_board & 1)) == 0) // If the number is uneven.
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
		perror("malloc");
		pthread_exit(NULL);
	}
    
  	int flags = fcntl(newfd, F_GETFL, 0);
  	fcntl(newfd, F_SETFL, flags | O_NONBLOCK);

	players_aux->socket = newfd;
	players_aux->rgb_R = rand() % 255 + 1;
	players_aux->rgb_G = rand() % 255 + 1;
	players_aux->rgb_B = rand() % 127 + 128;
	players_aux->score = 0; // Number of pairs.
	
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

void removePlayer(player *toRemove) 
{
	player *players_aux, *players_prev;
			
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

void interpret_final_msg(char final_msg[], player *me) 
{
	int code;
	char card1[3], card2[3];
	char str[100];

	if(sscanf(str, "%d%*s", &code) == 1) {
		printf("Received message with code %d\n", code);

		if(game == 1) {
			
			if(code == -1) {
				if(sscanf(str, "%*d-%d-%d\n", &(me->card1_x), &(me->card1_x)) == 2) {
					
					// VALIDATE CARD!!!
					
					// Gets card's string.
					memset(card1, 0, sizeof(card1));
					card1 = get_card_str(me->card1_x, me->card1_y);
					
					// Server updates its own graphics.
					paint_card(me->card1_x, me->card1_y , me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
					write_card(me->card1_x, me->card1_y, card1, 200, 200, 200); // Paints the letters on the card grey.

					// Updates the matched cards' status in the board.
					set_card_traits(me->card1_x, me->card1_y, 'u', me->rgb_R, me->rgb_G, me->rgb_B);

					// Gives the player feedback.
					memset(str, 0, sizeof(str));
					code = 5;
					sprintf(str, "%d-%c%c\n", code, card1[0], card1[1]);
					write(me->socket, str, strlen(str));

					// Updates the other players.
					memset(str, 0, sizeof(str));
					code = 1;
					sprintf(str, "%d-%c%c-%d-%d-%d-%d-%d\n", code, card1[0], card1[1], me->card1_x, me->card1_y, me->rgb_R, me->rgb_G, me->rgb_B);
					for(player *aux = players_head; aux != NULL; aux=aux->next) {
						if(aux == me) continue;
						else write(aux->socket, str, strlen(str));
					}
				}
				else {
					printf("Bad message from server!");
				}
			}

			else if(code == -2) {
				if(sscanf(str, "%*d-%d-%d\n", &(me->card2_x), &(me->card2_x)) == 2) {
					
					// VALIDATE CARD !!!
					
					// Gets card's string.
					card1 = get_card_str(me->card1_x, me->card1_y);
					card2 = get_card_str(me->card2_x, me->card2_y);

					if(strcmp(card1, card2) == 0) {
						printf("Cards match!\n");
						found_pairs++;

						// Server updates its own graphics.
						paint_card(me->card1_x, me->card1_y, me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
						write_card(me->card1_x, me->card1_y, card1, 0, 0, 0); // Paints the letters on the card grey.
						paint_card(me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
						write_card(me->card2_x, me->card2_y, card2, 0, 0, 0); // Paints the letters on the card grey.
						
						// Updates the matched cards' status in the board.
						set_card_traits(me->card1_x, me->card1_y, 'l', me->rgb_R, me->rgb_G, me->rgb_B);
						set_card_traits(me->card2_x, me->card2_y, 'l', me->rgb_R, me->rgb_G, me->rgb_B);
						
						memset(str, 0, sizeof(str));

						if(found_pairs != dim_board) { // If all pairs haven't been found yet.
							// Gives the player feedback.
							memset(str, 0, sizeof(str));
							code = 6;
							sprintf(str, "%d\n", code, card1[0], card1[1]);
							write(me->socket, str, strlen(str));

							// Updates the other players.
							memset(str, 0, sizeof(str));
							code = 2;
							sprintf(str, "%d-%c%c-%d-%d-%d-%d-%d-%d-%d\n", code, card1, card2, me->card1_x, me->card1_y, me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B);
							for(player *aux = players_head; aux != NULL; aux=aux->next) {
								if(aux == me) continue;
								else write(aux->socket, str, strlen(str));
							}
						}
						else { // If all pairs have been found.
							// Gives the player feedback.
							memset(str, 0, sizeof(str));
							code = 7;
							sprintf(str, "%d\n", code, card1[0], card1[1]);
							write(me->socket, str, strlen(str));

							// Updates the other players.
							memset(str, 0, sizeof(str));
							code = 3;
							sprintf(str, "%d-%c%c-%d-%d-%d-%d-%d-%d-%d\n", code, card1, card2, me->card1_x, me->card1_y, me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B);
							for(player *aux = players_head; aux != NULL; aux=aux->next) {
								if(aux == me) continue;
								else write(aux->socket, str, strlen(str));
							}

							// DO LOTS OF OTHER STUFF!
						}

					}
					else {
						printf("Cards don't match.\n");


					}
					
					
				}
				else {
					printf("Bad message from server!");
				}
			}

			else if(code == -3) {

			}

			else {
				printf("Impossible code number!");
			}

		}
		else
			printf("Game hasn't started yet. The server isn't supposed to be receiving messages from players!\n");

	} // End of if(sscanf(str, "%d%*s", &code) == 1).
	else
		printf("Couldn't read message!\n");

	return;
}

void *stdinSocket_thread(void *arg)
{
	int n;
	int fd_stdin = 0; // Keyboard's file descriptor.
	char str[20]; // String for commands from keyboard.

	int flags = fcntl(fd_stdin, F_GETFL, 0);
	fcntl(fd_stdin, F_SETFL, flags | O_NONBLOCK);

	while(!terminate) {
		memset(str, 0, sizeof(str));

		n = read(fd_stdin, &str, sizeof(str));
		if(n<=0){
			if( (errno == EAGAIN || errno == EWOULDBLOCK) && (n==-1) ) {//  If there was an error because there wasn't data to be read ...
				continue; // ... there wasn't actually an error. Loop continues.
			}
		}
		else {
			if(strcmp(str, "exit\n") == 0) {
				terminate = 1;
				pthread_exit(0);
			}
			else printf("Unsupported order!\n");
		}
		
	}

	pthread_exit(NULL);
}

void *checkTimer_thread(void *arg) // AND GRAPHICS
{
	int server_duration = 1200; // The server will be running for 20 minutes.
	time_t start_time, aux_time; 

	start_time = time(NULL);

	while(!terminate){
		aux_time = time(NULL);
		if((aux_time - start_time) >= server_duration) {
			terminate = 1;
			pthread_exit(0);
		}	
	}
	
	pthread_exit(0);

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
	if(fd<0) {
		perror("socket");
		exit(-1);
  	}
	
	int flags = fcntl(fd, F_GETFL, 0);
  	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  	 
	n = bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n<0) {
		perror("bind");
		exit(-1);
 	}

  	printf("Socket created and binded!\n");
  	listen(fd, 5);

	while(!terminate) {	
		int newfd = accept(fd, (struct sockaddr *) &client_addr, &client_addrlen);
		if(newfd <= 0) { // If there was an error accepting the new player.
			if(errno == EAGAIN || errno == EWOULDBLOCK) // If the error happened because there was no connection to accept ...
				continue; // ... the loop continues.
			else { // If the error was of another type, the player isn't added to the list of players.
				perror("accept");
				printf("Unnable to accept player!\n");
			}
		}
		else addPlayer(newfd);
	}

	close(fd);

	while(1) { // Waits for all players to disconnect, before exiting.
		if(nr_players == 0) pthread_exit(0);
	}
}

void *player_thread(void *arg)
{
	int n; // Aid variable.
	int code; // To store message's codes.
	char card[3];
	char *res_aux, buffer[200], final_msg[100], res[100];
	char str[100];
	char *str2 = NULL;
	player *me = (player*) arg;

	memset(card, 0, sizeof(card));
	memset(buffer, 0, sizeof(buffer));
	memset(final_msg, 0, sizeof(buffer));
	memset(res, 0, sizeof(buffer));

	// Sends initial message to player.
	memset(str, 0, sizeof(str));
	code = 9;
	sprintf(str, "%d-%d-%d-%d-%d-%d\n", code, dim_board, game, me->rgb_R, me->rgb_G, me->rgb_B);
	write(me->socket, str, strlen(str));

	// Server sends the state of the board to new players.
	if(nr_players > 2) { // This is only necessary if there are more than 2 players connected. Before that, changes to the board aren't possible.
		for(int i=0; i<dim_board; i++){
			for(int j=0; j<dim_board; j++) {
				if(get_card_status(i, j) != 'd') {
					str2 = get_str2send(i, j);
					strcpy(str, str2);
					write(me->socket, str, strlen(str));
				}
			}
		}

		code = 16;
		memset(str, 0, sizeof(str));
		sprintf(str, "%d\n", code);
		write(me->socket, str, strlen(str));
	}

	if((nr_players==2) && (game==0)) { // If there are 2 players connected but the game hasn't yet started ...
		game = 1; // ... the game starts.
		
		code = 12;
		memset(str, 0, sizeof(str));
		sprintf(str, "%d\n", code);
		write(me->next->socket, str, strlen(str)); // Tells the player who first connected to start playing.
		write(me->socket, str, strlen(str)); // Tells the second player to start playing.
	}

	while(!terminate) { // Server waits for player's decesion.
		memset(str, 0, sizeof(str));
	
		n = read(me->socket, str, sizeof(str));
		if(n <= 0) { // If the player disconnected (purposefully or otherwise) or there was no data to be read.
			if( (errno == EAGAIN || errno == EWOULDBLOCK) && (n==-1) ) { //  If there was an error because there wasn't data to be read ...
				continue; // ... there wasn't actually an error. Loop continues.
			}
			else { // If the player disconnected (purposefully or otherwise) ...
				printf("A player disconnected.\n");
				removePlayer(me); // ... the player is removed from the list of connected players.
			
				if(nr_players <= 1) { // If there is one or zero players, the game ends.
					
					// DO STUFF!!!!

				}
				
				pthread_exit(NULL); // ... and the thread is terminated.
			}
			
		}
		else { // If there's data to be read

			printf("read %d: %s\n", n, str);
			
			strcat(buffer, str);

			res_aux = strstr(buffer, "\n"); // res_aux is pointing to "\n"'s first occurance in buffer.
			while(res_aux != NULL) { // While there's a message to be read stored in buffer.
				res_aux++;
				memset(res, 0, sizeof(res));
				strcpy(res, res_aux);

				memset(final_msg, 0, sizeof(final_msg));
				int i=0;
				for(i=0; buffer[i] != '\n'; i++) {
					final_msg[i] = buffer[i];
				}
				final_msg[i] = '\n';
				final_msg[i+1] = '\0';

				memset(buffer, 0, sizeof(buffer));
				strcpy(buffer, res);

				interpret_final_msg(final_msg, me);

				res_aux = strstr(buffer, "\n");
			}

		} // End of else for read's n (n>0).
	} // End of while.

	printf("A player terminated.\n");
	removePlayer(me);
	pthread_exit(0);
}
