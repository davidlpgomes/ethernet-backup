#ifndef BACKUP_H
#define BACKUP_H

#include <sys/types.h>

#define START_MARKER 126
#define BUFFER_LEN 13
#define BUFFER_MAX_LEN 64

typedef enum message_type_e {
    BACKUP_FILE = 0b0000,
    BACKUP_FILES = 0b0001,
    RETRIEVE_FILE = 0b0010,
    RETRIEVE_FILES = 0b0011,
    DEFINE_BACKUP_DIRECTORY = 0b0100,
    CHECK_BACKUP = 0b0101,
    RETRIEVE_FILES_FILE_NAME = 0b0110,
    MD5 = 0b0111,
    DATA = 0b1000,
    END_FILE = 0b1001,
    END_FILES = 0b1010,
    NOT_USED = 0b1011,
    ERROR = 0b1100,
    OK = 0b1101,
    ACK = 0b1110,
    NACK = 0b1111
} message_type_e;

typedef enum eth_error_e {
    DISK_FULL,
    NO_WRITE_PERMISSION,
    FILE_DOES_NOT_EXIST,
    NO_READ_PERMISSION
} eth_error_e;

typedef struct message_t {
    unsigned char start_marker;
    unsigned char size;
    unsigned char sequence;
    message_type_e type;
    unsigned char* data;
    unsigned char parity;
} message_t;

message_t* make_message(unsigned char size, unsigned char sequence, message_type_e type);

void destroy_message(message_t* message);

ssize_t send_message(int socket, message_t* message);

unsigned char* message_to_buffer(message_t* message);

message_t* buffer_to_message(unsigned char* buffer);

void get_file_md5();

void set_message_parity(message_t* message);

#endif

