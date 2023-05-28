#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "backup.h"
#include "utils.h"
#include "ConexaoRawSocket.h"


void server_run() {
    #ifdef DEBUG
    printf("[ETHBKP] Running as server...\n");
    #endif

    backup_t *backup = create_backup();
    message_t *m = backup->recv_message;
    ssize_t size;

    unsigned char sequence = 0;

    for (;;) {
        printf("- Waiting message\n");

        size = receive_message(backup);

        printf("[ETHBKP] Message received, size=%zi\n", size);

        sequence = (sequence + 1) % 64;
        backup->sequence = sequence;

        sleep(2);
    }

    free_backup(backup);

    return;
}

