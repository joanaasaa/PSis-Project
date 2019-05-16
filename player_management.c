#include "player_management.h"
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
	int dim_board = get_dim_board();
	player *me = (player*) arg;
	char str[25];
	int game = 0; // Variável que indica se um jogo já está a decorrer(1) ou não(0).

	if(nr_players > 1) game = 1; // Se tiver pelo menos 2 jogadores, isto é, se o jogo já começou.

	//strcpy(str, "%d-%d-%d-%d-%d\n", dim_board, game, me->rgb_R, me->rgb_G, me->rgb_B);
	sprintf(str, "%d-%d-%d-%d-%d\n", dim_board, game, me->rgb_R, me->rgb_G, me->rgb_B);
	write(me->socket, str, strlen(str));

	for(int i=0; i<dim_board; i++){
		for(int j=0; j<dim_board; j++){

			memset(str, 0, 24);

			if (get_str_send(i, j, me->rgb_R, me->rgb_G, me->rgb_B) != NULL) { // Se esta carta é visível ao cliente (locked ou up).
				strcpy(str, get_str_send(i, j, me->rgb_R, me->rgb_G, me->rgb_B));
				write(me->socket, str, strlen(str));
			}
		}
	}
	
	pthread_exit(NULL);
}