#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// SDL:
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
// Sockets:
#include <sys/socket.h>
#include <arpa/inet.h>
// Flag control:
#include <fcntl.h> // To set file descriptor flags.
#include <errno.h> // For error checking.
// Threads:
#include <pthread.h>
// Constants:
#define SERVER_PORT 3000
#define MAX_WINDOW_SIZE 1000