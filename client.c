#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "ConexaoRawSocket.h"
#include "utils.h"

void client_run() {
    int socket = ConexaoRawSocket("lo");

    printf("[ETHBKP] Running as client...\n");
    unsigned char data_buffer[DATA_MAX_LEN];
    char file_name[DATA_MAX_LEN];

    message_t* message = create_message();

    for (;;) {
        printf("[ETHBKP] Type file name: ");
        scanf("%s", file_name);

        FILE* file = fopen(file_name, "r");
        if (!file) {
            printf("Error: %s\n", strerror(errno));
            continue;
        }

        make_backup_message(message, file_name);        

        print_message(message);

        ssize_t size = send_message(socket, message);

        if (size < 0)
            printf("Error: %s\n", strerror(errno));

        printf("[ETHBKP] Message sent, size=%ld\n", size);
    }

    destroy_message(message);
    return;
}
