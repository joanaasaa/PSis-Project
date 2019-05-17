#include "server_library.h"
#include "board_library.h"

int nr_players = 0;
player *players = NULL; // List of in-game players.

void addPlayer(int newfd)
{
	player *players_aux; 
	
	nr_players++;
	
	if(players == NULL) { // If the list is empty, the head is created.
		players = (player*) malloc(sizeof(player));
		if(players == NULL) {
			perror("malloc: ");
			pthread_exit(NULL);
		}

		players->next = NULL;
		players_aux = players;
	}
	else { // If the list isn't empty, the new player is inserted at the beginning of the list.
		players_aux = (player*) malloc(sizeof(player));
		if(players_aux == NULL) {
			perror("malloc: ");
			pthread_exit(NULL);
		}

		players_aux->next = players;
		players = players_aux;
	}

	players_aux->socket = newfd;
	players_aux->rgb_R = rand() % 255 + 1;
	players_aux->rgb_G = rand() % 255 + 1;
	players_aux->rgb_B = rand() % 127 + 128;

	pthread_create(&(players_aux->threadID), NULL, player_thread, players_aux);

	return;
}

void *player_thread(void *arg)
{
	int n;
	int game = 0; // Indica se o jogo já começou (1), ou não (0).
	int dim_board = get_dim_board();
	char str[25];
	player *me = (player*) arg;
	player *players_aux;

	if(nr_players >= 2) game = 1; // Se estiverem pelo menos 2 jogadores ligados, o jogo já começou.

	sprintf(str, "%d-%d-%d-%d-%d\n", dim_board, game, me->rgb_R, me->rgb_G, me->rgb_B);
	printf("str: %s\n", str);

	n = write(me->socket, str, strlen(str));
	printf("wrote %d characters.\n", n);

	for(int i=0; i<dim_board; i++){
		for(int j=0; j<dim_board; j++){

			memset(str, 0, sizeof(str));

			if(get_str2send(i, j, me->rgb_R, me->rgb_G, me->rgb_B) != NULL) { // Se esta carta é visível ao cliente (locked ou up).
				strcpy(str, get_str2send(i, j, me->rgb_R, me->rgb_G, me->rgb_B));
				printf("str: %s\n", str);
				
				n = write(me->socket, str, strlen(str));
				printf("wrote %d characters.\n", n);
			}
		}
	}

	while(1) {
		
		memset(str, 0, sizeof(str));
		
		n = read(me->socket, str, sizeof(str));
		if(n <= 0) { // If the player disconnected.
			perror("player socket: ");
			
			close(me->socket);
			
			
			pthread_exit(NULL);
		}
	}
}