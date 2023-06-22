#include "backup.h"

#include <arpa/inet.h>
#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <limits.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ConexaoRawSocket.h"
#include "utils.h"


int create_socket() {
    int socket = ConexaoRawSocket("lo");

    // Set timeout to U_TIMEOUT
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    if (
        setsockopt(
            socket,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (const char*) &tv,
            sizeof(tv)
        ) == -1
    ) {
        fprintf(stderr, "Error: could not set socket time out option\n");
        exit(EXIT_FAILURE);
    }

    return socket;
}

backup_t *create_backup() {
    backup_t *backup = malloc(sizeof(backup_t));
    test_alloc(backup, "backup");

    backup->socket = create_socket();
    backup->sequence = 0;

    backup->send_message = create_message();
    backup->recv_message = create_message();

    backup->send_buffer = malloc(sizeof(unsigned char) * BUFFER_MAX_LEN);
    test_alloc(backup->send_buffer, "backup send buffer");

    backup->recv_buffer = malloc(sizeof(unsigned char) * BUFFER_MAX_LEN);
    test_alloc(backup->recv_buffer, "backup receive buffer");

    return backup;
}

void free_backup(backup_t *backup) {
    if (!backup)
        return;

    if (backup->send_buffer)
        free(backup->send_buffer);

    if (backup->recv_buffer)
        free(backup->recv_buffer);

    free_message(backup->send_message);
    free_message(backup->recv_message);

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

void free_message(message_t *message) {
    if (!message)
        return;

    if (message->data)
        free(message->data);

    free(message);

    return;
}

void message_reset(message_t *message) {
    if (!message)
        return;

    message->start_marker = START_MARKER;

    if (message->data)
        free(message->data);

    message->size = 0;
    message->data = NULL;

    return;
}

void make_reset_sequence_message(backup_t *backup) {
    if (!backup)
        return;

    message_t *m = backup->send_message;

    backup->sequence = 0;

    message_reset(m);
    m->type = RESET_SEQUENCE;
    m->sequence = backup->sequence;

    set_message_parity(m);

    return;
}

void make_num_of_files_message(backup_t *backup, int num) {
    if (!backup)
        return;

    message_t *m = backup->send_message;

    message_reset(m);

    m->size = sizeof(int);
    m->sequence = backup->sequence;
    m->type = BACKUP_FILES;

    m->data = malloc(sizeof(int));
    test_alloc(m->data, "num of files message");

    memcpy(m->data, &num, sizeof(int));

    set_message_parity(m);

    return;
}

void make_end_files_message(backup_t *backup) {
    if (!backup) 
        return;

    message_t *m = backup->send_message;

    message_reset(m);

    m->sequence = backup->sequence;
    m->type = END_FILES;

    set_message_parity(m);

    return;

}

void make_backup_file_message(backup_t *backup, char *path) {
    if (!backup || !path)
        return;

    message_t *m = backup->send_message;

    message_reset(m);

    char *name = basename(path);

    int name_len = strlen(name);

    m->size = name_len;
    m->sequence = backup->sequence;
    m->type = BACKUP_FILE;

    m->data = malloc(sizeof(unsigned char) * name_len);
    test_alloc(m->data, "backup message file name");

    memcpy(m->data, name, name_len);

    set_message_parity(m);

    return;
}

void make_end_file_message(backup_t *backup) {
    if (!backup)
        return;

    message_t *m = backup->send_message;
    message_reset(m);

    m->sequence = backup->sequence;
    m->type = END_FILE;

    set_message_parity(m);

    return;
}

void make_data_message(
    backup_t *backup,
    unsigned char *data,
    unsigned data_size
) {
    if (!backup || !data)
        return;

    message_t *m = backup->send_message;
    message_reset(m);

    m->size = data_size;
    m->sequence = backup->sequence;
    m->type = DATA;

    m->data = malloc(sizeof(unsigned char) * data_size);
    test_alloc(m->data, "backup message data");

    memcpy(m->data, data, data_size);

    set_message_parity(m);

    return;
}

void make_backup_directory_message(backup_t *backup, char *path) {
    if (!backup || !path)
        return;

    message_t *m = backup->send_message;
    message_reset(m);

    ssize_t size = strlen(path);

    m->size = size;
    m->sequence = backup->sequence;
    m->type = DEFINE_BACKUP_DIRECTORY;

    m->data = malloc(sizeof(unsigned char) * size);
    test_alloc(m->data, "backup directory message");

    memcpy(m->data, path, size);

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
    message->type = NACK;

    return;
}

void send_acknowledgement(backup_t *backup, int is_ack) {
    if (!backup)
        return;

    message_reset(backup->send_message);
    backup->send_message->type = is_ack ? ACK : NACK;

    #ifdef DEBUG
    printf("[ETHBKP][SACK] Sending %s\n", is_ack ? "ACK" : "NACK");
    #endif

    message_to_buffer(backup->send_message, backup->send_buffer);

    send(backup->socket, backup->send_buffer, BUFFER_MAX_LEN, 0);

    return;
}

void update_sequence(backup_t *backup) {
    backup->sequence = (backup->sequence + 1) % 64;

    return;
}

ssize_t send_message(backup_t *backup) {
    if (!backup)
        return -1;

    message_t *m = backup->send_message;

    message_to_buffer(m, backup->send_buffer);

    #ifdef DEBUG
    printf("[ETHBKP][SDMSG] Sending message\n");
    print_message(m);
    #endif

    ssize_t size;
    int is_ack = 0;

    while (!is_ack) {
        size = send(
            backup->socket,
            backup->send_buffer,
            BUFFER_MAX_LEN,
            0
        );

        is_ack = wait_acknowledgement(backup);

        #ifdef DEBUG
        printf("[ETHBKP][SNDMSG] Message sent, is_ack=%d\n\n", is_ack);
        #endif
    }

    update_sequence(backup);

    return size;
}

ssize_t receive_message(backup_t *backup) {
    ssize_t size;
    message_t *m = backup->recv_message;

    int valid_message;

    for (;;) {
        size = recv(backup->socket, backup->recv_buffer, BUFFER_MAX_LEN, 0);

        if (size == -1 || backup->recv_buffer[0] != START_MARKER) 
            continue;

        buffer_to_message(backup->recv_buffer, m);

        if (m->type == ACK || m->type == NACK)
            continue;

        if (
            m->type != RESET_SEQUENCE &&
            m->sequence != backup->sequence
        )
            continue;

        #ifdef DEBUG
        printf("[ETHBKP][RCVM] Message received: ");
        print_message(backup->recv_message);
        #endif

        valid_message = check_message_parity(m);
        send_acknowledgement(backup, valid_message);
        
        if (valid_message)
            break; 
    };

    if (m->type == RESET_SEQUENCE) 
        backup->sequence = 1;
    else
        update_sequence(backup);

    return size;
}

int wait_acknowledgement(backup_t *backup) {
    if (!backup)
        return -1;

    #ifdef DEBUG
    printf("[ETHBKP][WACK] Waiting acknowledgement\n");
    #endif

    ssize_t size = -1;
    int is_ack = 0;

    for (;;) {
        size = recv(
            backup->socket,
            backup->recv_buffer,
            BUFFER_MAX_LEN,
            0
        );

        if (size == -1) {
            #ifdef DEBUG
            printf(
                "[ETHBKP][WACK] No message received after timeout: errno=%d\n",
                errno
            );
            #endif

            break;
        }

        buffer_to_message(backup->recv_buffer, backup->recv_message);

        if (backup->recv_message->start_marker != START_MARKER)
            continue;

        if (
            backup->recv_message->type != ACK &&
            backup->recv_message->type != NACK
        )
            continue;

        #ifdef DEBUG
        printf(
            "[ETHBKP][WACK] Received %s\n",
            backup->recv_message->type == ACK ? "ACK" : "NACK"
        );
        #endif

        if (backup->recv_message->type == ACK)
            is_ack = 1;

        break;
    }

    return is_ack;
}

int wait_ack_or_error(backup_t *backup) {
    if (!backup)
        return -1;

    #ifdef DEBUG
    printf("[ETHBKP][WACKOE] Waiting acknowledgement\n");
    #endif

    ssize_t size = -1;
    int is_ack = 0;

    for (;;) {
        size = recv(
            backup->socket,
            backup->recv_buffer,
            BUFFER_MAX_LEN,
            0
        );

        if (size == -1) {
            #ifdef DEBUG
            printf(
                "[ETHBKP][WACKOE] No message received after timeout: errno=%d\n",
                errno
            );
            #endif

            break;
        }

        buffer_to_message(backup->recv_buffer, backup->recv_message);

        if (backup->recv_message->start_marker != START_MARKER)
            continue;

        if (backup->recv_message->type == ERROR) {
            is_ack == backup->recv_message->data[0];
            break;
        }

        if (
            backup->recv_message->type != ACK &&
            backup->recv_message->type != NACK
        )
            continue;

        #ifdef DEBUG
        printf(
            "[ETHBKP][WACK] Received %s\n",
            backup->recv_message->type == ACK ? "ACK" : "NACK"
        );
        #endif

        if (backup->recv_message->type == ACK)
            is_ack = 1;

        break;
    }

    return is_ack;
    
}

void message_to_buffer(message_t* message, unsigned char* buffer) {
    if (!message || !buffer)
        return;

    int buffer_size = message->size + MESSAGE_CAPSULE_SIZE;

    buffer[0] = message->start_marker;
    buffer[1] = (message->size << 2) + (message->sequence >> 4);
    buffer[2] = message->type + (message->sequence << 4);

    if (message->size)
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

    if (message->size) {
        message->data = malloc(sizeof(unsigned char) * message->size);
        test_alloc(message->data, "message->data");

        memcpy(message->data, &buffer[3], message->size);
    }

    message->parity = buffer[message->size + 3];

    return;
}

void backup_files(backup_t *backup, char *pattern) {
    if (!backup || !pattern)
        return;

    glob_t globe;

    int ret = glob(pattern, GLOB_ERR | GLOB_TILDE, NULL, &globe);

    if (ret) {
        if (ret == GLOB_NOMATCH)
            printf("Nenhum arquivo encontrado\n");
        else
            printf("Erro: glob %s\n", strerror(errno));

        return;
    }

    size_t files = globe.gl_pathc;

    if (files > 1) {
        make_num_of_files_message(backup, files);
        ssize_t size = send_message(backup);
    }

    char **file = globe.gl_pathv;
    while (*file) {
        printf("Enviando arquivo %s...\n", *file);
        send_file(backup, *file);

        file++;
    }

    if (files > 1) {
        make_end_files_message(backup);
        ssize_t size = send_message(backup);
    }

    globfree(&globe);

    return;
}

void send_file(backup_t *backup, char *path) {
    if (!backup || !path)
        return;

    FILE *file = fopen(path, "rb");

    if (!file) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", strerror(errno));
        return;
    }

    // Sending file name
    make_backup_file_message(backup, path);
    ssize_t size = send_message(backup);

    if (size < 0) {
        printf("Error: %s\n", strerror(errno));
        return;
    }

    // Send file data
    int buffer_size = DATA_MAX_LEN;
    unsigned char *buffer = malloc(sizeof(unsigned char) * DATA_MAX_LEN);
    test_alloc(buffer, "client backup file buffer");

    ssize_t size_data;
    while (!feof(file)) {
        buffer_size = fread(buffer, sizeof(*buffer), DATA_MAX_LEN, file);

        if (!buffer_size)
            break;

        make_data_message(backup, buffer, buffer_size);
        size_data = send_message(backup);

        if (size_data < 0)
            printf("Error: %s\n", strerror(errno));
    }

    fclose(file);

    // Send end file
    make_end_file_message(backup);
    size = send_message(backup);

    if (size < 0) {
        printf("Error: %s\n", strerror(errno));
        return;
    }

    return;
}

