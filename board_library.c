#include "board_library.h"

int dim_board;
card *board; // Vector alocado onde se encontra guardado o conteúdo de todas as cartas.
int play1[2]; // Vector onde se guardam os índices da primeira carta escolhida. play1[0]=-1 se ainda não foi escolhida nenhuma carta. 
int n_corrects; // Guarda o número de cartas que já estão definitivamente viradas para cima.
pthread_mutex_t lock_board = PTHREAD_MUTEX_INITIALIZER;

void init_mutex()
{
	int n;
	
	n = pthread_mutex_init(&lock_board, NULL);
	if(n != 0) { 
        printf("Mutex initialization failed!\n"); 
        exit(-1); 
    }
}

int linear_conv(int i, int j) {
	return j*dim_board+i;
}

void set_card_traits(int i, int j, int status, int r, int g, int b) {
	
	pthread_mutex_lock(&lock_board);
	
	board[linear_conv(i, j)].status = status;
	board[linear_conv(i, j)].rgb_R = r;
	board[linear_conv(i, j)].rgb_G = g;
	board[linear_conv(i, j)].rgb_B = b;

	pthread_mutex_unlock(&lock_board);
}

void set_pair_traits(int i1, int j1, int i2, int j2, int status, int r, int g, int b) {
	
	pthread_mutex_lock(&lock_board);
	
	board[linear_conv(i1, j1)].status = status;
	board[linear_conv(i1, j1)].rgb_R = r;
	board[linear_conv(i1, j1)].rgb_G = g;
	board[linear_conv(i1, j1)].rgb_B = b;

	board[linear_conv(i2, j2)].status = status;
	board[linear_conv(i2, j2)].rgb_R = r;
	board[linear_conv(i2, j2)].rgb_G = g;
	board[linear_conv(i2, j2)].rgb_B = b;

	pthread_mutex_unlock(&lock_board);
}

char *get_card_str(int i, int j) {
	return board[linear_conv(i, j)].v;
}

char get_card_status(int i, int j) {
	char status;

	pthread_mutex_lock(&lock_board);

	status = board[linear_conv(i, j)].status;

	pthread_mutex_unlock(&lock_board);

	return status;
}

void clear_board(){
	free(board);
	board = NULL;
	destroy_mutex();
}

void init_board(int dim) {
	int count  = 0;
	int i, j;
	char *str_place;

	srand(time(NULL));

	dim_board= dim;
	n_corrects = 0;
	play1[0]= -1;
	board = malloc(sizeof(card) * dim * dim); // Cria espaço na memória para guardar strings para todos os lugares do tabuleiro.

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

				// Com a localização no tabuleiro, desse lugar, calula o número correspondente no vector board e devolve a respectiva string.
				str_place = get_card_str(i, j);
				printf("1)	%d	%d	-%s-\n", i, j, str_place);
			} while(str_place[0] != '\0'); // Enquanto os lugares no tabuleiro já tiverem uma string associada.

			// Associa-se uma string ao lugar encontrado (que anterioremente não tinha string).
			/*
			str_place[0] = c1;
			str_place[1] = c2;
			str_place[2] = '\0';
			*/
			board[linear_conv(i,j)].v[0] = c1;
			board[linear_conv(i,j)].v[1] = c2;
			board[linear_conv(i,j)].v[2] = '\0';
			board[linear_conv(i,j)].status = 'd';

			// Procura-se outro lugar no tabuleiro que ainda não tenha string, para ser o par.
			do {
				i = random()% dim_board;
				j = random()% dim_board;
				str_place = get_card_str(i, j);
				printf("2)	%d	%d	-%s-\n", i, j, str_place);
			} while(str_place[0] != '\0');

			// Atribui a mesma string a esse lugar.
			/*
			str_place[0] = c1;
			str_place[1] = c2;
			str_place[2] = '\0';
			*/
			board[linear_conv(i,j)].v[0] = c1;
			board[linear_conv(i,j)].v[1] = c2;
			board[linear_conv(i,j)].v[2] = '\0';
			board[linear_conv(i,j)].status = 'd';

			count += 2;
			if(count == dim_board*dim_board) {
				for(i=0; i < (dim_board * dim_board); i++) { // Imprime o conteúdo de todas as strings.
					printf("carta %d: %s - %c\n", i, board[i].v, board[i].status);
				}
				return; // Se já todas as cartas têm valores atribuídos, retorna.
			}
			
		}
	}

	init_mutex();	
}

char *get_str2send(int i, int j) 
{
	int code = 15;
	char *str_send = NULL;

	str_send = (char*)malloc(sizeof(char) * 100);
	sprintf(str_send, "%d-%c%c-%c-%d-%d-%d-%d-%d\n", code,  board[linear_conv(i,j)].v[0], 
															board[linear_conv(i,j)].v[1], 
															board[linear_conv(i,j)].status, i, j, 
															board[linear_conv(i,j)].rgb_R, 
															board[linear_conv(i,j)].rgb_G, 
															board[linear_conv(i,j)].rgb_B);	

	return str_send;
}

void destroy_mutex() 
{
	pthread_mutex_destroy(&lock_board);
}