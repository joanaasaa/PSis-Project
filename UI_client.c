#include "ui_client_library.h"

int main(int argc, char const *argv[])
{
	pthread_t threadID_read, threadID_write;

	argumentControl(argc, argv);

	create_socket(argv[1]);

	pthread_create(&threadID_read, NULL, thread_read, NULL);
	pthread_create(&threadID_write, NULL, thread_write, NULL);

	pthread_join(threadID_read, NULL);
	pthread_join(threadID_write, NULL);

	close_board_windows(); // GRAPHICS

    return 0;
}