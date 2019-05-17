#include "libraries.h"
#include "client_library.h"
#include "graphics_library.h"
#include "UI_library.h"

player_self me; // Player's info.
card *board; // Game board stored as an allocated vector.

void *thread_read(void *arg){

	int n;
	int fd = (int) arg;
	char str[24];

	printf("fd: %d\n", fd);

	while(1) {
		n = read(fd, &str, sizeof(str)); // Recebe uma string do servidor.
		printf("read %d: %s\n", n, str);
		memset(str, 0, sizeof(str));
	}
	

	//PEGAR NAS VARIAS INFORMAÇOES DA STRING.
}

void *thread_write(void *arg){
	//ENVIAR JOGADAS

	int fd = (int) arg;
	char str[24];

	while(1) {
		write(fd, str, strlen(str)); // Recebe uma string do servidor.
	}
}

int main(int argc, char const *argv[])
{
	int n;
	char str[10];
	struct sockaddr_in server_addr;
	pthread_t threadID_read, threadID_write;

	if(argc < 2){
    	printf("second argument should be server address\n");
    	exit(-1);
  	}
  	
  	int fd = socket(AF_INET, SOCK_STREAM, 0);

  	if(fd == -1){
    	perror("socket: ");
    	exit(-1);
  	}

  	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	
	inet_aton(argv[1], &server_addr.sin_addr); //argv[1] => SERVER_IP

	n = connect(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n == -1){
		perror("connect: ");
		exit(-1);
	}

	printf("fd: %d\n", fd);

	pthread_create(&threadID_read, NULL, thread_read, (int*) fd);
	pthread_create(&threadID_write, NULL, thread_write, (int*) fd);

	pthread_join(threadID_read, NULL);
	pthread_join(threadID_write, NULL);

	// close(fd);

    return 0;
}