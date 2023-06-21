#include "server.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

    char *cwd = malloc(sizeof(char) * PATH_MAX);
    test_alloc(cwd, "cwd");

    for (;;) {
        getcwd(cwd, PATH_MAX);
        printf("[%s] Waiting message\n", cwd);

        size = receive_message(backup);

        switch (backup->recv_message->type) {
            case BACKUP_FILE:
                server_backup(backup, (char*) backup->recv_message->data);
                break;
            case BACKUP_FILES:
                server_backup_files(backup, *backup->recv_message->data);
                break;
            case DEFINE_BACKUP_DIRECTORY:
                server_define_backup_directory(
                    backup,
                    (char*) backup->recv_message->data
                );
                break;
            default:
                break;
        }
    }

    free_backup(backup);

    return;
}

void make_error_message(backup_t *backup, eth_error_e error) {
    if (!backup || !error)
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

void server_backup_files(backup_t *backup, unsigned num_files) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: backup files\n");
    #endif

    if (!backup || !num_files)
        return;

    receive_files(backup, num_files);

    return;
}

void server_define_backup_directory(backup_t *backup, char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: define backup directory\n");
    #endif

    if (!backup || !dir)
        return;

    printf("Changing directory to %s\n", dir);

    int ret = chdir(dir);

    if (ret == -1)
        printf("Erro: %s\n", strerror(errno));

    return;
}

