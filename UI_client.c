#include "libraries.h"
#include "board_library.h"

card *board; // Tabuleiro do jogador.

void *thread_read(void *arg){

	char str[24];

	read(sock_fd, &str, sizeof(str)); // Recebe uma string do servidor.

	//PEGAR NAS VARIAS INFORMAÇOES DA STRING.
}

void *thread_write(void *arg){
	//ENVIAR JOGADAS
}

int main(int argc, char const *argv[])
{
	int n;
	char str[10];
	
	strcpy(str, "ola");

	struct sockaddr_in server_addr;
	if (argc <2){
    	printf("second argument should be server address\n");
    	exit(-1);
  	}
  	
  	int sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  	if (sock_fd == -1){
    	perror("socket: ");
    	exit(-1);
  	}

  	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	
	inet_aton(argv[1], &server_addr.sin_addr); //argv[1] => SERVER_IP

	n = connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n == -1){
		perror("connect: ");
		exit(-1);
	}

	pthread_create(&(players_aux->threadID), NULL, thread_read, NULL);
	pthread_create(&(players_aux->threadID), NULL, thread_write, NULL);

	close(sock_fd);

    return 0;
}