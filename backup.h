#ifndef BACKUP_H
#define BACKUP_H

#include <sys/types.h>

#define START_MARKER 126
#define MESSAGE_CAPSULE_SIZE 4

#define DATA_MAX_LEN 63
#define BUFFER_MAX_LEN 67

#define TIMEOUT 1

typedef enum message_type_e {
    BACKUP_FILE = 0b0000,
    BACKUP_FILES = 0b0001,
    RETRIEVE_FILE = 0b0010,
    RETRIEVE_FILES = 0b0011,
    DEFINE_BACKUP_DIRECTORY = 0b0100,
    CHECK_BACKUP = 0b0101,
    RETRIEVE_FILES_FILE_NAME = 0b0110,
    MD5_FILE = 0b0111,
    DATA = 0b1000,
    END_FILE = 0b1001,
    END_FILES = 0b1010,
    RESET_SEQUENCE = 0b1011,
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
    unsigned char start_marker; // 8 bits
    unsigned char size;         // 6 bits
    unsigned char sequence;     // 6 bits
    message_type_e type;        // 4 bits
    unsigned char *data;        // 0-63 bytes
    unsigned char parity;       // 8 bits 
} message_t;

typedef struct backup_t {
    int socket;
    unsigned char sequence;

    message_t *send_message;
    message_t *recv_message;

    unsigned char* send_buffer;
    unsigned char* recv_buffer;
} backup_t;


int create_socket();

backup_t *create_backup();

void free_backup(backup_t *backup);

message_t* create_message();

void free_message(message_t *message);

void message_reset(message_t *message);

void make_reset_sequence_message(backup_t *backup);

void make_backup_file_message(backup_t *backup, char *path);

void make_end_file_message(backup_t *backup);

void make_data_message(
    backup_t *backup,
    unsigned char *data,
    unsigned data_size
);

void make_backup_directory_message(backup_t *backup, char *path);

void make_ack_message(message_t *message);

void make_nack_message(message_t *message);

void send_acknowledgement(backup_t *backup, int is_ack);

void update_sequence(backup_t *backup);

ssize_t send_message(backup_t *backup);

ssize_t receive_message(backup_t *backup);

int wait_acknowledgement(backup_t *backup);

int wait_ack_or_error(backup_t *backup); 

void message_to_buffer(message_t *message, unsigned char *buffer);

void buffer_to_message(unsigned char *buffer, message_t *message);

void send_file(backup_t *backup, char *path);

void receive_file(backup_t *backup, char *file_name);

void get_file_md5(unsigned char *out, char *file_name);

void set_message_parity(message_t *message);

int check_message_parity(message_t *message);

int check_parity(unsigned char *buffer, int buffer_size);

#endif

