#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"
#include "server.h"


int main(int argc, char** argv) {
    // Disable buffer for stdout
    setvbuf(stdout, 0, _IONBF, 0);

    int run_mode = -1; // 0 -> CLIENT, 1 -> SERVER
    int loopback = 0;
    
    int opt;
    while ((opt = getopt(argc, argv, "csl")) != -1) {
        switch(opt) {
            case 'c':
                run_mode = 0;
                break;
            case 's':
                run_mode = 1;
                break;
            case 'l':
                loopback = 1;
            default:
                fprintf(stderr, "Usage: %s [-c] [-s]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (run_mode < 0) {
        fprintf(stderr, "Usage: %s [-c] [-s]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
   
    if (run_mode)
        server_run(loopback);
    else
        client_run(loopback);

    return 0;
}
