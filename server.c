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
    int sequence = 0;

    for (;;) {
        printf("[ETHBKP] Waiting message\n");

        ssize_t size = recv(socket, buffer, htons(BUFFER_MAX_LEN), 0);

        #if DEBUG
        print_buffer(buffer, BUFFER_MAX_LEN);
        #endif

        buffer_to_message(buffer, message);

        if (message->start_marker == START_MARKER) {
            if (check_parity(buffer, message->size + 4)) {
                if (message->sequence == sequence) {
                    printf("[ETHBKP] Message received\n");
                    print_message(message);
                    sequence++;
                    make_ack_message(message);
                    send_message(socket, message);
                }
            } else {
                make_nack_message(message);
                send_message(socket, message);
            }
        }
    }

    free(message);

    return;
}

