#include "libraries.h"
#include "board_library.h"

int main(int argc, char const *argv[])
{
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
  	
  	if( -1 == connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr))){
		printf("Error connecting\n");
		exit(-1);
	}

	write(sock_fd, &str, strlen(str));

	//read(sock_fd, &play_char, sizeof(play_char)); //PÔR AQUI A STRING QUE ELE RECEBE!

	close(sock_fd);

    return 0;
}