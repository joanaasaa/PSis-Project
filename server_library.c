#include "server_library.h"

int game = 0; // Registers if the game has started (1), or not (0).
int dim_board;
int found_pairs = 0; 
int nr_players = 0;
int terminate = 0;
player *players_head = NULL; // List of in-game players.
pthread_rwlock_t lock_players;

int get_terminate() {
	return terminate;
}

void set_terminate() {
	terminate = 1;
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

void init_lock() 
{
	int n;
	
	n = pthread_rwlock_init(&lock_players, NULL);
	if(n != 0) { 
        printf("Lock initialization failed!\n"); 
        exit(-1); 
    }
}

void destroy_lock() {
	pthread_rwlock_destroy(&lock_players);
}

void addPlayer(int newfd)
{
	player *players_aux; 

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
	players_aux->count_2seconds = 0;
	players_aux->count_5seconds = 0;

	pthread_rwlock_rdlock(&lock_players);
	
	if(players_head == NULL) { // If the list is empty, the head is created.
		players_aux->next = NULL;
		players_head = players_aux;
	}
	else { // If the list isn't empty, the new player is inserted at the head of the list.
		players_aux->next = players_head;
		players_head = players_aux;
	}

	pthread_rwlock_unlock(&lock_players);

	nr_players++;

	printf("A new player is now connected.\n\n");

	pthread_create(&(players_aux->threadID), NULL, player_thread, players_aux);
}

void removePlayer(player *toRemove) 
{
	int code;
	char str[50];
	player *players_aux, *players_prev;
			
	close(toRemove->socket); // Closing player's socket.

	if(toRemove->count_2seconds == 1) {
		memset(str, 0, sizeof(str));
		code = 14;
		sprintf(str, "%d-%d-%d-%d-%d\n", code, toRemove->card1_x, toRemove->card1_y, toRemove->card2_x, toRemove->card2_y);
		write2all(toRemove, str);
	}
	
	if(toRemove->count_5seconds == 1) {
		// Notifies the oterh players.
		memset(str, 0, sizeof(str));
		code = 13;
		sprintf(str, "%d-%d-%d\n", code, toRemove->card1_x, toRemove->card1_y);
		write2all(toRemove, str);
	}
	
	pthread_rwlock_wrlock(&lock_players); 

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

	pthread_rwlock_unlock(&lock_players);

	nr_players--;

	printf("There are now %d players in-game.\n\n", nr_players);

	return;
}

void *stdinSocket_thread(void *arg)
{
	int n;
	int fd_stdin = 0; // Keyboard's file descriptor.
	char str[20]; // String for commands from keyboard.

	int flags = fcntl(fd_stdin, F_GETFL, 0);
	fcntl(fd_stdin, F_SETFL, flags | O_NONBLOCK);

	while(terminate != 1) {
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
				pthread_exit(NULL);
			}
			else printf("Unsupported order!\n\n");
		}
		
	}

	pthread_exit(NULL);
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

  	printf("Socket created and binded!\n\n");
  	listen(fd, 5);

	while(terminate != 1) {	
		
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
		if(nr_players == 0) pthread_exit(NULL);
	}
}

void write2all(player *me, char str[])
{
	player *aux = NULL;
	
	pthread_rwlock_rdlock(&lock_players);

	for(player *aux = players_head; aux != NULL; aux=aux->next) {
		if(aux == me) continue;
		else write(aux->socket, str, strlen(str));
	}

	pthread_rwlock_unlock(&lock_players);

	printf("Wrote to all players: %s\n", str);
}

