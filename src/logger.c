#include "../inc/logger.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define LEVEL_LENGTH 5
#define THREAD_NAME_LENGTH 11

static char* get_current_date(void) {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char* date = malloc(40);
  if (date == NULL)
    return NULL;

  snprintf(date, 40, "%d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return date;
}

Logger* logger_create(void) {
  char* date = get_current_date();
  char* path = malloc(50);
  if (path == NULL)
    return NULL;

  strcpy(path, "./log/cpu-usage-tracker_");
  strcat(path, date);
  strcat(path, ".log");

  struct stat st = {0};
  if (stat("./log", &st) != 0)
    mkdir("./log", 0700);

  Logger* const logger = malloc(sizeof(*logger));
  if (logger == NULL)
    return NULL;

  *logger = (Logger){.f = fopen(path, "a+")};

  if (logger->f == NULL)
    return NULL;

  free(path);
  free(date);

  return logger;
}

void logger_put(register Buffer* restrict buffer, const char* restrict log_type, const char* restrict date, const char* restrict thread_name, const char* restrict  action, const size_t packet_size) {
  if (buffer == NULL)
    return;
  
  if (date == NULL || thread_name == NULL || action == NULL || log_type == NULL)
    return;

  if (strcmp(log_type, "INFO") != 0 && strcmp(log_type, "ERROR") != 0)
    return;

  uint8_t* log = malloc(packet_size);
  if (log == NULL)
    return;

  snprintf((char*)log, 250, "[%s] [%s] [%s] %s", log_type, date, thread_name, action);

  buffer_put(buffer, log, packet_size);

  free(log);
}

void logger_read(register const Logger* const logger, register Buffer* buffer) {
  if (buffer == NULL || logger == NULL)
    return;

  uint8_t* const log = buffer_get(buffer);
  if (log == NULL)
    return;

  char* log_type = strtok((char*)log, " ");
  char* date = strtok(NULL, " ");
  char* time = strtok(NULL, " ");
  char* thread_name = strtok(NULL, " ");
  fprintf(logger->f, "%*s %s %s %*s", -LEVEL_LENGTH, log_type, date, time, -THREAD_NAME_LENGTH, thread_name);
  
  char* action = strtok((char*)NULL, " ");
  while (action != NULL) {
    fprintf(logger->f, "%s ", action);
    action = strtok(NULL, " ");
  }
  fprintf(logger->f, "\n");

  fflush(logger->f);

  free(log);
  free(action);
}

void logger_destroy(Logger* const logger) {
  if (logger == NULL) 
    return;
  
  fclose(logger->f);
  free(logger);
}
