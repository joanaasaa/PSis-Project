#include "bot_client_library.h"

int game = 0; // Indicates the game's state.
int graphics = 0; // Indicates if the board's graphics have already been initialized (1), or not (0).
int terminate = 0; // Indicates if the program terminated (1) or not (0).
int fd; // Player socket.
int dim_board = 0;
bot_player_self me; // Player's info.
SDL_Event event;
//char card1[3], card2[3]; // Stores the 1st and 2nd choices of cards of each play.
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
	int chosen_x1, chosen_y1, chosen_x2, chosen_y2;
	//int chosen_x1, chosen_y1, chosen_x2, chosen_y2;
	//int print_R, print_G, print_B;
	char str[100];
	//char print_str1[3], print_str2[3];
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

			else if(code == 3) { // A player made its 2nd choice, the cards match and the game ends.
				printf("Awaiting game results...\n");
				waiting = 1;
			}

			else if(code == 6 || code == 7) { // Player made it's 2nd choice and the cards match.

				me.score++;
				card1_x = -1;
				card1_y = -1;
				card2_x = -1;
				card2_y = -1;
				//memset(card1, 0, sizeof(card1));
				//memset(card2, 0, sizeof(card2));

				if(code == 7) { // Game ends.
					printf("Awaiting game results...\n\n");
					waiting = 1; // Player waits for results.
				}

			}

			else if(code == 10 || code == 11) { // Winner or loser message.
				
				if(waiting == 1) {
					if(code == 10)
						printf(":)	Congratualations, winner!\n\n");
					else // code == 11
						printf(":(	Bummer, you lost... Better luck next time!\n\n");

					printf("Your final score: %d\n\n", me.score);

					waiting = 0;
					me.score = 0;

					game = 3;
				}
				else {
					printf("Player wasn't waiting for feedback from server!\n\n");
				}

			}

			else if(code == 17) { // Player is the only one in-game.
				game = 2; // Has to wait for code 12 message.
				printf("You're the only player in-game. Waiting for a second player to join...\n\n");
			}

			else if(code == 18) { // Player made its 1st choice, but didn't make 2nd. 5 seconds ran out.

				card1_x = -1;
				card1_y = -1;
			}

			else if(code == 19) { // Player made a bad play. 2 senconds ran out: Player can play again.

				card1_x = -1;
				card1_y = -1;
				card2_x = -1;
				card2_y = -1;
			}

			else {
				printf("Impossible code number!\n\n");
			}
		}
	}
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
	time_t now, start_time;

	srand(time(NULL));

	while(!terminate) {

		if(waiting != 0) // If player is waiting for something.
			printf("Wait!\n\n");
		else {

			now = time(NULL);

			if(game == 2) // If game is stopped.
				printf("Game is stopped! Still waiting for a 2nd player to join...\n\n");

			else if(game == 3) { // If server is restarting game.
				printf("Can't select any cards yet! Waiting for another game to start...\n\n");
			}

			else if((game == 1) && (now - start_time > 1)) { // If game is active and it has been more than 1 second since the last pick (bot plays every second).

				card_x_aux = random() % dim_board;
				card_y_aux = random() % dim_board;
				start_time = time(NULL);

				printf("Random coordinate x: %d\n", card_x_aux);
				printf("Random coordinate y: %d\n", card_y_aux);


				if(card1_x == -1) { // If this is the 1st choice, in a play.
					card1_x = card_x_aux;
					card1_y = card_y_aux;

					memset(str, 0, sizeof(str));
					code = -1;
					sprintf(str, "%d-%d-%d\n", code, card1_x, card1_y);
					write(fd, str, strlen(str));
					printf("Sent: %s\n", str);

				}
				else if(card2_x == -1) { // If this is the 2nd choice, in a play.
					card2_x = card_x_aux;
					card2_y = card_y_aux;

					memset(str, 0, sizeof(str));
					code = -2;
					sprintf(str, "%d-%d-%d\n", code, card2_x, card2_y);
					write(fd, str, strlen(str));
					printf("Sent: %s\n", str);

				}
				else // If the player has already made both his choices.
					printf("Wait!\n\n");

			}
		}

	}

	pthread_exit(0);
}