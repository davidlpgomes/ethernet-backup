#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"


int max(int a, int b) {
    if (a > b)
        return a;

    return b;
}

int min(int a, int b) {
    if (a < b)
        return a;

    return b;
}

void test_alloc(void* p, char* name) {
    if (p)
        return;

    fprintf(stderr, "Error: could not alloc %s\n", name);
    exit(EXIT_FAILURE);

    return;
}

void print_buffer(unsigned char* buffer, int size) {
    for (int i = 0; i < size; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");
}

void print_message(message_t *message) {
    printf("start_marker: %c\n", message->start_marker);
    printf("size: %d\n", message->size);
    printf("sequence: %d\n", message->sequence);
    printf("type: %d\n", message->type);
    printf("data: ");
    for (int i = 0; i < message->size; i++)
        printf("%c", message->data[i]);
    printf("\n");
    printf("parity: %c\n", message->parity);
}

double timestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

    return (double) (tp.tv_sec + tp.tv_nsec * 1.0e-9);
}