void interpret_final_msg(char final_msg[], player *me) 
{
	int code, aux;
	char card1[3], card2[3];
	char str[100];

	strcpy(str, final_msg);
	printf("Read: %s\n", str);
	
	if(sscanf(str, "-%d%*s", &code) == 1) {
		printf("Received message with code -%d\n", code);

		if(game == 1) { // The server can only receive messages when the game is active.
			
			if(code == 1) {
				if(sscanf(str, "-%*d-%d-%d\n", &(me->card1_x), &(me->card1_y)) == 2) {
					
					printf("x: %d	y: %d\n", me->card1_x, me->card1_y);
					printf("Card: %s	Status: %c\n\n", get_card_str(me->card1_x, me->card1_y), get_card_status(me->card1_x, me->card1_y));

					if(get_card_status(me->card1_x, me->card1_y) != 'd' || me->count_2seconds == 1) {
						memset(str, 0, sizeof(str));
						code = 0;
						sprintf(str, "%d\n", code);
						write(me->socket, str, strlen(str));
						printf("Wrote: %s\n", str);
						return;
					}

					// Updates the matched cards' status in the board.
					set_card_traits(me->card1_x, me->card1_y, 'u', me->rgb_R, me->rgb_G, me->rgb_B);
					
					// Gets card's string.
					memset(card1, 0, sizeof(card1));
					strcpy(card1, get_card_str(me->card1_x, me->card1_y));
					
					// Server updates its own graphics.
					paint_card(me->card1_x, me->card1_y , me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
					write_card(me->card1_x, me->card1_y, card1, 200, 200, 200); // Paints the letters on the card grey.

					// Gives the player feedback.
					memset(str, 0, sizeof(str));
					code = 5;
					sprintf(str, "%d-%c%c\n", code, card1[0], card1[1]);
					write(me->socket, str, strlen(str));
					printf("Wrote: %s\n", str);

					// Updates the other players.
					memset(str, 0, sizeof(str));
					code = 1;
					sprintf(str, "%d-%c%c-%d-%d-%d-%d-%d\n", code, card1[0], card1[1], me->card1_x, me->card1_y, me->rgb_R, me->rgb_G, me->rgb_B);
					write2all(me, str);

					me->aux_5seconds = time(NULL);
					me->count_5seconds = 1;
				}
				else {
					printf("Bad message from server!\n\n");
				}
			}

			else if(code == 2) {
				if(sscanf(str, "-%*d-%d-%d\n", &(me->card2_x), &(me->card2_y)) == 2) {

					printf("x: %d	y: %d\n", me->card2_x, me->card2_y);
					printf("Card: %s	Status: %c\n\n", get_card_str(me->card2_x, me->card2_y), get_card_status(me->card2_x, me->card2_y));
					
					if( (get_card_status(me->card2_x, me->card2_y) != 'd') /*|| ((me->card1_x == me->card2_x) && (me->card1_y == me->card2_y))*/ ) {
						memset(str, 0, sizeof(str));
						code = 0;
						sprintf(str, "%d\n", code);
						write(me->socket, str, strlen(str));
						printf("Wrote: %s\n", str);
						return;
					}

					me->count_5seconds = 0;
					
					// Gets card's string.
					strcpy(card1, get_card_str(me->card1_x, me->card1_y));
					strcpy(card2, get_card_str(me->card2_x, me->card2_y));

					if(strcmp(card1, card2) == 0) {
						printf("Cards match!\n");
						found_pairs++;
						me->score++;
						printf("Nr. of pairs found: %d\n\n", found_pairs);

						// Updates the matched cards' status in the board.
						set_pair_traits(me->card1_x, me->card1_y, me->card2_x, me->card2_y, 'l', me->rgb_R, me->rgb_G, me->rgb_B);

						// Server updates its own graphics.
						paint_card(me->card1_x, me->card1_y, me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
						write_card(me->card1_x, me->card1_y, card1, 0, 0, 0); // Paints the letters on the card black.
						paint_card(me->card2_x, me->card2_y,me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
						write_card(me->card2_x, me->card2_y, card2, 0, 0, 0); // Paints the letters on the card black.

						if(found_pairs != ((dim_board*dim_board)/2)) { // If all pairs haven't been found yet.
							// Gives the player feedback.
							memset(str, 0, sizeof(str));
							code = 6;
							sprintf(str, "%d\n", code);
							write(me->socket, str, strlen(str));
							printf("Wrote: %s\n", str);

							// Updates the other players.
							memset(str, 0, sizeof(str));
							code = 2;
							sprintf(str, "%d-%c%c-%d-%d-%d-%d-%d-%d-%d\n", code, card2[0], card2[1], me->card1_x, me->card1_y, me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B);
							write2all(me, str);

							me->card1_x = -1;
							me->card1_y = -1;
							me->card2_x = -1;
							me->card2_y = -1;
						}
						else { // If all pairs have been found.
							// Gives the player feedback.
							memset(str, 0, sizeof(str));
							code = 7;
							sprintf(str, "%d\n", code);
							write(me->socket, str, strlen(str));
							printf("Wrote: %s\n", str);

							// Updates the other players.
							memset(str, 0, sizeof(str));
							code = 3;
							sprintf(str, "%d-%c%c-%d-%d-%d-%d-%d-%d-%d\n", code, card2[0], card2[1], me->card1_x, me->card1_y, me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B);
							write2all(me, str);

							me->card1_x = -1;
							me->card1_y = -1;
							me->card2_x = -1;
							me->card2_y = -1;

							// AQUI!!!! --------------------------------------------
							found_pairs = 0;
							terminate = 2;
							game = 0;
							// pthread_create(&end_gameID, NULL, end_game_thread, NULL);
						}

					}
					else {
						printf("Cards don't match.\n\n");

						// Updates the matched cards' status in the board.
						set_pair_traits(me->card1_x, me->card1_y, me->card2_x, me->card2_y, 'f', me->rgb_R, me->rgb_G, me->rgb_B);
						
						// Server updates its own graphics.
						paint_card(me->card1_x, me->card1_y, me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
						write_card(me->card1_x, me->card1_y, card1, 255, 0, 0); // Paints the letters on the card red.
						paint_card(me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B); // Paints the card's backgroud with the player's color.
						write_card(me->card2_x, me->card2_y, card2, 255, 0, 0); // Paints the letters on the card red.
						
						// Gives the player feedback.
						memset(str, 0, sizeof(str));
						code = 8;
						sprintf(str, "%d-%c%c\n", code, card2[0], card2[1]);
						write(me->socket, str, strlen(str));
						printf("Wrote: %s\n", str);

						// Updates the other players.
						memset(str, 0, sizeof(str));
						code = 4;
						sprintf(str, "%d-%c%c-%c%c-%d-%d-%d-%d-%d-%d-%d\n", code, card1[0], card1[1], card2[0], card2[1], me->card1_x, me->card1_y, me->card2_x, me->card2_y, me->rgb_R, me->rgb_G, me->rgb_B);
						write2all(me, str);
						
						me->aux_2seconds = time(NULL);
						me->count_2seconds = 1;
					}	
				}
				else {
					printf("Bad message from server!\n\n");
				}
			}

			// else if(code == 3) {
			// 	// Sets the card's status to down (d).
			// 	set_card_traits(me->card1_x, me->card1_y, 'd', me->rgb_R, me->rgb_G, me->rgb_B);

			// 	// Server updates its own graphics
			// 	paint_card(me->card1_x, me->card1_y, 255, 255, 255); // Paints the card' white.
				
			// 	// Notifies the oterh players.
			// 	memset(str, 0, sizeof(str));
			// 	code = 13;
			// 	sprintf(str, "%d-%d-%d\n", code, me->card1_x, me->card1_y);
			// 	write2all(me, str);

			// 	me->card1_x = -1;
			// 	me->card1_y = -1;
			// }

			// else if(code == 4) {	
			// 	// Sets the card's status to down (d).
			// 	set_pair_traits(me->card1_x, me->card1_y, me->card2_x, me->card2_y, 'd', me->rgb_R, me->rgb_G, me->rgb_B);

			// 	// Server updates its own graphics
			// 	paint_card(me->card1_x, me->card1_y, 255, 255, 255); // Paints the card' white.
			// 	paint_card(me->card2_x, me->card2_y, 255, 255, 255); // Paints the card' white.
				
			// 	// Notifies the oterh players.
			// 	memset(str, 0, sizeof(str));
			// 	code = 14;
			// 	sprintf(str, "%d-%d-%d-%d-%d\n", code, me->card1_x, me->card1_y, me->card2_x, me->card2_y);
			// 	write2all(me, str);

			// 	me->card1_x = -1;
			// 	me->card1_y = -1;
			// 	me->card2_x = -1;
			// 	me->card2_y = -1;
			// }

			else {
				printf("Impossible code number!\n\n");
			}

		}
		else
			printf("Game hasn't started yet. The server isn't supposed to be receiving messages from players!\n\n");

	} // End of if(sscanf(str, "%d%*s", &code) == 1).
	else
		printf("Couldn't read message!\n\n");

	return;
}

void *player_thread(void *arg)
{
	int n, i; // Aid variable.
	int code; // To store message's codes.
	char card[3];
	char *res_aux, buffer[200], final_msg[100], res[100];
	char str[100];
	char *str2 = NULL;
	time_t now;
	player *me = (player*) arg;

	memset(card, 0, sizeof(card));
	memset(buffer, 0, sizeof(buffer));
	memset(final_msg, 0, sizeof(final_msg));
	memset(res, 0, sizeof(res));

	me->card1_x = -1;
	me->card1_y = -1;
	me->card2_x = -1;
	me->card2_y = -1;

	// Sends initial message to player.
	memset(str, 0, sizeof(str));
	code = 9;
	sprintf(str, "%d-%d-%d-%d-%d-%d\n", code, dim_board, game, me->rgb_R, me->rgb_G, me->rgb_B);
	write(me->socket, str, strlen(str));
	printf("Wrote: %s\n", str);

	// Server sends the state of the board to new players.
	if(nr_players > 2 || game == 2) { // This is only necessary if there are more than 2 players connected. Before that, changes to the board aren't possible.
		for(int i=0; i<dim_board; i++){
			for(int j=0; j<dim_board; j++) {
				if(get_card_status(i, j) != 'd') {
					str2 = get_str2send(i, j);
					strcpy(str, str2);
					write(me->socket, str, strlen(str));
					printf("Wrote: %s\n", str);
				}
			}
		}

		code = 16;
		memset(str, 0, sizeof(str));
		sprintf(str, "%d\n", code);
		write(me->socket, str, strlen(str));
		printf("Wrote: %s\n", str);
	}

	if( nr_players==2 && (game==0 || game == 2) ) { // If there are 2 players connected but the game hasn't yet started or was frozen ...
		
		game = 1; // ... the game starts.

		code = 12;
		memset(str, 0, sizeof(str));
		sprintf(str, "%d\n", code);
		write(me->next->socket, str, strlen(str)); 	// game=0: Tells the player who first connected to start playing.
													// game=2: Tells the player who stayed connected with the game frozen, to continue playing.
		printf("Wrote: %s\n", str);
		
		write(me->socket, str, strlen(str)); // Tells the second player to start playing.
		printf("Wrote: %s\n", str);
	}

	while(terminate != 1) { // Server waits for player's decesion.
		
		if(me->count_2seconds == 1) {
			now = time(NULL);
			if(now - me->aux_2seconds >= 2) {
				me->count_2seconds = 0; // No need to keep counting.

				// Sets the card's status to down (d).
				set_pair_traits(me->card1_x, me->card1_y, me->card2_x, me->card2_y, 'd', me->rgb_R, me->rgb_G, me->rgb_B);

				// Server updates its own graphics
				paint_card(me->card1_x, me->card1_y, 255, 255, 255); // Paints the card' white.
				paint_card(me->card2_x, me->card2_y, 255, 255, 255); // Paints the card' white.

				// Notifies the player the 2 seconds have passed.
				memset(str, 0, sizeof(str));
				code = 19;
				sprintf(str, "%d\n", code);
				write(me->socket, str, strlen(str));
				
				// Notifies the oterh players.
				memset(str, 0, sizeof(str));
				code = 14;
				sprintf(str, "%d-%d-%d-%d-%d\n", code, me->card1_x, me->card1_y, me->card2_x, me->card2_y);
				write2all(me, str);

				me->card1_x = -1;
				me->card1_y = -1;
				me->card2_x = -1;
				me->card2_y = -1;
			}
		}

		if(me->count_5seconds == 1) {
			now = time(NULL);
			if(now - me->aux_5seconds >= 5) {
				me->count_5seconds = 0; // No need to keep counting.

				// Sets the card's status to down (d).
				set_card_traits(me->card1_x, me->card1_y, 'd', me->rgb_R, me->rgb_G, me->rgb_B);

				// Server updates its own graphics
				paint_card(me->card1_x, me->card1_y, 255, 255, 255); // Paints the card' white.

				// Notifies the player the 2 seconds have passed.
				memset(str, 0, sizeof(str));
				code = 18;
				sprintf(str, "%d\n", code);
				write(me->socket, str, strlen(str));
				
				// Notifies the oterh players.
				memset(str, 0, sizeof(str));
				code = 13;
				sprintf(str, "%d-%d-%d\n", code, me->card1_x, me->card1_y);
				write2all(me, str);

				me->card1_x = -1;
				me->card1_y = -1;
			}
		}
		
		memset(str, 0, sizeof(str));
		n = read(me->socket, str, sizeof(str));
		if(n <= 0) { // If the player disconnected (purposefully or otherwise) or there was no data to be read.
			if( (errno == EAGAIN || errno == EWOULDBLOCK) && (n==-1) ) { //  If there was an error because there wasn't data to be read ...
				continue; // ... there wasn't actually an error. Loop continues.
			}
			else { // If the player disconnected (purposefully or otherwise) ...
				printf("A player disconnected.\n\n");
				removePlayer(me); // ... the player is removed from the list of connected players.

				if(nr_players == 1) {
					game = 2;
	
					memset(str, 0, sizeof(str));
					code = 17;
					sprintf(str, "%d\n", code);
					write2all(NULL, str);

					printf("There's only 1 player in-game! Waiting for a second player...\n\n");
				}

				if(nr_players == 0) {
					game = 2;
					printf("There are 0 players in-game. Waiting for 2 players to join...\n\n");
				}
				
				pthread_exit(NULL); // ... and the thread is terminated.
			}
			
		}
		else { // If there's data to be read

			if(terminate == 2) {
				printf("Can't read since there's no game going-on!\n\n");
				continue;
			}

			strcat(buffer, str);

			res_aux = strstr(buffer, "\n"); // res_aux is pointing to "\n"'s first occurance in buffer.
			while(res_aux != NULL) { // While there's a message to be read stored in buffer.
				res_aux++;
				memset(res, 0, sizeof(res));
				strcpy(res, res_aux);

				memset(final_msg, 0, sizeof(final_msg));
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

	printf("A player terminated.\n\n");
	removePlayer(me);
	pthread_exit(NULL);
}

void *endGame_thread(void *arg) 
{
	int code;
	char str[50];
	player *winner = NULL;
	time_t aux_10seconds, now;

	aux_10seconds = time(NULL);

	pthread_rwlock_rdlock(&lock_players);

	// Server searches for winner.
	for(player* aux = players_head; aux != NULL; aux = aux->next) {
		if(winner == NULL) {
			winner = aux;
		}	
		else {
			if(winner->score <= aux->score) 
				winner = aux;
		}
	}

	// Sends message to winner.
	memset(str, 0, sizeof(str));
	code = 10;
	sprintf(str, "%d\n", code);
	write(winner->socket, str, strlen(str));
	printf("Wrote: %s\n", str);

	// Notifies losers. !!!!! VER DISTO !!!!!
	memset(str, 0, sizeof(str));
	code = 11;
	sprintf(str, "%d\n", code);

	for(player* aux = players_head; aux != NULL; aux = aux->next) {
		aux->score = 0;
		aux->card1_x = -1;
		aux->card1_y = -1;
		aux->card2_x = -1;
		aux->card2_y = -1;
		if(aux == winner) continue;
		else write(aux->socket, str, strlen(str));

		printf("Wrote to losers: %s\n", str);
	}

	pthread_rwlock_unlock(&lock_players);

	clear_board();
	close_board_windows();

	printf("The game ended! Wait for 10 seconds...\n\n");

	now = time(NULL);
	while(now - aux_10seconds <= 10)
		now = time(NULL);
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}
	if(TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(-1);
	}
	create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board); // GRAPHICS
	init_board(dim_board);

	terminate = 0;

	printf("A new game has started!\n\n");

	if(nr_players >= 2) {
		game = 1;
		printf("All set! There's more than 1 player connected.\n\n");
		memset(str, 0, sizeof(str));
		code = 12;
		sprintf(str, "%d\n", code);
		write2all(NULL, str);
	}
	else { 
		game = 0;
		printf("Waiting for a 2nd player to join...\n\n");
		memset(str, 0, sizeof(str));
		code = 17;
		sprintf(str, "%d\n", code);
		write2all(NULL, str);
	}

	
	pthread_exit(NULL);
}