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

Reader* reader_create(char* path, size_t read_frequency);
void reader_destroy(Reader* reader);

uint8_t* reader_read(const Reader* reader, size_t cores, size_t one_core_size);
void reader_reset(Reader* reader);

#endif
