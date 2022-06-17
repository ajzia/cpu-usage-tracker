#include "buffer.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#define BUFFER_SIZE 10

typedef struct Buffer {
  int buffer[BUFFER_SIZE];

  size_t size;
  size_t head;
  size_t tail;

  pthread_mutex_t mutex;
  pthread_cond_t can_produce;
  pthread_cond_t can_consume;
} Buffer;

Buffer* buffer_create(void)
{
    Buffer* buffer = malloc(sizeof(*buffer));
    if (buffer == NULL)
        return NULL;

    *buffer = (Buffer){.buffer = {0},
                             .size = 0,
                             .head = 0,
                             .tail = 0,
                             .mutex = PTHREAD_MUTEX_INITIALIZER,
                             .can_produce = PTHREAD_COND_INITIALIZER,
                             .can_consume = PTHREAD_COND_INITIALIZER
                            };

    return buffer;
}

void buffer_destroy(Buffer* const buffer) {
  pthread_cond_destroy(&buffer->can_consume);
  pthread_cond_destroy(&buffer->can_produce);
  pthread_mutex_destroy(&buffer->mutex);
  
  free(buffer);
}

bool buffer_is_empty(const Buffer* const restrict buffer) {
  return buffer->size == 0;
}

bool buffer_is_full(const Buffer* const restrict buffer) {
  return buffer->size == BUFFER_SIZE;
}

void buffer_put(Buffer* const buffer, const int product) {
  if (buffer_is_full(buffer))
    return;

  buffer->buffer[buffer->head] = product;
  buffer->head = (buffer->head + 1) % BUFFER_SIZE;
  ++buffer->size;
}

int buffer_get(Buffer* const buffer) {
  if (buffer_is_empty(buffer))
    return -1;

  const int product = buffer->buffer[buffer->tail];
  buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
  --buffer->size;

  return product;
}

void buffer_lock(Buffer* const buffer) {
  pthread_mutex_lock(&buffer->mutex);
}

void buffer_unlock(Buffer* const buffer) {
  pthread_mutex_unlock(&buffer->mutex);
}

void buffer_call_producer(Buffer* const buffer) {
  pthread_cond_signal(&buffer->can_produce);
}

void buffer_call_consumer(Buffer* const buffer) {
  pthread_cond_signal(&buffer->can_consume);
}

void buffer_wait_for_producer(Buffer* const buffer) {
  pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
}

void buffer_wait_for_consumer(Buffer* const buffer) {
  pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
}
