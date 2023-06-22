#ifndef CLIENT_H
#define CLIENT_H

#include "backup.h"

typedef enum commands_enum {
    C_BACKUP,
    C_RETRIEVE,
    C_CHECK,
    C_DEFINE_BACKUP_DIRECTORY,
    C_CHANGE_DIRECTORY,
    C_LIST_DIRECTORY,
    C_HELP,
    C_EXIT,
    C_INVALID
} commands_enum;


void client_run(int loopback);

commands_enum parse_command(char *command, char *arg, int* arg_size);

void reset_server_sequence(backup_t *backup);

void client_backup(backup_t *backup, char *path);

void client_retrieve(backup_t *backup, char *file_name);

void client_check(backup_t *backup, char *file_name);

void client_define_backup_dir(backup_t *backup, char *dir);

void client_change_directory(char *dir);

void client_list_directory(char *dir);

void make_md5_message(backup_t *backup, char *file_name);

void client_help();

#endif

