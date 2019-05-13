#include "Server_library.h"

int argumentControl(int argc, char const *argv[]) {
    int dim_board;
    
    if(argc != 2) {
        printf("Unsupported number of arguments!\n");
        return -1;
    }

    dim_board = atoi(argv[1]);
    printf("Board size: %dx%d\n", dim_board, dim_board);
    
    return dim_board;
}