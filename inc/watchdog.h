#ifndef WATCHDOG_H
#define WATCHDOG_H
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>

#define THREAD_NAME 16

typedef struct Watchdog {
  pthread_mutex_t mutex;
  struct timespec alarm;
  char name[THREAD_NAME];
  pthread_t id;
  size_t time_limit;
  int alarm_flag;
} Watchdog;

Watchdog* watchdog_create(const pthread_t id, const char* const name, const size_t time_limit);

void watchdog_scratch(Watchdog* restrict watchdog);

char* watchdog_get_name(const Watchdog* const restrict watchdog);

int watchdog_get_alarm_flag(const Watchdog* const restrict watchdog);

void watchdog_set_flag(Watchdog* watchdog);

bool watchdog_check_alarm(Watchdog* restrict watchdog);

void watchdog_lock(Watchdog* watchdog);

void watchdog_unlock(Watchdog* watchdog);

void watchdog_destroy(Watchdog* watchdog);

#endif
