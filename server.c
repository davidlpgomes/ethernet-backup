#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ConexaoRawSocket.h"


void server_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as server...\n");
    unsigned char buffer[BUFFER_MAX_LEN];

    for (;;) {
        printf("[ETHBKP] Waiting message\n");

        ssize_t size = recv(socket, buffer, htons(BUFFER_MAX_LEN), 0);

        message_t* message = buffer_to_message(buffer);

        printf("[ETHBKP] Message received\n");

        for (int i = 0; i < message->size; i++)
            printf("%c ", message->data[i]);

        printf("\n");
    }

    return;
}

