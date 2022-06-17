#include "../inc/buffer.h"
#include <assert.h>

static void buffer_create_test(void) {
  Buffer* buffer = buffer_create();
  assert(buffer != NULL);

  buffer_destroy(buffer);
}

static void buffer_is_empty_test(void) {
  Buffer* buffer = buffer_create();
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);
  buffer_destroy(buffer);
} 

static void buffer_put_test(void) {
  Buffer* buffer = buffer_create();
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  register const size_t value_to_insert = 10;

  buffer_put(buffer, (int)value_to_insert);

  is_empty = buffer_is_empty(buffer);
  assert(!is_empty);

  buffer_destroy(buffer);
}

static void buffer_get_test(void) {
  Buffer* buffer = buffer_create();
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  register const size_t value_to_insert = 10;

  buffer_put(buffer, (int)value_to_insert);

  is_empty = buffer_is_empty(buffer);
  assert(!is_empty);

  buffer_get(buffer);

  is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  buffer_destroy(buffer);
}

static void buffer_is_full_test(void) {
  Buffer* buffer = buffer_create();
  assert(buffer != NULL);

  bool is_empty = buffer_is_empty(buffer);
  assert(is_empty);

  register const size_t value_to_insert = 10;

  for (size_t i = 0; i < 10; ++i) {
    buffer_put(buffer, (int)value_to_insert);
  }

  const bool is_full = buffer_is_full(buffer);
  assert(is_full);

  buffer_destroy(buffer);
}

void buffer_tests(void) {
  buffer_create_test();
  buffer_put_test();
  buffer_get_test();
  buffer_is_empty_test();
  buffer_is_full_test();
}
