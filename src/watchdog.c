#include <time.h> 
#include "../inc/watchdog.h"
#include <string.h>
#include <stdio.h> 

void watchdog_pet(Watchdog* restrict watchdog) {
  if (watchdog == NULL)
    return;

  clock_gettime(CLOCK_MONOTONIC, &watchdog->alarm);
}

Watchdog* watchdog_create(const pthread_t id, const char* const name, const size_t time_limit) {
  if (name == NULL || time_limit == 0) 
    return NULL;

  Watchdog* watchdog = malloc(sizeof(*watchdog));
  if (watchdog == NULL)
    return NULL;

  *watchdog = (Watchdog){.id = id,
               .time_limit = time_limit
              };

  strcpy(watchdog->name, name);
  watchdog_pet(watchdog);

  return watchdog;
}

bool watchdog_check_alarm(const Watchdog* const restrict watchdog) {
  if (watchdog == NULL)
    return false;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  const size_t seconds_passed = (size_t)now.tv_sec - (size_t)watchdog->alarm.tv_sec;

  return seconds_passed >= watchdog->time_limit;
}

void watchdog_destroy(Watchdog* const watchdog) {
  if (watchdog == NULL)
    return;

  free(watchdog);
}
