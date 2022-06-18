#include "../inc/reader.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define one_core_size 250 

static void reader_create_test() {
  Reader* reader = reader_create("/proc/stat", 1);
  assert(reader != NULL);

  reader_destroy(reader);
}

static void reader_read_test() {
  Reader* reader = reader_create("/proc/stat", 1);
  assert(reader != NULL);

  const size_t cores = sysconf(_SC_NPROCESSORS_ONLN);
  const size_t packet_size = one_core_size * (cores + 1);

  uint8_t* packet = reader_read(reader, cores, packet_size);
  assert(packet != NULL);

  free(packet);
  reader_destroy(reader);
}

static void reader_rewind_test() {
  Reader* reader = reader_create("/proc/stat", 1);
  assert(reader != NULL);

  fseek(reader->f, 0, 2);
  reader_rewind(reader);
  assert(ftell(reader->f) == 0);

  reader_destroy(reader);
}

void reader_tests(void) {
  reader_create_test();
  reader_read_test();
  reader_rewind_test();
}
