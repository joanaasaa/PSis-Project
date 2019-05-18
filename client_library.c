#include "client_library.h"

player_self me; // Player's info.
card *board; // Game board stored as an allocated vector.

void set_variables(char str){


	if(strcmp(final_msg, 'start\n') == 0){ // Start.
		//COMEÇA A JOGAR.
	}
	else if(strcmp(final_msg, 'winner\n') == 0){

	}
	else if(final_msg[0] == 'u' || final_msg[0] == 'd' || final_msg[0] == 'l'){ // Information about the board.
		while()
	}
	else{

	}
}

