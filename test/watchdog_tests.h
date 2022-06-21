#include "../inc/watchdog.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

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

static void watchdog_set_get_test(void);
static void watchdog_set_get_test(void) {
  Watchdog* watchdog = watchdog_create(1, "Hector", 1);
  assert(watchdog != NULL);

  char* name = watchdog_get_name(watchdog);
  assert(strcmp(name, "Hector") == 0);

  int flag = watchdog_get_alarm_flag(watchdog);
  assert(flag == 0);
  
  watchdog_set_flag(watchdog);

  flag = watchdog_get_alarm_flag(watchdog);
  assert(flag == 1);  

  free(name);
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

static void watchdog_scratch_test(void);
static void watchdog_scratch_test(void) {
  Watchdog* watchdog = watchdog_create(1, "Boris", 1);
  assert(watchdog != NULL);

  printf("> watchdog_scratch_test\n");
  printf("The watchdog is sleeping for 1 second...\n");
  sleep((unsigned int)1);

  watchdog_scratch(watchdog);
  bool alarm_rung = watchdog_check_alarm(watchdog);
  assert(alarm_rung == false);

  watchdog_destroy(watchdog);
}

void watchdog_tests(void);
void watchdog_tests(void) {
  watchdog_create_test();
  watchdog_set_get_test();
  watchdog_check_alarm_test();
  watchdog_scratch_test();
}
