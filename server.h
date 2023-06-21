#ifndef SERVER_H
#define SERVER_H

#include "backup.h"

void server_run();

void server_backup(backup_t *backup, char *file_name);

void server_backup_files(backup_t *backup, unsigned num_files);

void server_define_backup_directory(backup_t *backup, char *dir);

#endif

