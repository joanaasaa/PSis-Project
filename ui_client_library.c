#include "ui_client_library.h"

int game = 0; // Indicates the game's state.
int graphics = 0; // Indicates if the board's graphics have already been initialized (1), or not (0).
int terminate = 0; // Indicates if the program terminated (1) or not (0).
int fd; // Player socket.
int dim_board = 0;
player_self me; // Player's info.
SDL_Event event;
char card1[3], card2[3]; // Stores the 1st and 2nd choices of cards of each play.
int card1_x = -1;
int card1_y = -1;
int card2_x = -1;
int card2_y = -1;
int waiting = 0; // Indicates if the player's waiting for feedback on its choice (1), or not (0).

void argumentControl(int argc, char const *argv[])
{
    char str[20];

	if(argc < 2) {
    	printf("Second argument has to be the server's IP address!\n");
    	exit(-1);
  	}
	if(argc > 2) {
    	printf("Unsupported number of arguments!\n");
    	exit(-1);
  	}

	return;
}

void create_socket(const char *server_ip)
{
	int n;
	struct sockaddr_in server_addr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
  	if(fd<0){
    	perror("socket");
    	exit(-1);
  	}

  	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	inet_aton(server_ip, &server_addr.sin_addr); //argv[1] => SERVER_IP

	n = connect(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n == -1){
		perror("connect");
		exit(-1);
	}

	int flags = fcntl(fd, F_GETFL, 0);
  	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return;
}