void receive_files(backup_t *backup, unsigned num_files) {
    if (!backup)
        return;

    printf("Recebendo %u arquivos\n", num_files);

    ssize_t size;
    for (int i = 0; i < num_files; i++) {
        size = receive_message(backup);

        if (!size) {
            fprintf(stderr, "Receive files - SIZE is invalid!");
            exit(1);
        }

        receive_file(backup);
    }

    size = receive_message(backup);
    printf("Fim do grupo de arquivos\n");

    return;
}

void receive_file(backup_t *backup) {
    if (!backup)
        return;

    int size_f = backup->recv_message->size; 

    char *f = malloc(sizeof(char) * (size_f + 1));
    test_alloc(f, "receive_file file name");

    f[size_f] = '\0';
    memcpy(f, backup->recv_message->data, size_f);

    printf("Salvando arquivo %s\n", f);

    FILE *file = fopen(f, "wb");

    free(f);

    if (!file) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", strerror(errno));
        return;
    }

    ssize_t size = receive_message(backup);

    if (size < 0) {
        printf("Error: %s\n", strerror(errno));
        return;
    }

    while (backup->recv_message->type == DATA) {
        fwrite(
            backup->recv_message->data,
            sizeof(*backup->recv_message->data),
            backup->recv_message->size,
            file
        );
         
        size = receive_message(backup);

        if (size < 0) {
            printf("Error: %s\n", strerror(errno));
            return;
        }
    }

    fclose(file);

    return;
}

void get_file_md5(unsigned char *out, char *file_name) {
    if (!out || !file_name)
        return;

    FILE *file = fopen(file_name, "rb");

    if (!file) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", strerror(errno));
        return;
    }

    int buffer_size = DATA_MAX_LEN;
    unsigned char *buffer = malloc(sizeof(unsigned char) * DATA_MAX_LEN);
    test_alloc(buffer, "MD5 file buffer");

    MD5_CTX c;
    MD5_Init(&c);

    ssize_t size = fread(buffer, sizeof(*buffer), DATA_MAX_LEN, file);

    while (size > 0) {
        MD5_Update(&c, buffer, size);
        size = fread(buffer, sizeof(*buffer), DATA_MAX_LEN, file);
    }

    MD5_Final(out, &c);

    free(buffer);

    #ifdef DEBUG
    printf("[ETHBKP] MD5: ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%c ", out[i]);
    printf("\n");
    #endif

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

int check_message_parity(message_t *message) {
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

