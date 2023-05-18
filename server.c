#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "ConexaoRawSocket.h"


void server_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as server...\n");
    char buffer[BUFFER_LEN];

    for (;;) {
        printf("[ETHBKP] Waiting message\n");

        ssize_t size = recv(socket, buffer, BUFFER_LEN, 0);

        printf("[ETHBKP] Message received\n");

        for (int i = 0; i < BUFFER_LEN; i++)
            printf("%c ", buffer[i]);

        printf("\n");
    }

    return;
}

