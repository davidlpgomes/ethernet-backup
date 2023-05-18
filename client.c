#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "ConexaoRawSocket.h"


void client_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as client...\n");
    unsigned buffer[BUFFER_LEN];

    for (;;) {
        printf("Escreva a mensagem (%d inteiros): ", BUFFER_LEN);

        for (int i = 0; i < BUFFER_LEN; i++)
            scanf("%u", &buffer[i]);

        printf("[ETHBKP] Sending message\n");

        ssize_t size = send(socket, buffer, BUFFER_LEN, 0);

        printf("[ETHBKP] Message sent\n");
    }

    return;
}
