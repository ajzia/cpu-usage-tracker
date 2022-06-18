#ifndef RAEDER_H
#define RAEDER_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Reader {
  char* path;
  FILE* f;
  size_t read_frequency;
} Reader;

Reader* reader_create(char* path, const size_t read_frequency);

uint8_t* reader_read(register const Reader* restrict reader, const size_t cores, const size_t one_core_size);

void reader_destroy(Reader* reader);

void reader_reset(Reader* restrict reader);

#endif
