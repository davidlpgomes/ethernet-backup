#include "client.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "backup.h"
#include "utils.h"

void client_run(int loopback) {
    #ifdef DEBUG
    printf("[ETHBKP] Running as client...\n");
    #endif

    backup_t *backup = create_backup(loopback);
    reset_server_sequence(backup);

    char *command = malloc(sizeof(char) * STR_LEN);
    test_alloc(command, "command");

    char *arg = malloc(sizeof(char) * (DATA_MAX_LEN + 1));
    test_alloc(arg, "arg");

    int arg_size;

    char *cwd = malloc(sizeof(char) * PATH_MAX);
    test_alloc(cwd, "cwd");

    commands_enum cmd_type;

    for (;;) {
        getcwd(cwd, PATH_MAX);
        printf("[%s] >>> ", cwd);

        fgets(command, STR_LEN, stdin);
        command[strlen(command) - 1] = '\0';

        cmd_type = parse_command(command, arg, &arg_size);

        if (arg_size > DATA_MAX_LEN) {
            printf("Invalid argument length\n");
            continue;
        }

        if (cmd_type == C_EXIT) {
            printf("\nExiting, see ya!\n");
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
            case C_LIST_DIRECTORY:
                client_list_directory(arg);
                break;
            case C_CHECK:
                client_check(backup, arg);
                break;
            case C_HELP:
                client_help();
                break;
            case C_INVALID:
                printf("Invalid command\n");
                break;
            default:
                continue;
        }
    }

    free(cwd);
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

        arg[*arg_size] = '\0';
    } else {
        *arg_size = 0;
        arg[0] = '\0';
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
    else if (!strcmp(cmd, "ls"))
        cmd_type = C_LIST_DIRECTORY;
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

    backup_files(backup, path);

    return;
}

void client_retrieve(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: retrieve\n");
    #endif

    if (!backup || !file_name)
        return;

    char is_wildcard = 0;

    if (
        strchr(file_name, '?') ||
        strchr(file_name, '*') ||
        strchr(file_name, '[')
    )
        is_wildcard = 1;


    make_retrieve_file_message(backup, file_name, is_wildcard);
    if (send_message(backup) < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }

    if (backup->recv_message->type == ERROR) {
        printf("File not found on server\n");
        return;
    }

    if (is_wildcard)
        retrieve_files(backup);
    else
        retrieve_file(backup, file_name);

    return;
}

void client_check(backup_t *backup, char *file_name) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: check\n");
    #endif
    
    if (!backup || !file_name)
        return;

    if (access(file_name, F_OK) == -1) {
        printf("File %s does not exist\n", file_name);
        return;
    }

    unsigned char *out = malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
    test_alloc(out, "client check cache");

    get_file_md5(out, file_name);

    make_check_message(backup, file_name);
    send_message(backup);

    if (backup->recv_message->type == ERROR) {
        printf("File not found on server\n");
        free(out);
        return;
    }

    receive_message(backup);
    message_t *m = backup->recv_message;
    
    if (!strncmp((char *) out, (char *) m->data, m->size))
        printf("File's (%s) hashes are the SAME\n", file_name);
    else
        printf(
            "File's (%s) hashes are DIFFERENTE, the file is corrupted!\n",
            file_name
        );

    free(out);

    return;
}

void make_check_message(backup_t *backup, char *file_name) {
    if (!backup || !file_name)
        return;

    message_t *m = backup->send_message;
    message_reset(m);

    int name_len = strlen(file_name);

    m->size = name_len;
    m->sequence = backup->sequence;
    m->type = CHECK_BACKUP;

    m->data = malloc(sizeof(unsigned char) * name_len);
    test_alloc(m->data, "backup message file name");

    memcpy(m->data, file_name, name_len);

    set_message_parity(m);

    return;
}


void client_define_backup_dir(backup_t *backup, char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: define backup directory\n");
    #endif

    if (!backup || !dir)
        return;

    make_backup_directory_message(backup, dir);
    ssize_t size = send_message(backup);

    if (backup->recv_message->type == ERROR)
        printf("The directory does not exist on server\n");

    if (size < 0)
        printf("Error: %s\n", strerror(errno));

    return;
}

void client_change_directory(char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: change directory\n");
    #endif

    if (!dir) {
        printf("Error: invalid directory\n");
        return;
    }

    if (access(dir, F_OK)) {
        printf("Invalid directory");
        return;
    }

    int ret = chdir(dir);

    if (ret == -1)
        printf("Error on chdir: %s\n", strerror(errno));

    return;
}

void client_list_directory(char *dir) {
    #ifdef DEBUG
    printf("[ETHBKP] Command: list directory %s\n", dir);
    #endif

    char path[PATH_MAX];

    if (!strlen(dir))
        getcwd(path, PATH_MAX);
    else
        realpath(dir, path);

    printf("Directory's content %s:\n", path);

    DIR *dirstream;
    struct dirent *direntry;

    dirstream = opendir(path);

    if (!dirstream) {
        printf("Error: could not open directory %s\n", path);
        return;
    }

    direntry = readdir(dirstream);

    while (direntry) {
        switch (direntry->d_type) {
            case DT_UNKNOWN:
                printf("U - ");
                break;
            case DT_REG:
                printf("A - ");
                break;
            case DT_DIR:
                printf("D - ");
                break;
            default:
                printf("O - ");
        }

        printf("%s\n", direntry->d_name);

        direntry = readdir(dirstream);
    }

    closedir(dirstream);

    return;
}

void client_help() {
    #ifdef DEBUG
    printf("[ETHBKP] Command: help\n");
    #endif

    printf("Welcome to the Ethernet Backup!\n");
    printf("Commands:\n");
    printf("  backup, bkp, b <file>     Backups the file to the server\n");
    printf("  retrieve, rtv, r <file>   Retrieves file from server\n");
    printf("  check, chk, c <file>      Checks if the file's hashes match\n");
    printf("  dfd                       Defines the backup directory on server\n");
    printf("  cd                        Changes the local working directory\n");
    printf("  ls                        Lists the working directory's content\n");
    printf("  help                      Shows list of commands\n");
    printf("  exit                      Exit\n");

    return;
}

