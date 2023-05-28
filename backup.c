#include "backup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ConexaoRawSocket.h"
#include "utils.h"


int create_socket() {
    int socket = ConexaoRawSocket("lo");

    return socket;
}

backup_t *create_backup() {
    backup_t *backup = malloc(sizeof(backup_t));
    test_alloc(backup, "backup");

    backup->socket = create_socket();
    backup->sequence = 0;

    backup->message = create_message();

    backup->recv_buffer = malloc(sizeof(unsigned char) * BUFFER_MAX_LEN);
    test_alloc(backup->recv_buffer, "backup receive buffer");

    return backup;
}

void free_backup(backup_t *backup) {
    if (!backup)
        return;

    if (backup->recv_buffer)
        free(backup->recv_buffer);

    free_message(backup->message);
    free(backup);

    return;
}

message_t *create_message() {
    message_t *message = malloc(sizeof(message_t));
    test_alloc(message, "message");

    message->start_marker = 0;
    message->size = 0;
    message->sequence = 0;
    message->type = 0;
    message->data = NULL;
    message->parity = 0;

    return message;
}

void free_message(message_t* message) {
    if (!message)
        return;

    if (message->data)
        free(message->data);

    free(message);

    return;
}

void message_reset(message_t* message) {
    if (!message)
        return;

    message->start_marker = START_MARKER;

    if (message->data)
        free(message->data);

    message->size = 0;
    message->data = NULL;

    return;
}

void make_backup_message(backup_t *backup, char *path) {
    message_t* m = backup->message;

    message_reset(m);

    int name_len = strlen(path);

    m->size = name_len;
    m->sequence = backup->sequence;
    m->type = BACKUP_FILE;

    m->data = malloc(sizeof(unsigned char) * name_len);
    test_alloc(m->data, "backup message data");

    memcpy(m->data, path, name_len);

    set_message_parity(m);

    return;
}

void make_ack_message(message_t* message) {
    message_reset(message);
    message->type = ACK;

    return;
}

void make_nack_message(message_t* message) {
    message_reset(message);
    message->type = ACK;

    return;
}

ssize_t send_message(backup_t *backup) {
    if (!backup)
        return -1;

    message_t *m = backup->message;

    int buffer_size = m->size + MESSAGE_CAPSULE_SIZE;

    unsigned char *buffer = malloc(sizeof(unsigned char) * buffer_size);
    test_alloc(buffer, "buffer");

    message_to_buffer(m, buffer);

    #ifdef DEBUG
    print_buffer(buffer, buffer_size);
    #endif

    ssize_t size = send(backup->socket, buffer, BUFFER_MAX_LEN, 0);

    free(buffer);

    return size;
}

ssize_t receive_message(backup_t *backup) {
    if (!backup)
        return -1;

    message_t *m = backup->message;

    #ifdef DEBUG
    printf("[ETHBKP][RCVM] Waiting message\n");
    #endif

    ssize_t size = recv(
        backup->socket,
        backup->recv_buffer,
        BUFFER_MAX_LEN,
        0
    );

    #ifdef DEBUG
    printf("[ETHBKP][RCVM] Message received\n");
    print_buffer(backup->recv_buffer, BUFFER_MAX_LEN); 
    #endif

    buffer_to_message(backup->recv_buffer, backup->message);

    #ifdef DEBUG
    printf("[ETHBKP][RCVM] Message created\n");
    print_message(backup->message);
    #endif

    return size;
}

void message_to_buffer(message_t* message, unsigned char* buffer) {
    if (!message || !buffer)
        return;

    int buffer_size = message->size + MESSAGE_CAPSULE_SIZE;

    buffer[0] = message->start_marker;
    buffer[1] = (message->size << 2) + (message->sequence >> 4);
    buffer[2] = message->type + (message->sequence << 4);

    memcpy(&buffer[3], message->data, message->size);

    buffer[buffer_size - 1] = message->parity;

    return;
} 

void buffer_to_message(unsigned char *buffer, message_t *message) {
    if (!buffer || !message)
        return;

    message_reset(message);

    message->start_marker = buffer[0];
    message->size = buffer[1] >> 2;

    message->sequence = ((buffer[1] & 0b11) << 4) + (buffer[2] >> 4);

    message->type = buffer[2] & 0b1111;

    message->data = malloc(sizeof(unsigned char) * message->size);
    test_alloc(message->data, "message->data");

    memcpy(message->data, &buffer[3], message->size);

    message->parity = buffer[message->size + 3];

    return;
}

void get_file_md5() {
    return;
}

void set_message_parity(message_t* message) {
    if (!message)
        return;

    int buffer_size = message->size + MESSAGE_CAPSULE_SIZE;

    unsigned char* buffer = malloc(sizeof(unsigned char) * buffer_size);
    test_alloc(buffer, "set message parity buffer");

    message_to_buffer(message, buffer);

    unsigned char sum = buffer[1];

    for (int i = 2; i < buffer_size - 1; i++)
        sum ^= buffer[i];

    free(buffer);

    message->parity = sum;

    return;
}

int check_parity_message(message_t *message) {
    int buffer_size = message->size + MESSAGE_CAPSULE_SIZE;

    unsigned char* buffer = malloc(sizeof(unsigned char) * buffer_size);
    test_alloc(buffer, "check parity message buffer");

    message_to_buffer(message, buffer);

    int parity = check_parity(buffer, buffer_size);

    free(buffer);

    return parity;
}

int check_parity(unsigned char* buffer, int buffer_size) {
    if (!buffer)
        return -1;

    unsigned char sum = buffer[1];

    for (int i = 2; i < buffer_size; i++)
        sum ^= buffer[i];

    return sum == 0;
}

