#include "../inc/logger.h"
#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>

static void logger_create_test(void) {
  Logger* logger = logger_create();
  assert(logger != NULL);
  assert(logger->f != NULL);

  logger_destroy(logger);
}

static void logger_put_read_test(void) {
  register const size_t packet_size = 200;
  register const size_t max_size = 50;

  Buffer* buffer = buffer_create(packet_size, max_size);
  assert(buffer != NULL);

  logger_put(buffer, "INFO", "2022-19-06_14:38:57", "LOGGER", "LOGGER_PUT_TEST", packet_size);

  bool empty = buffer_is_empty(buffer);
  assert(!empty);

  Logger* logger = logger_create();
  assert(logger != NULL);

  logger_read(logger, buffer);

  buffer_destroy(buffer);
  logger_destroy(logger);
}


void logger_tests(void) {
  logger_create_test();
  logger_put_read_test();
}
