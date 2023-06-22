#ifndef SERVER_H
#define SERVER_H

#include "backup.h"

void server_run();

void server_backup(backup_t *backup, char *file_name);

void server_backup_files(backup_t *backup, unsigned num_files);

void server_retrieve(backup_t *backup, char *file_name);

void server_retrieve_files(backup_t *backup, char *pattern);

void server_define_backup_directory(backup_t *backup, char *dir);

void server_send_md5(backup_t *backup, char *file_name);

void make_md5_response(backup_t *backup, unsigned char *md5_str);

#endif

