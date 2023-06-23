#include "server.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/md5.h>

#include "backup.h"
#include "utils.h"
#include "ConexaoRawSocket.h"


void server_run(int loopback) {
    #ifdef DEBUG
    printf("[ETHBKP] Running as server...\n");
    #endif

    backup_t *backup = create_backup(loopback);
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
            case RETRIEVE_FILE:
                server_retrieve(backup, (char*) backup->recv_message->data);
                break;
            case RETRIEVE_FILES:
                server_retrieve_files(
                    backup,
                    (char*) backup->recv_message->data
                );
                break;
            case DEFINE_BACKUP_DIRECTORY:
                server_define_backup_directory(
                    backup,
                    (char*) backup->recv_message->data
                );
                break;
            case MD5_FILE:
                server_send_md5(backup, (char *) backup->recv_message->data);
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

    receive_file(backup, file_name, backup->recv_message->size);

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

void server_retrieve(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: retrieve\n");
    #endif

    if (!backup || !file_name)
        return;

    printf("Enviando arquivo: %s\n", file_name);
    send_file(backup, file_name);

    return;
}

void server_retrieve_files(backup_t *backup, char *pattern) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: retrieve files\n");
    #endif

    if (!backup || !pattern)
        return;

    glob_t globe;

    printf("Buscando arquivos com o padrão %s\n", pattern);

    int ret = glob(pattern, GLOB_ERR | GLOB_TILDE, NULL, &globe);

    if (ret) {
        if (ret == GLOB_NOMATCH)
            printf("Nenhum arquivo encontrado\n");
        else
            printf("Erro: glob %s\n", strerror(errno));

        return;
    }

    ssize_t size;

    size_t files = globe.gl_pathc;
    char **file = globe.gl_pathv;

    while (*file) {
        printf("Enviando arquivo %s...\n", *file);

        make_retrieve_file_name_message(backup, *file);
        size = send_message(backup);

        if (size < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            return;
        }

        send_file(backup, *file);

        file++;
    }

    make_end_files_message(backup);
    size = send_message(backup);

    if (size < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return;
    }

    globfree(&globe);

    return;
}

void server_define_backup_directory(backup_t *backup, char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: define backup directory\n");
    #endif

    if (!backup || !dir)
        return;

    int size = backup->recv_message->size;

    char *d = malloc(sizeof(char) * size + 1);
    test_alloc(d, "server_define_backup_directory dir name");

    d[size] = '\0';
    memcpy(d, dir, size);

    printf("Changing directory to %s\n", d);

    int ret = chdir(d);

    free(d);

    if (ret == -1)
        printf("Erro: %s\n", strerror(errno));

    return;
}

void server_send_md5(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: get file md5\n");
    #endif

    if (!backup || !file_name)
        return;

    int size = backup->recv_message->size;

    char *f = malloc(sizeof(char) * size + 1);
    test_alloc(f, "server_send_md5 file name");

    f[size] = '\0';
    memcpy(f, file_name, size);

    printf("Generating md5 for %s\n", f);

    unsigned char *out = malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
    test_alloc(out, "server md5 cache");

    get_file_md5(out, f);

    make_md5_message(backup, out);
    send_message(backup);

    free(out);
    free(f);

    return;
}

void make_md5_message(backup_t *backup, unsigned char *md5_str) {
    if (!backup || !md5_str)
        return;

    message_t *m = backup->send_message;
    message_reset(m);

    m->size = MD5_DIGEST_LENGTH;
    m->sequence = backup->sequence;
    m->type = MD5_FILE;

    m->data = malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
    test_alloc(m->data, "md5 response");

    memcpy(m->data, md5_str, MD5_DIGEST_LENGTH);

    set_message_parity(m);

    return;

}
