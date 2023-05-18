#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"
#include "server.h"


int main(int argc, char** argv) {
    int run_mode = -1; // 0 -> CLIENT, 1 -> SERVER
    
    int opt;
    while ((opt = getopt(argc, argv, "cs")) != -1) {
        switch(opt) {
            case 'c':
                run_mode = 0;
                break;
            case 's':
                run_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-c] [-s]", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (run_mode < 0) {
        fprintf(stderr, "Usage: %s [-c] [-s]", argv[0]);
        exit(EXIT_FAILURE);
    }
   
    if (run_mode)
        server_run();
    else
        client_run();

    return 0;
}
