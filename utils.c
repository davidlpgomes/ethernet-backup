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

double timestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

    return (double) (tp.tv_sec + tp.tv_nsec * 1.0e-9);
}

