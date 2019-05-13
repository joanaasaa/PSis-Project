#include "player_management.h"

void *listenSocket(void *arg)
{
	int n;
	int fd, newfd;
	char str[20];
	struct sockaddr_in server_addr, client_addr;
	unsigned int client_addrlen;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	fd = socket(AF_INET, SOCK_STREAM, 0); 	// AF_INET: Socket for communication between diffrent machines;
											// SOCK_STREAM: Stream socket. Connection oriented.
	if (fd == -1) {
		perror("socket: ");
		exit(-1);
  	}
  	
	n = bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n == -1) {
		perror("bind: ");
		exit(-1);
 	}
  	printf("Socket created and binded!\n");
  	listen(fd, 5);

	// Server is listening for clients.

	newfd = accept(fd, (struct sockaddr *) &client_addr, &client_addrlen);

	read(newfd, str, sizeof(str));
	printf("%s\n", str);

	close(newfd);
	close(fd);

    pthread_exit(0);
}