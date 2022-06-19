#ifndef WATCHDOG_H
#define WATCHDOG_H
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define THREAD_NAME 16

typedef struct Watchdog {
  pthread_t id;
  size_t time_limit;
  struct timespec alarm;
  char name[THREAD_NAME];
} Watchdog;

Watchdog* watchdog_create(const pthread_t id, const char* const name, const size_t time_limit);

void watchdog_pet(Watchdog* restrict dog);

bool watchdog_check_alarm(const Watchdog* restrict dog);

void watchdog_destroy(Watchdog* watchdog);

#endif
