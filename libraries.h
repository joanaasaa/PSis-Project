#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 3000
#define MAX_WINDOW_SIZE 1000