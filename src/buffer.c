#include "../inc/buffer.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

typedef struct Buffer {
  size_t max_size;
  size_t packet_size;
  size_t current_size;
  size_t head;
  size_t tail;
  pthread_mutex_t mutex;
  pthread_cond_t can_produce;
  pthread_cond_t can_consume;
  uint8_t buffer[]; /* FAM */
} Buffer;

Buffer* buffer_create(const size_t packet_size, const size_t max_size) {
  if (packet_size == 0 || max_size == 0)
    return NULL;

  size_t buffer_size = packet_size * max_size;

  Buffer* const buffer = malloc(sizeof(*buffer) + sizeof(uint8_t) * buffer_size);
  if (buffer == NULL)
      return NULL;

  *buffer = (Buffer){.max_size = max_size,
                     .packet_size = packet_size,
                     .current_size = (size_t) 0,
                     .head = (size_t) 0,
                     .tail = (size_t) 0
                    };

  pthread_mutex_init(&buffer->mutex, NULL);
  pthread_cond_init(&buffer->can_produce, NULL);
  pthread_cond_init(&buffer->can_consume, NULL);

  return buffer;
}

bool buffer_is_empty(const Buffer* const buffer) {
  if (buffer == NULL)
    return false;

  return buffer->current_size == 0;
}

bool buffer_is_full(const Buffer* const buffer) {
  if (buffer == NULL)
    return false;
    
  return buffer->current_size == buffer->max_size;
}

void buffer_put(Buffer* const buffer, register const uint8_t packet[], register const size_t packet_size) {
  if (buffer == NULL)
    return;
  
  if (buffer_is_full(buffer))
    return;

  if (packet_size != buffer->packet_size) {
    return;
  }
  size_t packet_address = buffer->head * buffer->packet_size;

  memcpy(&buffer->buffer[packet_address], &packet[0], buffer->packet_size);

  buffer->head = (buffer->head + 1) % buffer->max_size;
  ++buffer->current_size;
}

uint8_t* buffer_get(Buffer* const buffer) {
  if (buffer == NULL)
    return NULL;

  if (buffer_is_empty(buffer))
    return NULL;

  uint8_t* packet = malloc(sizeof(packet) * buffer->packet_size);
  if (packet == NULL)
    return NULL;

  size_t packet_address = buffer->tail * buffer->packet_size;

  memcpy(&packet[0], &buffer->buffer[packet_address], buffer->packet_size);

  buffer->tail = (buffer->tail + 1) % buffer->max_size;
  --buffer->current_size;

  return packet;
}

void buffer_lock(Buffer* const buffer) {
  if (buffer == NULL)
    return;

  pthread_mutex_lock(&buffer->mutex);
}

void buffer_unlock(Buffer* const buffer) {
  if (buffer == NULL)
    return;

  pthread_mutex_unlock(&buffer->mutex);
}

void buffer_call_producer(Buffer* const buffer) {
  if (buffer == NULL)
    return;

  pthread_cond_signal(&buffer->can_produce);
}

void buffer_call_consumer(Buffer* const buffer) {
  if (buffer == NULL)
    return;

  pthread_cond_signal(&buffer->can_consume);
}

void buffer_wait_for_producer(Buffer* const buffer) {
  if (buffer == NULL)
    return;

  pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
}

void buffer_wait_for_consumer(Buffer* const buffer) {
  if (buffer == NULL)
    return;
    
  pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
}

void buffer_destroy(Buffer* const buffer) {
  if (buffer == NULL)
    return;
    
  pthread_mutex_destroy(&buffer->mutex);
  pthread_cond_destroy(&buffer->can_consume);
  pthread_cond_destroy(&buffer->can_produce);
  
  free(buffer);
}
