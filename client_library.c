#include "client_library.h"

int terminate = 0;
int game = 0;
int fd;
int dim_board = 0;
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

	int flags = fcntl(fd, F_GETFL, 0);
  	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return;
}

void interpret_final_msg(char final_msg[]) {

	if(game == 0) {
		if(sscanf(final_msg, "%d-%d-%d-%d-%d\n", &dim_board, &game, &(me.rgb_R), &(me.rgb_G), &(me.rgb_B)) == 5) {
			if(game == 0) { // There+s only one player connected. We have to wait for another one to join.
				game = 2; // The player has to wait for a "start\n" message.
				printf("Waiting for a second player...");
				return;
			}

			// PRINT INITIAL BOARD ON SCREEN.

		}
		else return;
	}
	else if (game == 2) {
		if(strcmp(final_msg, "start\n") == 0) {
			game = 1;
			printf("Start palying!");

			// PRINT INITIAL BOARD ON SCREEN.

			//--------------------------------------------------------------------------
			//(Para testar o server):
			int x=2, y=3;
			char str[30];
			sprintf(str, "%d-%d-%d-%d-%d\n", x, y, me.rgb_R, me.rgb_G, me.rgb_B);
			printf("str: %s\n", str);
			write(fd, str, strlen(str));
			//--------------------------------------------------------------------------

		}
		else return;
	}

	// The function only gets to here if the game has already started?

}

void *thread_read(void *arg) 
{
	int n;
	char str[100];
	char *res_aux, buffer[200], final_msg[25], res[100];

	while(!terminate) {
		memset(str, 0, sizeof(str));

		n = read(fd, &str, sizeof(str)); // Recebe uma string do servidor.
		if(n<=0) { // If there was an error reading.
			if( (errno == EAGAIN || errno == EWOULDBLOCK) && (n==-1) ) 
				continue;
			else {
				terminate = 1;
				close(fd);
				break;
			}
		}
		else {
			printf("read %d: %s\n", n, str);
			
			strcat(buffer, str);

			res_aux = strstr(buffer, "\n"); // res_aux is pointing to "\n"'s first occurance in buffer.
			while(res_aux != NULL) { // While there's a message to be read stored in buffer.
				res_aux++;
				memset(res, 0, sizeof(res));
				strcpy(res, res_aux);

				memset(final_msg, 0, sizeof(final_msg));
				int i=0;
				for(i=0; buffer[i] != '\n'; i++) {
					final_msg[i] = buffer[i];
				}
				final_msg[i] = '\n';
				final_msg[i+1] = '\0';

				memset(buffer, 0, sizeof(buffer));
				strcpy(buffer, res);

				interpret_final_msg(final_msg);

				res_aux = strstr(buffer, "\n");
			}
		}
	}

	pthread_exit(0);
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
