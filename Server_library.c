#include "Server_library.h"

int argumentControl(int argc, char const *argv[]) {
    int board_size;
    
    if(argc > 2)
    {
        printf("Too many arguments!\n");
        return -1;
    }

    board_size = atoi(argv[1]);
    printf("Board size: %dx%d\n", board_size, board_size);
    
    return board_size;
}