#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "ConexaoRawSocket.h"
#include "utils.h"

void client_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as client...\n");
    unsigned char buffer[BUFFER_LEN];

    for (;;) {
        printf("Escreva a mensagem (%d inteiros): ", BUFFER_LEN);

        for (int i = 0; i < BUFFER_LEN; i++)
            scanf("%c ", &buffer[i]);

        printf("[ETHBKP] Sending message\n");

        message_t* message = make_message(13, 0, DATA);

        message->data = (unsigned char *) malloc(sizeof(unsigned char) * 13);
        memcpy(message->data, buffer, BUFFER_LEN);

        print_message(message);


        ssize_t size = send_message(socket, message);

        free(message->data);
        destroy_message(message);

        if (size < 0)
            printf("Error: %s\n", strerror(errno));

        printf("[ETHBKP] Message sent, size=%ld\n", size);
    }

    return;
}
