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

    for (;;) {
        printf("- Waiting message\n");

        size = receive_message(backup);
        printf("[ETHBKP] Message received, size=%zi\n", size);

        switch (backup->recv_message->type) {
            default:
                break;
        }
    }

    free_backup(backup);

    return;
}