void interpret_final_msg(char final_msg[])
{
	int game_server;
	int code; // Stores server's messages' codes.
	int print_x1, print_y1, print_x2, print_y2;
	int print_R, print_G, print_B;
	char str[100];
	char print_str1[3], print_str2[3];
	char status;

	strcpy(str, final_msg);
	printf("Read: %s\n", str);

	if(sscanf(str, "%d%*s", &code) == 1) {
		printf("Received message with code %d\n\n", code);

		if(game == 0) { 

			if(code == 9) { // Start message.
				if(sscanf(str, "%*d-%d-%d-%d-%d-%d\n", &dim_board, &game_server, &(me.rgb_R), &(me.rgb_G), &(me.rgb_B)) == 5) {
					if(game_server == 0) { // Game hasn't started.
						game = 2; // Only one connected. Waits for 2nd player.
						printf("Waiting for a second player...\n\n");
					}
					else if(game_server == 1) { // The game has already started.
						game = 1;
						waiting = 2; // Waits for board update.
						printf("Waiting for board updates...\n\n");
					}
					else if(game_server == 2) { // The game is frozen.
						game = 2;
						waiting = 2; // Waits for board update.
						printf("Game is frozen. Waiting for more players to join...\n\n"); // Needs code 12 to continue playing.
					}

					me.score = 0;

					// Print initial board on screen:
					if(SDL_Init(SDL_INIT_VIDEO) < 0) {
						printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
						exit(-1);
					}
					if(TTF_Init() == -1) {
						printf("TTF_Init: %s\n", TTF_GetError());
						exit(2);
					}
					create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board, 1); // Prints a clean board to the screen.

					graphics = 1;
				}
				else {
					printf("Bad message from server!\n\n");
				}
			}
			else {
				printf("Impossible code number!\n\n");
			}

		}

		else if(game == 2) { // Player is connected, but player is waiting for a 2nd player. The game is either frozen or hasn't started. 

			if(code == 12) {
				game = 1;
				printf("Start palying!\n\n");
			}
			else if(code == 15 || code == 16) {

				if(sscanf(str, "%*d-%c%c-%c-%d-%d-%d-%d-%d\n", &print_str1[0], &print_str1[1], &status, &print_x1, &print_y1, &print_R, &print_G, &print_B) == 8) { //------------------------------------------------

					paint_card(print_x1, print_y1, print_R, print_G, print_B); // Paints the card's backgroud with our color.

					if(status == 'l') { // Locked card.

						write_card(print_x1, print_y1, print_str1, 0, 0, 0); // Paints the letters on the card black.
					}
					else if(status == 'u') { // Up card.

						write_card(print_x1, print_y1, print_str1, 200, 200, 200); // Paints the letters on the card grey.
					}
					else if(status == 'f') { // Up card chosen by a player who chose 2 different cards.

						write_card(print_x1, print_y1, print_str1, 255, 0, 0); // Paints the letters on the card red.
					}
				}
				else if(code == 16) {
					waiting = 0; // If this is the last card received, this player is no longer  waiting for another board card.
				}
				else {
					printf("Bad message from server!\n\n");
				}

				
			}
			else {
				printf("Impossible code number!\n\n");
			}
		}

		else if(game == 3) { // Player is connected, but the server is restarting another game.

			if(code == 12) { // Game has restarted. Player can start playing.
				game = 1; // Game is active.
				printf("Start palying!\n\n");
			}

			else if(code == 17) { // Game has restarted, but there aren't enough players.
				game = 2; // Player has to wait for code 12 message.
				printf("You're the only player in-game. Waiting for a second player to join...\n\n");
			}

			else {
				printf("Impossible code number!\n\n");
			}
		}

		else { // game = 1

			if(code == 0) {
				if(card2_x != -1) { // If the choice was the 2nd in a play.
					card2_x = -1;
					card2_y = -1;
				}
				else {
					card1_x = -1;
					card1_y = -1;
				}
			}

			else if(code == 1) { // A player made its 1st choice.
				if(sscanf(str, "%*d-%c%c-%d-%d-%d-%d-%d\n", &(print_str1[0]), &(print_str1[1]), &print_x1, &print_y1, &print_R, &print_G, &print_B) == 7) {
					print_str1[2] = '\0';
					paint_card(print_x1, print_y1 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x1, print_y1, print_str1, 200, 200, 200); // Paints the letters on the card grey.
				}
				else {
					printf("Bad message from server!\n");
				}
			}

			else if(code == 2 || code == 3) { // A player made its 2nd choice and the cards match.
				if(sscanf(str, "%*d-%c%c-%d-%d-%d-%d-%d-%d-%d\n", &(print_str1[0]), &(print_str1[1]), &print_x1, &print_y1, &print_x2, &print_y2, &print_R, &print_G, &print_B) == 9) {
					print_str1[2] = '\0';
					paint_card(print_x1, print_y1 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x1, print_y1, print_str1, 0, 0, 0); // Paints the letters on the card black.
					paint_card(print_x2, print_y2 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x2, print_y2, print_str1, 0, 0, 0); // Paints the letters on the card black.

					if(code == 3) { // The game ends.
						printf("Awaiting game results...\n");
						waiting = 1;
					}
				}
				else {
					printf("Bad message from server!");
				}
			}

			else if(code == 4) { // A player made its 2nd choice but the cards don't match.
				if(sscanf(str, "%*d-%c%c-%c%c-%d-%d-%d-%d-%d-%d-%d\n", &(print_str1[0]), &(print_str1[1]), &(print_str2[0]), &(print_str2[1]), &print_x1, &print_y1, &print_x2, &print_y2, &print_R, &print_G, &print_B) == 11) {
					print_str1[2] = '\0';
					print_str2[2] = '\0';
					paint_card(print_x1, print_y1 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x1, print_y1, print_str1, 255, 0, 0); // Paints the letters on the card red.
					paint_card(print_x2, print_y2 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x2, print_y2, print_str2, 255, 0, 0); // Paints the letters on the card red.
				}
				else {
					printf("Bad message from server!\n\n");
				}
			}

			else if(code == 5) { // Player made its first choice.

				if(sscanf(str, "%*d-%c%c\n", &(card1[0]), &(card1[1])) == 2) {
					card1[2] = '\0';
					paint_card(card1_x, card1_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
					write_card(card1_x, card1_y, card1, 200, 200, 200); // Paints the letters on the card grey.
				}
				else {
					printf("Bad message from server!\n\n");
				}

			}

			else if(code == 6 || code == 7) { // Player made it's 2nd choice and the cards match.
				
				paint_card(card1_x, card1_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
				write_card(card1_x, card1_y, card1, 0, 0, 0); // Paints the letters on the card black.
				paint_card(card2_x, card2_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
				write_card(card2_x, card2_y, card1, 0, 0, 0); // Paints the letters on the card black.
				me.score++;
				card1_x = -1;
				card1_y = -1;
				card2_x = -1;
				card2_y = -1;
				memset(card1, 0, sizeof(card1));
				memset(card2, 0, sizeof(card2));

				if(code == 7) { // Game ends.
					printf("Awaiting game results...\n\n");
					waiting = 1; // Player waits for results.
				}

			}

			else if(code == 8) { // Player made its 2nd choide, but the cards don't match.
				
				if(sscanf(str, "%*d-%c%c\n", &(card2[0]), &(card2[1])) == 2) {
					card2[2] = '\0';
					paint_card(card1_x, card1_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
					write_card(card1_x, card1_y, card1, 255, 0, 0); // Paints the letters on the card red.
					paint_card(card2_x, card2_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
					write_card(card2_x, card2_y, card2, 255, 0, 0); // Paints the letters on the card red.
				}
				else {
					printf("Bad message from server!\n\n");
				}

			}

			else if(code == 10 || code == 11) { // Winner or loser message.
				
				if(waiting == 1) {
					if(code == 10)
						printf(":D	Congratualations, winner!\n\n");
					else // code == 11
						printf(":(	Bummer, you lost... Better luck next time!\n\n");

					printf("Your final score: %d\n\n", me.score);

					graphics = 0;

					create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board, 0); // Prints a clean board to the screen.
					graphics = 1;

					waiting = 0;
					me.score = 0;

					game = 3;
				}
				else {
					printf("Player wasn't waiting for feedback from server!\n\n");
				}

			}

			else if(code == 13) { // A player made 1st choice, but didn't make 2nd.
				
				if(sscanf(str, "%*d-%d-%d\n", &print_x1, &print_y1) == 2) {
					paint_card(print_x1, print_y1, 255, 255, 255); // Paints the card white.
				}
				else {
					printf("Bad message from server!\n\n");
				}

			}

			else if(code == 14) { // A player made a bad play.
				
				if(sscanf(str, "%*d-%d-%d-%d-%d\n", &print_x1, &print_y1, &print_x2, &print_y2) == 4) {
					paint_card(print_x1, print_y1, 255, 255, 255); // Paints the card white.
					paint_card(print_x2, print_y2, 255, 255, 255); // Paints the card white.
				}
				else {
					printf("Bad message from server!\n\n");
				}

			}

			else if(code == 15 || code == 16) { // Receiving board...
				
				if(sscanf(str, "%*d-%c%c-%c-%d-%d-%d-%d-%d\n", &print_str1[0], &print_str1[1], &status, &print_x1, &print_y1, &print_R, &print_G, &print_B) == 8) { //------------------------------------------------

					paint_card(print_x1, print_y1, print_R, print_G, print_B); // Paints the card's backgroud with our color.

					if(status == 'l') { // Locked card.

						write_card(print_x1, print_y1, print_str1, 0, 0, 0); // Paints the letters on the card black.
					}
					else if(status == 'u') { // Up card.

						write_card(print_x1, print_y1, print_str1, 200, 200, 200); // Paints the letters on the card grey.
					}
					else if(status == 'f') { // Up card chosen by a player who chose 2 different cards.

						write_card(print_x1, print_y1, print_str1, 255, 0, 0); // Paints the letters on the card red.
					}
				}
				else if(code == 16) {
					waiting = 0; // If this is the last card received, this player is no longer  waiting for another board card.
				}
				else {
					printf("Bad message from server!\n\n");
				}
			}

			else if(code == 17) { // Player is the only one in-game.
				game = 2; // Has to wait for code 12 message.
				printf("You're the only player in-game. Waiting for a second player to join...\n\n");
			}

			else if(code == 18) { // Player made its 1st choice, but didn't make 2nd. 5 seconds ran out.
				// Player updates its graphics.
				paint_card(card1_x, card1_y, 255, 255, 255); // Paints the card' white.

				card1_x = -1;
				card1_y = -1;
			}

			else if(code == 19) { // Player made a bad play. 2 senconds ran out: Player can play again.
				// Player updates its graphics.
				paint_card(card1_x, card1_y, 255, 255, 255); // Paints the card' white.
				paint_card(card2_x, card2_y, 255, 255, 255); // Paints the card' white.

				card1_x = -1;
				card1_y = -1;
				card2_x = -1;
				card2_y = -1;
			}

			else {
				printf("Impossible code number!\n\n");
			}

		} // End of game = 1.

	} // End of if(sscanf(str, "%d%*s", &code) == 1).
	else
		printf("Couldn't read message!\n\n");

	return;
} // End of function.

void *thread_read(void *arg)
{
	int n;
	int code;
	char str[100];
	char *res_aux, buffer[200], final_msg[100], res[100];
	time_t now;

	memset(buffer, 0, sizeof(buffer));
	memset(final_msg, 0, sizeof(final_msg));
	memset(res, 0, sizeof(res));

	while(!terminate) {

		memset(str, 0, sizeof(str));

		n = read(fd, &str, sizeof(str)); // Recebe uma string do servidor.
		if(n<=0) { // If there was an error reading.
			if( (errno == EAGAIN || errno == EWOULDBLOCK) && (n==-1) )
				continue;
			else {
				terminate = 1;
				close(fd);
				break;
			}
		}
		else {
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

				interpret_final_msg(final_msg);

				res_aux = strstr(buffer, "\n");
			}
		}
	}
	pthread_exit(0);
}

void *thread_write(void *arg)
{
	int code; // Stores server's messages' codes.
	int card_x_aux = -1;
	int card_y_aux = -1;
	char str[24];
	time_t now;

	while(!terminate) {

		if(graphics == 1) { // If the board's graphics have already been initialized.

			while(SDL_PollEvent(&event)) {

				switch(event.type) {

					case(SDL_QUIT): {

						if(waiting != 0) // If player is waiting for something.
							printf("Wait!\n");
						else {
							printf("Quitting...\n");

							terminate = SDL_TRUE;
						}

						break;
					}

					case(SDL_MOUSEBUTTONDOWN): {

						get_board_card(event.button.x, event.button.y, &card_x_aux, &card_y_aux);

						if(waiting != 0) // If player is waiting for something.
							printf("Wait!\n\n");
						else {
							if(game == 2) // If game is stopped.
								printf("Game is stopped! Still waiting for a 2nd player to join...\n\n");

							else if(game == 3) { // If server is restarting game.
								printf("Can't select any cards yet! Waiting for another game to start...\n\n");
							}

							else if(game == 1) { // If game is active.

								if(card1_x == -1) { // If this is the 1st choice, in a play.
									card1_x = card_x_aux;
									card1_y = card_y_aux;

									memset(str, 0, sizeof(str));
									code = -1;
									sprintf(str, "%d-%d-%d\n", code, card1_x, card1_y);
									write(fd, str, strlen(str));
									printf("Sent: %s\n", str);

									//waiting = 1; // Player is waiting for server's feedback on choice.
								}
								else if(card2_x == -1) { // If this is the 2nd choice, in a play.
									card2_x = card_x_aux;
									card2_y = card_y_aux;

									memset(str, 0, sizeof(str));
									code = -2;
									sprintf(str, "%d-%d-%d\n", code, card2_x, card2_y);
									write(fd, str, strlen(str));
									printf("Sent: %s\n", str);

									//waiting = 1; // Player is waiting for server's feedback on choice.
								}
								else // If the player has already made both his choices.
									printf("Wait!\n\n");
							}
						}

						break;
					} // End of case(SDL_MOUSEBUTTONDOWN):
				} // End of switch.
			} // End of while(SDL_PollEvent(&event))
		} // End of if(board == 1)
	} // End of while(!terminate)

	if(graphics == 1) close_board_windows();
	pthread_exit(0);
} // End of thread.
