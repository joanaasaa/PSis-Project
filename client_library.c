#include "client_library.h"

int terminate = 0;
int fd;
player_self me; // Player's info.
//card *board; // Game board stored as an allocated vector.

void argumentControl(int argc, char const *argv[]) {
    
    char str[20];
	
	if(argc < 2){
    	printf("Second argument has to be the server's IP address!\n");
    	exit(-1);
  	}
	if(argc > 2){
    	printf("Unsupported number of arguments!\n");
    	exit(-1);
  	}

	return;
}

void create_socket(const char *server_ip)
{
	int n;
	struct sockaddr_in server_addr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
  	if(fd<0){
    	perror("socket");
    	exit(-1);
  	}

  	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	
	inet_aton(server_ip, &server_addr.sin_addr); //argv[1] => SERVER_IP

	n = connect(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
  	if(n == -1){
		perror("connect");
		exit(-1);
	}

	printf("fd: %d\n", fd);

	// fcntl(fd, F_SETFL, O_NONBLOCK); // Defines fd as a non-blocking socket.
}

void set_variables(char str){

	/*
	if(strcmp(final_msg, "start\n") == 0){ // Start.
		//COMEÇA A JOGAR.
	}
	else if(strcmp(final_msg, "winner\n") == 0){

	}
	else if(final_msg[0] == 'u' || final_msg[0] == 'd' || final_msg[0] == 'l'){ // Information about the board.
		
	}
	else{

	}
	*/
}

void *thread_read(void *arg) 
{
	int n;
	//int fd = (int) arg;
	char str[100];
	char buffer[200], final_msg[25], res[100];
	char *res_aux;

	printf("fd: %d\n", fd);

	while(1) {
		n = read(fd, &str, sizeof(str)); // Recebe uma string do servidor.
		if(n<=0) { // If there was an error reading.
			// if(errno == EAGAIN || errno == EWOULDBLOCK) 
			// 	continue;
			// else {
			// 	terminate = 1;
			// }
		}
		printf("read %d: %s\n", n, str);
		memset(str, 0, sizeof(str));

		// strcat(buffer, str);

		// res_aux = strstr(buffer, "\n");
		// while(res_aux != NULL){
		// 	res_aux++;
		// 	strcpy(res, res_aux);

		// 	int i = 0;
		// 	while(buffer[i] != '\n'){
		// 		final_msg[i] = buffer[i];
		// 		i++;
		// 	}

		// 	final_msg[i] = '\n';
		// 	final_msg[i+1] = '\0';

		// 	memset(buffer, 0, sizeof(buffer));
		// 	strcpy(buffer, res);
		// 	res_aux = strstr(buffer, "\n");
		// }


	}
	
	
}

void *thread_write(void *arg)
{
	//int fd = (int) arg;
	char str[24];

	while(!terminate) {
		write(fd, str, strlen(str)); // Recebe uma string do servidor.
	}

	pthread_exit(0);
}

