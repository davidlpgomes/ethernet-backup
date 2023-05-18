#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "ConexaoRawSocket.h"

void client_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as client...\n");
    char buffer[BUFFER_LEN];

    for (;;) {
        printf("Escreva a mensagem (%d inteiros): ", BUFFER_LEN);

        for (int i = 0; i < BUFFER_LEN; i++)
            scanf("%c ", &buffer[i]);

        printf("[ETHBKP] Sending message\n");

        ssize_t size = send(socket, buffer, htons(BUFFER_LEN), 0);

        if (size < 0)
            printf("Error: %s\n", strerror(errno));

        printf("[ETHBKP] Message sent, size=%ld\n", size);
    }

    return;
}
