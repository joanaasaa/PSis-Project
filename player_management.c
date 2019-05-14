#include "player_management.h"

int nr_players = 0;
player *players = NULL; // List of in-game players.

void addPlayer(int newfd)
{
	player *players_aux; 
	
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
	player *me = (player*) arg;
	
	printf("joana e a mariana é ok também.\n");
	
	pthread_exit(NULL);
}