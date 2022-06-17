#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>   /* size_t */
#include <stdbool.h>  /* bool */

/* typedef struct Buffer_Observability {
  size_t no_put;
  size_t no_put_on_full;
  size_t no_get;
  size_t no_get_on_full;
} Buffer_Observability; */

typedef struct Buffer Buffer;

Buffer* buffer_create(void);
void buffer_destroy(Buffer* const buffer);

bool buffer_is_empty(const Buffer* restrict buffer);
bool buffer_is_full(const Buffer* restrict buffer);

void buffer_put(Buffer* buffer, int product);
int buffer_get(Buffer* buffer);

void buffer_lock(Buffer* buffer);
void buffer_unlock(Buffer* buffer);
void buffer_call_producer(Buffer* buffer);
void buffer_call_consumer(Buffer* buffer);
void buffer_wait_for_producer(Buffer* buffer);
void buffer_wait_for_consumer(Buffer* buffer);

#endif
