#include <time.h> 
#include "../inc/watchdog.h"
#include <string.h>
#include <stdio.h> 

void watchdog_lock(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return;

  pthread_mutex_lock(&watchdog->mutex);
}

void watchdog_unlock(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return;

  pthread_mutex_unlock(&watchdog->mutex);
}

void watchdog_scratch(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return;

  watchdog_lock(watchdog);
  clock_gettime(CLOCK_MONOTONIC, &watchdog->alarm);
  watchdog_unlock(watchdog);
}

Watchdog* watchdog_create(const pthread_t id, const char* const name, const size_t time_limit) {
  if (name == NULL || time_limit == 0) 
    return NULL;

  Watchdog* const watchdog = malloc(sizeof(*watchdog));
  if (watchdog == NULL)
    return NULL;

  *watchdog = (Watchdog){.id = id,
               .time_limit = time_limit,
               .alarm_flag = 0
              };

  strcpy(watchdog->name, name);
  watchdog_scratch(watchdog);
  pthread_mutex_init(&watchdog->mutex, NULL);

  return watchdog;
}

char* watchdog_get_name(const Watchdog* const watchdog) {
  if (watchdog == NULL)
    return NULL;

  char* name = malloc(sizeof(watchdog->name));
  if (name == NULL)
    return NULL;

  memcpy(name, watchdog->name, sizeof(watchdog->name));

  return name;
}

int watchdog_get_alarm_flag(const Watchdog* const restrict watchdog) {
  if (watchdog == NULL)
    return 0;

  return watchdog->alarm_flag;
}

void watchdog_set_flag(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return;

  watchdog->alarm_flag = 1;
}

bool watchdog_check_alarm(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return false;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  const size_t seconds_passed = (size_t)now.tv_sec - (size_t)watchdog->alarm.tv_sec;

  if (seconds_passed >= watchdog->time_limit)
    watchdog->alarm_flag = 1;
    
  return (bool)watchdog->alarm_flag;
}

void watchdog_destroy(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return;

  pthread_mutex_destroy(&watchdog->mutex);
  free(watchdog);
}
