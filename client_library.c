#include "libraries.h"
#include "client_library.h"
//#include "board_library.h"
#include "graphics_library.h"

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
int count_2seconds = 0; // Indicates if the program is counting 2 seconds, during which the player is blocked from playing, (1), or not (0). Counts 2 seconds after a bad play.
int count_5seconds = 0; // Indicates if the program is counting 5 seconds (1), or not (0). It starts counting after the player's first choice
time_t aux_2seconds, aux_5seconds;

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
	int code; // Stores server's messages' codes.
	int print_x1, print_y1, print_x2, print_y2;
	int print_R, print_G, print_B;
	int final_score;
	char str[100];
	char print_str1[3], print_str2[3];
	char status;

	strcpy(str, final_msg);
	printf("Message Received : %s\n", str);

	if(sscanf(str, "%d%*s", &code) == 1) {
		printf("Received message with code %d\n\n", code);

		if(game == 0) {
			
			if(code == 9) {
				if(sscanf(str, "%*d-%d-%d-%d-%d-%d\n", &dim_board, &game, &(me.rgb_R), &(me.rgb_G), &(me.rgb_B)) == 5) {
					if(game == 0) { // There's only one player connected. We have to wait for another one to join.
						game = 2; // The player has to wait for a "start\n" message.
						printf("Waiting for a second player...\n");
					}
					else if(game == 1) { 
						if(waiting != 3) { // The player joined in the middle of a game. (If waiting == 3 it means the player was already playing and this is a new game).
							waiting = 2; // The player is now waiting for the server to send him all the cards that are already known by other players.
						}
					}

					// Print initial board on screen:
					if(SDL_Init(SDL_INIT_VIDEO) < 0) {
						printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
						exit(-1);
					}
					if(TTF_Init() == -1) {
						printf("TTF_Init: %s\n", TTF_GetError());
						exit(2);
					}
					create_board_window(WINDOW_SIZE, WINDOW_SIZE, dim_board); // Prints a clean board to the screen.

					graphics = 1;
				}
				else {
					printf("Bad message from server!");
				}
			}
			else {
				printf("Impossible code number!");
			}

		}

		else if(game == 2) {
			
			if(code == 12) {
				game = 1;
				printf("Start palying!\n");
			}
			else {
				printf("Impossible code number!");
			}
		}

		else { // game = 1

			if(code == 0) {
				if(waiting == 1) {
					waiting = 0;
					// Erase previous card choice:
					if(card1_x == -1) { // if it was the second choice.
						card2_x = -1;
						card2_y = -1;
					}
					else { // if it was the first choice.
						card1_x = -1;
						card1_y = -1;
					}
				}
				else {
					printf("Player wasn't waiting for feedback from server!");
				}
			}

			else if(code == 1) { // A player made its 1st choice.
				if(sscanf(str, "%*d-%c%c-%d-%d-%d-%d-%d\n", &(print_str1[0]), &(print_str1[1]), &print_x1, &print_y1, &print_R, &print_G, &print_B) == 7) {
					print_str1[2] = '\0';
					paint_card(print_x1, print_y1 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x1, print_y1, print_str1, 200, 200, 200); // Paints the letters on the card grey.
				}
				else {
					printf("Bad message from server!");
				}
			}

			else if(code == 2 || code == 3) { // A player made its 2nd choice. The cards match but the game continues || ends.
				if(sscanf(str, "%*d-%c%c-%d-%d-%d-%d-%d-%d-%d\n", &(print_str1[0]), &(print_str1[1]), &print_x1, &print_y1, &print_x2, &print_y2, &print_R, &print_G, &print_B) == 9) {
					print_str1[2] = '\0';
					paint_card(print_x1, print_y1 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x1, print_y1, print_str1, 0, 0, 0); // Paints the letters on the card black.
					paint_card(print_x2, print_y2 , print_R, print_G, print_B); // Paints the card's backgroud with the player's color.
					write_card(print_x2, print_y2, print_str1, 0, 0, 0); // Paints the letters on the card black.
				
					if(code == 3) {
						printf("Awaiting game results...\n");
						waiting = -1;
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

			else if(code == 5) { // I made my first choice.
				if(waiting == 1) {
					waiting = 0;
					if(sscanf(str, "%*d-%c%c\n", &(card1[0]), &(card1[1])) == 2) {
						card1[2] = '\0';
						paint_card(card1_x, card1_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
						write_card(card1_x, card1_y, card1, 200, 200, 200); // Paints the letters on the card grey.

						aux_5seconds = time(NULL); 
						count_5seconds = 1; // Starts counting the time until our next play
					}
					else {
						printf("Bad message from server!\n\n");
					}
				}
				else {
					printf("Player wasn't waiting for feedback from server!\n\n");
				}
			}

			else if(code == 6 || code == 7) { // I made my 2nd choice and the cards match.
				if(waiting == 1) {
					waiting = 0;
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

					if(code == 7) {
						printf("Awaiting game results...\n\n");
						waiting = -1;
					}
				}
				else {
					printf("Player wasn't waiting for feedback from server!\n\n");
				}
			}

			else if(code == 8) { // We made our 2nd choice and the cards don't match.
				if(waiting == 1) {
					waiting = 0;
					if(sscanf(str, "%*d-%c%c\n", &(card2[0]), &(card2[1])) == 2) {
						card2[2] = '\0';
						paint_card(card1_x, card1_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
						write_card(card1_x, card1_y, card1, 255, 0, 0); // Paints the letters on the card red.
						paint_card(card2_x, card2_y, me.rgb_R, me.rgb_G, me.rgb_B); // Paints the card's backgroud with our color.
						write_card(card2_x, card2_y, card2, 255, 0, 0); // Paints the letters on the card red.

						aux_2seconds = time(NULL); 
						count_2seconds = 1; // Starts counting the time until we're unblocked from playing.
					}
					else {
						printf("Bad message from server!\n\n");
					}
				}
				else {
					printf("Player wasn't waiting for feedback from server!\n\n");
				}
			}

			else if(code == 10 || code == 11) { // Received message saying this player was the winner or he lost.
				if(waiting == -1) {
					//waiting = 0; //VER ABAIXO: WAITING = 3.
					if(sscanf(str, "%*d-%d\n", &final_score) == 1) {
						if(code == 10)
							printf("Congratualations, winner!\n\n");
						else // code == 11
							printf("Bummer, you lost... Better luck next time!\n\n");
						
						printf("Your final score: %d\n", final_score);

						// TAKE ACTIONS TO RESTART GAME !!!!
						game = 0; // To receive new initial message.
						close_board_windows();
						graphics = 0;
						waiting = 3; // Waits for a new game to start.
					}
					else {
						printf("Bad message from server!\n\n");
					}
				}
				else {
					printf("Player wasn't waiting for feedback from server!\n\n");
				}
			}

			else if(code == 13) {
				if(sscanf(str, "%*d-%d-%d\n", &print_x1, &print_y1) == 2) {
					paint_card(print_x1, print_y1, 255, 255, 255); // Paints the card white.
				}
				else {
					printf("Bad message from server!\n\n");
				}
			}

			else if(code == 14) {
				if(sscanf(str, "%*d-%d-%d-%d-%d\n", &print_x1, &print_y1, &print_x2, &print_y2) == 4) {
					paint_card(print_x1, print_y1, 255, 255, 255); // Paints the card white.
					paint_card(print_x2, print_y2, 255, 255, 255); // Paints the card white.
				}
				else {
					printf("Bad message from server!\n\n");
				}
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
				else {
					printf("Bad message from server!\n\n");
				}
				
				if(code == 16) 
					waiting = 0; // If this is the last card received, this player is no longer  waiting for another board card.

			}

			else {
				printf("Impossible code number!\n\n");
			}

		} // End of game = 1.

	} // End of if(sscanf(str, "%d%*s", &code) == -1).
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

		if(count_2seconds == 1) { // If the program is counting the time until player is blocked from playing.
			now = time(NULL);
			if(now - aux_2seconds >= 2) { 
				count_2seconds = 0; // No need to keep counting.

				paint_card(card1_x, card1_y, 255, 255, 255); // Paints the card' white.
				paint_card(card2_x, card2_y, 255, 255, 255); // Paints the card' white.

				card1_x = -1; // The first play is undone.
				card1_y = -1; // The first play is undone.
				card2_x = -1; // The first play is undone.
				card2_y = -1; // The first play is undone.
				
				memset(str, 0, sizeof(str));
				code = 20;
				sprintf(str, "%d\n", code);
				write(fd, str, strlen(str));
			}
		}

		if(count_5seconds == 1) { // If the program is counting the time until player makes its 2nd choice.
			now = time(NULL);
			if(now - aux_5seconds >= 5) { 
				count_5seconds = 0; // No need to keep counting.

				paint_card(card1_x, card1_y, 255, 255, 255); // Paints the card' white.

				card1_x = -1; // The first play is undone.
				card1_y = -1; // The first play is undone.
				
				memset(str, 0, sizeof(str));
				code = 19;
				sprintf(str, "%d\n", code);
				write(fd, str, strlen(str));
			}
		}

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

						if(count_2seconds == 1 || waiting == 1) // If a player is blocked after a bad choice or is waiting for feedback on play.
							printf("Wait!\n");
						else {
							printf("Quitting...\n");

							if(game == 1 || game == 2) {

								if(count_5seconds == 1) {
									memset(str, 0, sizeof(str));
									code = -3; // Send message to the server as if the 5 seconds have passed.
									sprintf(str, "%d\n", code);
									write(fd, str, strlen(str));
								}
							}

							terminate = SDL_TRUE;
						}
						
						break;
					} 

					case(SDL_MOUSEBUTTONDOWN): {

						get_board_card(event.button.x, event.button.y, &card_x_aux, &card_y_aux);

						if(count_2seconds==1 || waiting!=0) // If a player is blocked after a bad choice or is waiting for feedback on play or is waiting for board cards (when joining in the middle of a game).
							printf("Wait!\n");
						else { // If the player's not blocked.
							if(game==2)
								printf("Can't select any cards yet! Still waiting for a 2nd player to join.");

							if(game==1) {
								if(card1_x == -1) { // If this is the first choice, in a play.
									card1_x = card_x_aux;
									card1_y = card_y_aux;

									memset(str, 0, sizeof(str));
									code = 17;
									sprintf(str, "%d-%d-%d\n", code, card1_x, card1_y);
									write(fd, str, strlen(str));

									waiting = 1; // Player is waiting for server's feedback on choice.
								}
								else { // If this isn't the player's first choice.

									count_5seconds = 0;
									
									card2_x = card_x_aux;
									card2_y = card_y_aux;

									memset(str, 0, sizeof(str));
									code = 18;
									sprintf(str, "%d-%d-%d\n", code, card2_x, card2_y);
									write(fd, str, strlen(str));

									waiting = 1; // Player is waiting for server's feedback on choice.
								}
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