#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "backup.h"
#include "utils.h"

void client_run() {
    #ifdef DEBUG
    printf("[ETHBKP] Running as client...\n");
    #endif

    backup_t *backup = create_backup();
    reset_server_sequence(backup);

    char *command = malloc(sizeof(char) * STR_LEN);
    test_alloc(command, "command");

    char *arg = malloc(sizeof(char) * (DATA_MAX_LEN + 1));
    test_alloc(arg, "arg");

    int arg_size;

    commands_enum cmd_type;

    for (;;) {
        printf(">>> ");
        fgets(command, STR_LEN, stdin);

        cmd_type = parse_command(command, arg, &arg_size);

        if (arg_size > DATA_MAX_LEN) {
            printf("Tamanho do argumento inválido\n");
            continue;
        }

        if (cmd_type == C_EXIT) {
            printf("\nSaindo, até logo!\n");
            break;
        }

        switch (cmd_type) {
            case C_BACKUP:
                client_backup(backup, arg);
                break;
            case C_RETRIEVE:
                client_retrieve(backup, arg);
                break;
            case C_DEFINE_BACKUP_DIRECTORY:
                client_define_backup_dir(backup, arg);
                break;
            case C_CHANGE_DIRECTORY:
                client_change_directory(arg);
                break;
            case C_CHECK:
                client_check(backup, arg);
                break;
            case C_HELP:
                client_help();
                break;
            case C_INVALID:
                printf("Comando inválido\n");
                break;
            default:
                continue;
        }
    }

    free(arg);
    free(command);

    free_backup(backup);

    return;
}

commands_enum parse_command(char* command, char* arg, int* arg_size) {
    #ifdef DEBUG
    printf("[ETHBKP] Parsing command\n");
    #endif

    char *cmd = strtok(command, " ");
    char *aux = strtok(NULL, " ");

    if (aux) {
        *arg_size = strlen(aux);
        memcpy(arg, aux, min(*arg_size, DATA_MAX_LEN));

        arg[*arg_size - 1] = '\0';
    }

    commands_enum cmd_type = C_INVALID;

    if (
        !strcmp(cmd, "backup") ||
        !strcmp(cmd, "bkp") ||
        !strcmp(cmd, "b")
    )
        cmd_type = C_BACKUP;
    else if (
        !strcmp(cmd, "retrieve") ||
        !strcmp(cmd, "rtv") ||
        !strcmp(cmd, "r")
    )
        cmd_type = C_RETRIEVE;
    else if (
        !strcmp(cmd, "check") ||
        !strcmp(cmd, "chk") ||
        !strcmp(cmd, "c")
    )
        cmd_type = C_CHECK;
    else if (!strcmp(cmd, "dfd"))
        cmd_type = C_DEFINE_BACKUP_DIRECTORY;
    else if (!strcmp(cmd, "cd"))
        cmd_type = C_CHANGE_DIRECTORY;
    else if (!strcmp(cmd, "exit"))
        cmd_type = C_EXIT;
    else if (
        !strcmp(cmd, "help") ||
        !strcmp(cmd, "h")
    )
        cmd_type = C_HELP;

    return cmd_type;
}

void reset_server_sequence(backup_t *backup) {
    #ifdef DEBUG
    printf("[ETHBKP] Reseting server sequence\n");
    #endif

    make_reset_sequence_message(backup);
    ssize_t size = send_message(backup);

    if (size < 0) {
        printf("Error on sending reset_server_sequence message\n");
        return;
    }

    #ifdef DEBUG 
    printf("[ETHBKP] Message sent, size=%zi\n", size);
    #endif

    return;
}

void client_backup(backup_t* backup, char *path) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: backup\n");
    #endif

    if (access(path, F_OK)) {
        printf("Erro: arquivo não existe\n"); 
        return;
    }

    if (access(path, R_OK)) {
        printf("Erro: você não tem permissão para ler esse arquivo\n");
        return;
    }

    send_file(backup, path);

    return;
}

void client_retrieve(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: retrieve\n");
    #endif

    return;
}

void client_check(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: check\n");
    #endif

    return;
}

void client_define_backup_dir(backup_t *backup, char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: define backup directory\n");
    #endif

    return;
}

void client_change_directory(char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: change directory\n");
    #endif

    return;
}

void client_help() {
    #ifdef DEBUG
    printf("[ETHBKP] Command: help\n");
    #endif

    return;
}

