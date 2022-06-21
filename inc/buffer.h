#ifndef BUFFER_H
#define BUFFER_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct Buffer Buffer;

Buffer* buffer_create(size_t packet_size, size_t max_size);
void buffer_destroy(Buffer* const buffer);

bool buffer_is_empty(const Buffer* buffer);
bool buffer_is_full(const Buffer* buffer);

void buffer_put(Buffer* buffer, const uint8_t packet[], size_t packet_size);
uint8_t* buffer_get(Buffer* buffer);

void buffer_lock(Buffer* buffer);
void buffer_unlock(Buffer* buffer);

void buffer_call_producer(Buffer* buffer);
void buffer_call_consumer(Buffer* buffer);

void buffer_wait_for_producer(Buffer* buffer);
void buffer_wait_for_consumer(Buffer* buffer);

#endif
