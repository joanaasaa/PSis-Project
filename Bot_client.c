#include "libraries.h"
#include "bot_client_library.h"
#include "graphics_library.h"
#include "board_library.h"

int main(int argc, char const *argv[])
{
	int n;
	char str[10];
	pthread_t threadID_read, threadID_write;

	argumentControl(argc, argv);

	create_socket(argv[1]);

	pthread_create(&threadID_read, NULL, thread_read, NULL);
	pthread_create(&threadID_write, NULL, thread_write, NULL);

	pthread_join(threadID_read, NULL);
	pthread_join(threadID_write, NULL);

	close_board_windows();

    return 0;
}