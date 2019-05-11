#include "board_library.h"

int dim_board;
board_place * board;
int play1[2];
int n_corrects;

int linear_conv(int i, int j) {
	return j*dim_board+i;
}
char *get_board_place_str(int i, int j) {
	return board[linear_conv(i, j)].v;
}

void init_board(int dim) {
	int count  = 0;
	int i, j;
	char *str_place;

	dim_board= dim;
	n_corrects = 0;
	play1[0]= -1;
	board = malloc(sizeof(board_place)* dim * dim); // Cria espaço na memória para guardar strings para todos os lugares do tabuleiro.

	for(i=0; i < (dim_board * dim_board); i++) { // Limpa o conteúdo de todas as strings.
	board[i].v[0] = '\0';
	}

	for(char c1 = 'a' ; c1 < ('a'+ dim_board); c1++) {
		for(char c2 = 'a' ; c2 < ('a'+ dim_board); c2++) {
			// Procuram-se lugares no tabuleiro que ainda não tenham string associada.
			do {
			// Escolhe um lugar do tabuleiro, aleatoriamente.
			i = random() % dim_board;
			j = random() % dim_board;

			// Com a localização, no tabuleiro, desse lugar, calula o número correspondente no vector board e devolve a respectiva string.
			str_place = get_board_place_str(i, j);
			printf("1) %d %d -%s-\n", i, j, str_place);
			} while(str_place[0] != '\0'); // Enquanto os lugares no tabuleiro já tiverem uma string associada.

			// Associa-se uma string ao lugar encontrado (que anterioremente não tinha string).
			str_place[0] = c1;
			str_place[1] = c2;
			str_place[2] = '\0';

			// Procura-se outro lugar no tabuleiro que ainda não tenha string, para ser o par.
			do {
			i = random()% dim_board;
			j = random()% dim_board;
			str_place = get_board_place_str(i, j);
			printf("2) %d %d -%s-\n", i, j, str_place);
			} while(str_place[0] != '\0');

			// Atribui a mesma string a esse lugar.
			str_place[0] = c1;
			str_place[1] = c2;
			str_place[2] = '\0';

			count += 2;
			if(count == dim_board*dim_board) return; // Se já todas as cartas têm valores atribuídos, retorna.
		}
	}
}

play_response board_play(int x, int y){
	play_response resp;
	resp.code =10;
	if(strcmp(get_board_place_str(x, y), "")==0) {
		printf("FILLED\n");
    	resp.code =0;
	} 
  	else {
    	if(play1[0]== -1) {
        	printf("FIRST\n");
        	resp.code =1;

			play1[0]=x;
			play1[1]=y;
			resp.play1[0]= play1[0];
			resp.play1[1]= play1[1];
			strcpy(resp.str_play1, get_board_place_str(x, y));
      	}
		else {
			char * first_str = get_board_place_str(play1[0], play1[1]);
			char * secnd_str = get_board_place_str(x, y);

        	if( (play1[0]==x) && (play1[1]==y) ) {
          		resp.code =0;
          		printf("FILLED\n");
        	} 
			else {
          		resp.play1[0]= play1[0];
          		resp.play1[1]= play1[1];
          		strcpy(resp.str_play1, first_str);
          		resp.play2[0]= x;
          		resp.play2[1]= y;
          		strcpy(resp.str_play2, secnd_str);

          		if (strcmp(first_str, secnd_str) == 0) {
            		printf("CORRECT!!!\n");


            		strcpy(first_str, "");
            		strcpy(secnd_str, "");

            		n_corrects +=2;
            		if(n_corrects == dim_board* dim_board) resp.code =3;
            		else resp.code =2;
          		}
				else {
            		printf("INCORRECT");

            		resp.code = -2;
          		}
          		play1[0]= -1;
        	}
      	}
    }

	return resp;
}
