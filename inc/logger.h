#ifndef LOGGER_H
#define LOGGER_H

#include "buffer.h"
#include <stdio.h>

typedef struct Logger {
  FILE* f;
} Logger;

Logger* logger_create(void);
void logger_destroy(Logger* logger);

void logger_put(Buffer* restrict buffer, const char* restrict log_type, const char* restrict date, const char* restrict thread_name, const char* restrict action, size_t packet_size);
void logger_read(const Logger* logger, Buffer* buffer);

#endif
