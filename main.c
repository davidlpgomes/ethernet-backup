#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "ConexaoRawSocket.h"

#define BUFFER_LEN 8

int main(int argc, char** argv) {
    int socket = ConexaoRawSocket("lo");
    
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
   
    if (run_mode) {
        // RUN AS SERVER
        printf("[ETHBKP] Running as server...\n");
        unsigned buffer[BUFFER_LEN];
        while (1) {
            printf("[ETHBKP] Waiting message\n");
            ssize_t size = recv(socket, buffer, BUFFER_LEN, 0);
            printf("[ETHBKP] Message received\n");
            for (int i = 0; i < BUFFER_LEN; i++)
                printf("%u", buffer[i]);
            printf("\n");
        }
    } else {
        // RUN AS CLIENT
        printf("[ETHBKP] Running as client...\n");
        unsigned buffer[BUFFER_LEN];
        while (1) {
            printf("Escreva a mensagem (%d inteiros): ", BUFFER_LEN);
            for (int i = 0; i < BUFFER_LEN; i++)
                scanf("%u", &buffer[i]);
            printf("[ETHBKP] Sending message\n");
            ssize_t size = send(socket, buffer, BUFFER_LEN, 0);
            printf("[ETHBKP] Message sent\n");
        }
    }


    return 0;
}
