#include "../inc/watchdog.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>

static void watchdog_create_test(void);
static void watchdog_create_test(void) {
  Watchdog* watchdog = watchdog_create(1, NULL, 10);
  assert(watchdog == NULL);

  watchdog = watchdog_create(1, "a", 0);
  assert(watchdog == NULL);

  watchdog = watchdog_create(1, "name", 10);
  assert(watchdog != NULL);

  watchdog_destroy(watchdog);
}

static void watchdog_check_alarm_test(void);
static void watchdog_check_alarm_test(void) {  
  Watchdog* watchdog = watchdog_create(1, "Hector", 1);
  assert(watchdog != NULL);

  printf("> watchdog_check_alarm_test\n");
  printf("The watchdog is sleeping for 2 seconds...\n");
  sleep((unsigned int)2);

  bool alarm_rung = watchdog_check_alarm(watchdog);
  assert(alarm_rung);

  watchdog_destroy(watchdog);
}

static void watchdog_pet_test(void);
static void watchdog_pet_test(void) {
  Watchdog* watchdog = watchdog_create(1, "Boris", 2);
  assert(watchdog != NULL);

  sleep((unsigned int)2);

  watchdog_pet(watchdog);

  printf("> watchdog_pet_test\n");
  printf("The watchdog is sleeping for 1 second...\n");
  sleep(1);

  watchdog_pet(watchdog);
  bool alarm_rung = watchdog_check_alarm(watchdog);
  assert(alarm_rung == false);

  watchdog_destroy(watchdog);
}

void watchdog_tests(void);
void watchdog_tests(void) {
  watchdog_create_test();
  watchdog_check_alarm_test();
  watchdog_pet_test();
}
