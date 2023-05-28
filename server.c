#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "backup.h"
#include "utils.h"
#include "ConexaoRawSocket.h"


void server_run() {
    #ifdef DEBUG
    printf("[ETHBKP] Running as server...\n");
    #endif

    backup_t *backup = create_backup();
    message_t *m = backup->message;
    ssize_t size;

    unsigned char sequence = 0;

    for (;;) {
        printf("- Waiting message\n");

        size = receive_message(backup);

        return;

        if (m->start_marker != START_MARKER)
            continue;

        if (check_parity_message(m)) {
            if (m->sequence != sequence)
                printf("ERROR: SEQUENCE IS DIFFERENT\n");

            printf("[ETHBKP] Message received\n");
            print_message(m);

            sequence = (sequence + 1) % 64;

            make_ack_message(m);
            send_message(backup);
        } else {
            make_nack_message(m);
            send_message(backup);
        }
    }

    free_backup(backup);

    return;
}

