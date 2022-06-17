#include "../inc/buffer.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static void buffer_create_test(void) {
  Buffer* buffer = buffer_create(0, 1);
  assert(buffer == NULL);

  buffer = buffer_create(1, 0);
  assert(buffer == NULL);

  buffer = buffer_create(1, 1);
  assert(buffer != NULL);

  buffer_destroy(buffer);
}

static void buffer_is_empty_test(void) {
  Buffer* buffer = buffer_create(1, 1);
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);
  buffer_destroy(buffer);
} 

static void buffer_put_test(void) {
  register const size_t packet_size = 1;
  register const size_t max_size = 1;

  Buffer* buffer = buffer_create(packet_size, max_size);
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  const uint8_t packet[] = {0xF};;

  buffer_put(buffer, packet, packet_size);

  is_empty = buffer_is_empty(buffer);
  assert(!is_empty);

  buffer_destroy(buffer);
}

static void buffer_get_test(void) {
  register const size_t packet_size = 1;
  register const size_t max_size = 1;

  Buffer* buffer = buffer_create(packet_size, max_size);
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  const uint8_t packet[] = {10};

  buffer_put(buffer, packet, packet_size);

  is_empty = buffer_is_empty(buffer);
  assert(!is_empty);

  uint8_t* const packet2 = buffer_get(buffer);

  is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  free(packet2);
  buffer_destroy(buffer);
}

static void buffer_is_full_test(void) {
  register const size_t packet_size = 1;
  register const size_t max_size = 10;

  Buffer* buffer = buffer_create(packet_size, max_size);
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  const uint8_t packet[] = {0xF};

  for (size_t i = 0; i < max_size; ++i) {
    buffer_put(buffer, packet, packet_size);
  }

  const bool is_full = buffer_is_full(buffer);
  assert(is_full);

  buffer_destroy(buffer);
}

void buffer_tests(void) {
  buffer_create_test();
  buffer_is_empty_test();
  buffer_put_test();
  buffer_get_test();
  buffer_is_full_test();
}
