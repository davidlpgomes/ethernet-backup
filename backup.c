#include "backup.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ConexaoRawSocket.h"
#include "utils.h"


int create_socket() {
    int socket = ConexaoRawSocket("lo");

    return socket;
}

backup_t* create_backup() {
    backup_t* backup = malloc(sizeof(backup_t));
    test_alloc(backup, "backup");

    backup->socket = create_socket();
    backup->sequence = 0;

    backup->buffer = malloc(sizeof(unsigned char*) * BUFFER_MAX_LEN);
    test_alloc(backup->buffer, "backup buffer");

    backup->message = create_message();

    return backup;
}

void free_backup(backup_t* backup) {
    if (!backup)
        return;

    if (backup->buffer)
        free(backup->buffer);

    if (backup->message)
        free(backup->message);

    free(backup);

    return;
}

message_t* create_message() {
    message_t* message = (message_t *) malloc(sizeof(message_t));
    test_alloc(message, "message");

    message->start_marker = START_MARKER;

    return message;
}

void message_clear(message_t* message) {
    message->start_marker = START_MARKER;

    if (message->data)
        free(message->data);

    return;
}

void make_backup_message(message_t* message, char* file_name, int sequence) {
    if (message->data)
        free(message->data);

    int name_len = strlen(file_name);

    message->start_marker = START_MARKER;
    message->size = name_len;
    message->sequence = sequence;
    message->type = BACKUP_FILE;

    message->data = (unsigned char *) malloc(sizeof(unsigned char) * name_len);
    test_alloc(message->data, "message->data");

    memcpy(message->data, file_name, name_len);

    set_message_parity(message);

    return;
}

void make_ack_message(message_t* message) {
    if (message->data) free(message->data);

    message->start_marker = START_MARKER;
    message->size = 0;
    message->type = ACK;
    message->data = NULL;
}

void make_nack_message(message_t* message) {
    if (message->data) free(message->data);

    message->start_marker = START_MARKER;
    message->size = 0;
    message->type = ACK;
    message->data = NULL;
}

void destroy_message(message_t* message) {
    if (!message)
        return;

    if (message->data)
        free(message->data);

    free(message);

    return;
}

ssize_t send_message(int socket, message_t* message) {
    int buffer_size = message->size + 4;
    unsigned char* buffer = (unsigned char *) malloc(sizeof(unsigned char) * buffer_size);
    test_alloc(buffer, "buffer");


    message_to_buffer(message, buffer);

    #if DEBUG
    print_buffer(buffer, buffer_size);
    #endif

    ssize_t size = send(socket, buffer, htons(buffer_size), 0);

    free(buffer);

    return size;
}

void message_to_buffer(message_t* message, unsigned char* buffer) {
    int buffer_size = message->size + 4;

    buffer[0] = message->start_marker;
    buffer[1] = (message->size << 2) + (message->sequence >> 4);
    buffer[2] = message->type + (message->sequence << 4);

    memcpy(&buffer[3], message->data, message->size);

    buffer[buffer_size - 1] = message->parity;

    return buffer;
} 

void buffer_to_message(unsigned char* buffer, message_t* message) {
    message->start_marker = buffer[0];
    message->size = buffer[1] >> 2;
    message->sequence = ((buffer[1] & 0b11) << 4) + (buffer[2] >> 4);
    message->type = buffer[2] & 0b1111;
    message->data = (unsigned char *) malloc(sizeof(unsigned char) * message->size);
    test_alloc(message->data, "message->data");

    memcpy(message->data, &buffer[3], message->size);

    message->parity = buffer[message->size + 3];

    return message;
}

void get_file_md5() {
    return;
}

void set_message_parity(message_t* message) {
    int buffer_size = message->size + 4;
    unsigned char* buffer = (unsigned char *) malloc(sizeof(unsigned char) * buffer_size);

    message_to_buffer(message, buffer);

    unsigned char sum = buffer[1];

    for (int i = 2; i < buffer_size - 1; i++) {
        sum ^= buffer[i];
    }

    free(buffer);

    message->parity = sum;
}

int check_parity(unsigned char* buffer, int buffer_size) {
    unsigned char sum = buffer[1];
    for (int i = 2; i < buffer_size; i++) {
        sum ^= buffer[i];
    }

    return sum == 0;
}
