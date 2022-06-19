#ifndef LOGGER_H
#define LOGGER_H

#include "buffer.h"
#include <stdio.h>

typedef struct Logger {
  FILE* f;
} Logger;

Logger* logger_create(void);

void logger_put(register Buffer* buffer, const char* log_type, const char* date, const char* thread_name, const char* action, const size_t packet_size);

void logger_read(register const Logger* logger, register Buffer* buffer);

void logger_destroy(Logger* logger);

#endif
