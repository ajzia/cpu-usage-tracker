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
  char pad[4];
} Watchdog;

Watchdog* watchdog_create(pthread_t id, const char* name, size_t time_limit);
void watchdog_destroy(Watchdog* watchdog);

char* watchdog_get_name(const Watchdog* watchdog);
int watchdog_get_alarm_flag(const Watchdog* watchdog);
void watchdog_set_flag(Watchdog* watchdog);

void watchdog_scratch(Watchdog* watchdog);
bool watchdog_check_alarm(Watchdog* watchdog);

void watchdog_lock(Watchdog* watchdog);
void watchdog_unlock(Watchdog* watchdog);

#endif
