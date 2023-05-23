#ifndef UTILS_H
#define UTILS_H

#include "backup.h"

int max(int a, int b);

int min(int a, int b);

void test_alloc(void* p, char* name);

void print_buffer(unsigned char* buffer, int size);

void print_message(message_t *message);

double timestamp();

#endif

