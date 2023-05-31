#ifndef SERVER_H
#define SERVER_H

#include "backup.h"

void server_run();

void server_backup(backup_t *backup, char *file_name);

#endif

