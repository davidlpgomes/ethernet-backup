#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "utils.h"
#include "ConexaoRawSocket.h"


void server_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as server...\n");
    unsigned char buffer[BUFFER_MAX_LEN];

    message_t* message = create_message();

    for (;;) {
        printf("[ETHBKP] Waiting message\n");

        ssize_t size = recv(socket, buffer, htons(BUFFER_MAX_LEN), 0);

        #if DEBUG
        print_buffer(buffer, BUFFER_MAX_LEN);
        #endif

        buffer_to_message(buffer, message);

        if (message->start_marker == START_MARKER) {
            printf("[ETHBKP] Message received\n");
            print_message(message);
        }
    }

    free(message);

    return;
}

