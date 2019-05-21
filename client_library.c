#include "client_library.h"
#include "graphics_library.h"
#include "libraries.h"

int terminate = 0;
int game = 0;
int board = 0; // To indicate if the board's graphics have already been initialized (1), or not (0).
int fd;
int dim_board = 0;
player_self me; // Player's info.
//card *board; // Game board stored as an allocated vector.
char card1[3], card2[3]; // String correspondente às cartas escolhidas da 1ª e 2ª vez. (correspondem a str_play1 e str_play2 no servidor).
int card1_index[2] = {-1, -1};
int card2_index[2] = {-1, -1};
SDL_Event event;
int code = 10; // Code used to identify the type of message that was received by the server.
		// 0 - Impossibilidade de escolher a carta seleccionada.
		// 1 - The 1st card was chosen.
		// 2 - The 2nd card was chosen, both cards are the same and the game goes on.
		// 3 - The 2nd card was chosen, both cards are the same and the game ended.
		// 4 - The 2nd card was chosen, but the cards are not a match.
		// 5 - Initial message containing the dim_board and other information.
		// 6 - Message announcing that this player is the winner and its score
		// 7 - Message announcing that the game just started and the player may start playing.
		// 8 - Message containing a card turned up in the current board (in case this player joins in the middle of a game).


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
	int sscanf_aux = 0;

	if( sscanf(str, "%d-%*s", &code) == 1) {
		printf("Success!\n");
		printf("Code: %d\n", code);
	}
	else
	{
		printf("Couldn't read message!\n");
		return;
	}

	if(code == 6) { // Winner.
		
		printf("You won! Score: %d\n", me.final_score);

		// DO LOTS OF OTHER STUFF!!

	}
		

	if(game == 0) { // If the game hasn't started yet.
		if(code == 5) { // Initial message.
			sscanf(final_msg, "%*d-%d-%d-%d-%d-%d\n", &dim_board, &game, &(me.rgb_R), &(me.rgb_G), &(me.rgb_B));
			if(game == 0) { // There's only one player connected. We have to wait for another one to join.
				game = 2; // The player has to wait for a "start\n" message.
				printf("dim_board = %d\n", dim_board);
				printf("Waiting for a second player...\n");
				return;
			}

			// PRINT INITIAL BOARD ON SCREEN.
			if(SDL_Init(SDL_INIT_VIDEO) < 0) {
				printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
				exit(-1);
			}
			if(TTF_Init() == -1) {
				printf("TTF_Init: %s\n", TTF_GetError());
				exit(2);
			}
			create_board_window(MAX_WINDOW, MAX_WINDOW, dim_board); // Prints a clean board to the screen.
			board = 1; 
		}
		else return;
	}
	else if(game == 2) { // If the player is connected to the server, but waiting for a second player to join.
		if(code == 7) { // Start.
			game = 1;
			printf("Start palying!\n");

			return;
		}
		else return;
	}
	// The function only gets to here if the game has already started?
	else if(game == 1 && board == 1) {
		if(code == 1) { // Se for a segunda jogada armazena na segunda carta.
			sscanf_aux = sscanf(final_msg, "%d-%c%c\n", &code, &card2[0], &card2[1]);
		}
		else if(code == 2 || code == 3 || code == 4) sscanf_aux = sscanf(final_msg, "%d-%c%c\n", &code, &card1[0], &card1[1]); // Se for a primeira jogada armazena na primeira carta.
		else if(code == 0) printf("Invalid play!\n");

		if(sscanf_aux == 3) {
			switch(code) {
				case(1): // Foi a primeira escolha de uma jogada.
					paint_card(card1_index[0], card1_index[1], 7, 200, 100); // Pinta o fundo da carta de verde.
					write_card(card1_index[0], card1_index[1], card1, 200, 200, 200); // Pinta as letras de cinzento.
					break;
				case(3): // O jogo terminou.
					me.score++;
					terminate = 1;
				case(2): // The play's 2nd choice was made. The cards matched but the game still goes on.
					me.score++;
					paint_card(card1_index[0], card1_index[1], 7, 200, 100); // Pinta o fundo da carta de verde.
					write_card(card1_index[0], card1_index[1], card1, 0, 0, 0); // Pinta as letras a preto.
					paint_card(card2_index[0], card2_index[1], 7, 200, 100); // Pinta o fundo da carta de verde.
					write_card(card2_index[0], card2_index[1], card2, 0, 0, 0); // Pinta as letras a preto.
					break;
				case(4):
					paint_card(card1_index[0], card1_index[1], 7, 200, 100); // Pinta o fundo da carta de verde.
					write_card(card1_index[0], card1_index[1], card1, 255, 0, 0); // Pinta as letras a vermelho.
					paint_card(card2_index[0], card2_index[1], 7, 200, 100); // Pinta o fundo da carta de verde.
					write_card(card2_index[0], card2_index[1], card2, 255, 0, 0); // Pinta as letras a vermelho.
					
					sleep(2); 

					// Pinta de novo as cartas de branco.
					paint_card(card1_index[0], card1_index[1], 255, 255, 255); 
					paint_card(card2_index[0], card2_index[1], 255, 255, 255);
					break;
			}
		}
		//play_game();
	}	
}

void *thread_read(void *arg) 
{
	int n;
	char str[100];
	char *res_aux, buffer[200] = "\0", final_msg[25], res[100];

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

				interpret_final_msg(final_msg);

				res_aux = strstr(buffer, "\n");
			}
		}
	}
	pthread_exit(0);
}

void *thread_write(void *arg)
{
	char str[24];

	while(!terminate) {	
		
		if(board== 1) { // If the board's graphics have already been initialized.
			
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					
					case(SDL_QUIT): {
						printf("Quitting...\n");
						terminate = SDL_TRUE;
						break;
					}

					case(SDL_MOUSEBUTTONDOWN): {
						if(game==2)
							printf("Can't select any cards yet! Still waiting for a 2nd player to join.");
						
						if(game==1) { // NAO HAVERÁ PROBLEMAS AQUI POR CAUSA DOS CODES TODOS?????????????????????????????????????????
							if(code == 1) {
								get_board_card(event.button.x, event.button.y, &card2_index[0], &card2_index[1]); // Se a primeira carta ja foi jogada (sendo esta a segunda).
								sprintf(str, "%d-%d\n", card2_index[0], card2_index[1]);
								write(fd, str, strlen(str)); // Recebe uma string do servidor.
								printf("click_2 (%s)\n", str); // Imprime na consola as coordenadas do rato enviadas ao sevidor.
							}
							else {
								get_board_card(event.button.x, event.button.y, &card1_index[0], &card1_index[1]); // Se esta é a primeira jogada.
								sprintf(str, "%d-%d\n", card1_index[0], card1_index[1]);
								write(fd, str, strlen(str)); // Recebe uma string do servidor.
								printf("click (%s)\n", str); // Imprime na consola as coordenadas do rato enviadas ao sevidor.
							}
						}

					}

				}
			}
		}
	}

	pthread_exit(0);
}