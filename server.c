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

        switch (backup->recv_message->type) {
            case BACKUP_FILE:
                server_backup(backup, (char*) backup->recv_message->data);
                break;
            default:
                break;
        }
    }

    free_backup(backup);

    return;
}

void make_error_message(backup_t *backup, eth_error_e error) {
    if (!backup || !data)
        return;

    message_t *m = backup->send_message;
    message_reset(m);

    m->size = 1;
    m->sequence = backup->sequence;
    m->type = ERROR;

    m->data = malloc(sizeof(unsigned char));
    test_alloc(m->data, "backup message data");

    m->data[0] = error;

    set_message_parity(m);

    return;
}

void server_backup(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: backup\n");
    #endif

    if (!backup || !file_name)
        return;

    //TODO: Check permissions

    receive_file(backup, file_name);

    return;
}

