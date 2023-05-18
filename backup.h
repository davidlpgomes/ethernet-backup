#ifndef BACKUP_H
#define BACKUP_H

#define START_MARKER 126

typedef enum message_type_e {
    BACKUP_FILE,
    BACKUP_FILES,
    RETRIEVE_FILE,
    RETRIEVE_FILES,
    DEFINE_BACKUP_DIRECTORY,
    CHECK_BACKUP,
    RETRIEVE_FILES_FILE_NAME,
    MD5,
    DATA,
    END_FILE,
    END_FILES,
    NOT_USED,
    ERROR,
    OK,
    ACK,
    NACK
} message_type_e;

typedef enum eth_error_e {
    DISK_FULL,
    NO_WRITE_PERMISSION,
    FILE_DOES_NOT_EXIST,
    NO_READ_PERMISSION
} eth_error_e;

typedef struct message_t {
    char start_marker;
    short size;
    unsigned sequence;
    message_type_e type;
    unsigned* data;
    unsigned parity;
} message_t;

void send_message(message_t* message);

void get_file_md5();

void set_message_parity(message_t* message);

#endif

