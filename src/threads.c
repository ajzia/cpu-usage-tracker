#include "../inc/threads.h"
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#define reader_one_core 250 
#define logger_message_size 250 
#define THREADS_COUNT 5
#define THREADS_BUFFER_SIZE 10
#define LOGGER_BUFFER_SIZE 50
#define WATCHDOG_TIMER 2

#define logger_log(buffer, log_type, thread_name, action, packet_size) \
  do { \
    buffer_lock(buffer); \
    if (buffer_is_full(buffer)) { \
      buffer_wait_for_consumer(buffer); \
    } \
    char* date = get_current_date(); \
    logger_put(buffer, log_type, date, thread_name, action, packet_size);  \
    buffer_call_consumer(buffer); \
    free(date); \
    buffer_unlock(buffer); \
  } while(0) \

#define watchdog_check(watchdog) \
  do { \
    watchdog_lock(watchdog); \
    watchdog_check_alarm(watchdog); \
    watchdog_unlock(watchdog); \
  } while(0) \

#define watchdog_alarm(watchdog) \
  do { \
    watchdog_lock(watchdog); \
    if (watchdog_get_alarm_flag(watchdog)) { \
      pthread_mutex_lock(&watchdog_mutex); \
      watchdog_flag = true; \
      dog_name = watchdog_get_name(watchdog); \
      pthread_mutex_unlock(&watchdog_mutex); \
    } \
    watchdog_unlock(watchdog); \
  } while(0) \

static size_t cores;
static size_t reader_packet_size;
static size_t read_frequency;
static size_t analyzerpacket_one_core;
static size_t analyzerpacket_all_cores;
static size_t procstatdata_one_core;
static size_t procstatdata_all_cores;

static bool watchdog_flag = false;

static pthread_t tid[THREADS_COUNT];
static pthread_mutex_t watchdog_mutex;

static char* dog_name;

static Watchdog* restrict reader_watchdog; 
static Watchdog* restrict anaylzer_watchdog; 
static Watchdog* restrict printer_watchdog; 
static Watchdog* restrict logger_watchdog; 

static Buffer* restrict logger_buffer;
static Buffer* restrict reader_analyzer_buffer;
static Buffer* restrict analyzer_printer_buffer;

static char* get_current_date(void) {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char* date = malloc(40);
  if (date == NULL) {
    return NULL;
  }

  snprintf(date, 40, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return date;
}

static void watchdog_notify(Watchdog* watchdog) {
  watchdog_lock(watchdog); 
  watchdog_set_flag(watchdog); 
  watchdog_unlock(watchdog); 
}

static void wake_threads(void) {
  buffer_call_consumer(reader_analyzer_buffer);
  buffer_call_producer(reader_analyzer_buffer);
  buffer_call_consumer(analyzer_printer_buffer);
  buffer_call_producer(analyzer_printer_buffer);
  buffer_call_consumer(logger_buffer);
  buffer_call_producer(logger_buffer);
}

void signal_exit(const int signum) {
  logger_log(logger_buffer, "INFO", "SIGNAL", "Signal detected, exiting program.  ", logger_message_size);

  printf(" Signal %d detected, exiting program...\n", signum);

  pthread_mutex_lock(&watchdog_mutex);
  watchdog_flag = true;
  dog_name = "signal";
  pthread_mutex_unlock(&watchdog_mutex);
}

void* reader_thread(void* arg) {
  char* thread_name = "READER";

  (void)arg;

  register Reader* const reader = reader_create("/proc/stat", read_frequency);
  
  while(true) {

    watchdog_lock(reader_watchdog);
    if (watchdog_get_alarm_flag(reader_watchdog) == 1) {
      watchdog_unlock(reader_watchdog);
      break;
    }
    watchdog_unlock(reader_watchdog);

    logger_log(logger_buffer, "INFO", thread_name, "Resetting reader.", logger_message_size);
    reader_reset(reader);
    if (reader->f == NULL) {
      logger_log(logger_buffer, "ERROR", thread_name, "Problem with reopening the file.", logger_message_size);
      return NULL;
    }

    logger_log(logger_buffer, "INFO", thread_name, "Scratching watchdog.", logger_message_size);
    watchdog_scratch(reader_watchdog);

    logger_log(logger_buffer, "INFO", thread_name, "Reading from file.", logger_message_size);
    uint8_t* packet = reader_read(reader, cores, reader_one_core);

    buffer_lock(reader_analyzer_buffer);

    if (buffer_is_full(reader_analyzer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The_buffer is full, waiting for consumer.", logger_message_size);
      buffer_wait_for_consumer(reader_analyzer_buffer);
    }

    watchdog_lock(reader_watchdog);
    if (watchdog_get_alarm_flag(logger_watchdog) == 1) {
      watchdog_unlock(reader_watchdog);
      break;
    }
    watchdog_unlock(reader_watchdog);
    
    logger_log(logger_buffer, "INFO", thread_name, "Sending_packet.", logger_message_size);
    buffer_put(reader_analyzer_buffer, packet, reader_packet_size);
    buffer_call_consumer(reader_analyzer_buffer);

    buffer_unlock(reader_analyzer_buffer);

    logger_log(logger_buffer, "INFO", thread_name, "Sleeping...", logger_message_size);
    free(packet);
    sleep((unsigned int)reader->read_frequency);
  }

  logger_log(logger_buffer, "INFO", thread_name, "Exiting...", logger_message_size);
  printf("Exiting reader...\n");
  reader_destroy(reader);
  return NULL;
}

void* analyzer_thread(void* arg) {
  char* thread_name = "ANALYZER";

  (void)arg;

  bool prev_flag = false;
  uint8_t* prev = malloc(procstatdata_all_cores);
  if (prev == NULL) {
    logger_log(logger_buffer, "ERROR", thread_name, "Problem with mallocing a variable.", logger_message_size);
    return NULL;
  }
    
  while(true) {

    watchdog_lock(anaylzer_watchdog);
    if (watchdog_get_alarm_flag(anaylzer_watchdog) == 1) {
      watchdog_unlock(anaylzer_watchdog);
      break;
    }
    watchdog_unlock(anaylzer_watchdog);

    logger_log(logger_buffer, "INFO", thread_name, "Scratching watchdog.", logger_message_size);
    watchdog_scratch(anaylzer_watchdog);

    buffer_lock(reader_analyzer_buffer);

    if (buffer_is_empty(reader_analyzer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The buffer is empty, waiting for producer.", logger_message_size);
      buffer_wait_for_producer(reader_analyzer_buffer);
    }

    watchdog_lock(anaylzer_watchdog);
    if (watchdog_get_alarm_flag(anaylzer_watchdog) == 1) {
      watchdog_unlock(anaylzer_watchdog);
      break;
    }
    watchdog_unlock(anaylzer_watchdog);

    logger_log(logger_buffer, "INFO", thread_name, "Getting packet.", logger_message_size);
    uint8_t* curr = buffer_get(reader_analyzer_buffer);
    buffer_call_producer(reader_analyzer_buffer);

    buffer_unlock(reader_analyzer_buffer);

    uint8_t* restrict analyzed_packet = malloc(analyzerpacket_all_cores);
    if (analyzed_packet == NULL) {
      logger_log(logger_buffer, "ERROR", thread_name, "Problem with mallocing a variable.", logger_message_size);
      return NULL;
    }

    logger_log(logger_buffer, "INFO", thread_name, "Analyzing...", logger_message_size);
    for (size_t i = 0; i <= cores; ++i) {
      ProcStatData* restrict curr_data = procstatdata_create();
      ProcStatData* restrict prev_data = procstatdata_create();

      sscanf((char*)&curr[i * reader_one_core], 
            "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            curr_data->core_name,
            &curr_data->user,           
            &curr_data->nice,
            &curr_data->system,
            &curr_data->idle,
            &curr_data->iowait,
            &curr_data->irq,
            &curr_data->softirq,
            &curr_data->steal,
            &curr_data->guest,
            &curr_data->guest_nice  
            );

      if (!prev_flag) {
        memcpy(prev_data, curr_data, sizeof(*curr_data));
      } else {
        memcpy(&prev_data[0], &prev[i * procstatdata_one_core], procstatdata_one_core);
      }

      (void)prev;

      AnalyzerPacket* analyzed_core = analyzer_count_cpu_usage(prev_data, curr_data);
      memcpy(&analyzed_packet[i * analyzerpacket_one_core], &analyzed_core[0], analyzerpacket_one_core);

      memcpy(&prev[i * procstatdata_one_core], curr_data, sizeof(*curr_data));

      free(analyzed_core);
      free(curr_data);
      free(prev_data);
    }
    
    watchdog_lock(anaylzer_watchdog);
    if (watchdog_get_alarm_flag(anaylzer_watchdog) == 1) {
      watchdog_unlock(anaylzer_watchdog);
      break;
    }
    watchdog_unlock(anaylzer_watchdog);

    buffer_lock(analyzer_printer_buffer);

    if (buffer_is_full(analyzer_printer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The_buffer is full, waiting for consumer.", logger_message_size);
      buffer_wait_for_consumer(analyzer_printer_buffer);
    }

    watchdog_lock(anaylzer_watchdog);
    if (watchdog_get_alarm_flag(anaylzer_watchdog) == 1) {
      watchdog_unlock(anaylzer_watchdog);
      break;
    }
    watchdog_unlock(anaylzer_watchdog);
    
    logger_log(logger_buffer, "INFO", thread_name, "Sending packet.", logger_message_size);
    buffer_put(analyzer_printer_buffer, analyzed_packet, analyzerpacket_all_cores);
    buffer_call_consumer(analyzer_printer_buffer);

    buffer_unlock(analyzer_printer_buffer);

    prev_flag = true;

    free(curr);
    free(analyzed_packet);
  }

  free(prev);

  logger_log(logger_buffer, "INFO", thread_name, "Exiting...", logger_message_size);
  printf("Exiting analyzer...\n");
  return NULL;
}

void* printer_thread(void* arg) {
  char* thread_name = "PRINTER";

  (void)arg;

  while(true) {

    watchdog_lock(printer_watchdog);
    if (watchdog_get_alarm_flag(printer_watchdog) == 1) {
      watchdog_unlock(printer_watchdog);
      break;
    }
    watchdog_unlock(printer_watchdog);

    logger_log(logger_buffer, "INFO", thread_name, "Scratching watchdog.", logger_message_size);
    watchdog_scratch(printer_watchdog);

    buffer_lock(analyzer_printer_buffer);

    if (buffer_is_empty(analyzer_printer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The buffer is empty, waiting for producer.", logger_message_size);
      buffer_wait_for_producer(analyzer_printer_buffer);
    }

    watchdog_lock(printer_watchdog);
    if (watchdog_get_alarm_flag(printer_watchdog) == 1) {
      watchdog_unlock(printer_watchdog);
      break;
    }
    watchdog_unlock(printer_watchdog);

    logger_log(logger_buffer, "INFO", thread_name, "Getting packet.", logger_message_size);
    uint8_t* packet = buffer_get(analyzer_printer_buffer);
    buffer_call_producer(analyzer_printer_buffer);

    buffer_unlock(analyzer_printer_buffer);

    logger_log(logger_buffer, "INFO", thread_name, "Printing...", logger_message_size);
    printer_print(packet, analyzerpacket_one_core);

    free(packet);
  }

  logger_log(logger_buffer, "INFO", thread_name, "Exiting...", logger_message_size);
  printf("Exiting printer...\n");
  return NULL;
}

void* logger_thread(void* arg) {
  (void)arg;

  register Logger* restrict logger = logger_create();

  while(true) {

    watchdog_lock(logger_watchdog);
    buffer_lock(logger_buffer);
    if (watchdog_get_alarm_flag(logger_watchdog) == 1 && buffer_is_empty(logger_buffer)) {
      buffer_unlock(logger_buffer);
      watchdog_unlock(logger_watchdog);
      break;
    }
    buffer_unlock(logger_buffer);
    watchdog_unlock(logger_watchdog);

    watchdog_scratch(logger_watchdog);

    buffer_lock(logger_buffer);

    if (buffer_is_empty(logger_buffer)) {
      buffer_wait_for_producer(logger_buffer);
    }

    watchdog_lock(logger_watchdog);
    if (watchdog_get_alarm_flag(logger_watchdog) == 1 && buffer_is_empty(logger_buffer)) {
      watchdog_unlock(logger_watchdog);
      break;
    }
    watchdog_unlock(logger_watchdog);

    logger_read(logger, logger_buffer);
    buffer_call_consumer(logger_buffer);

    buffer_unlock(logger_buffer);
  }

  printf("Exiting logger...\n");
  logger_destroy(logger);
  return NULL;
}

void* watchdog_thread(void* arg) {
  char* thread_name = "WATCHDOG";

  (void)arg;

  while(true) {

    watchdog_check(reader_watchdog);
    watchdog_check(anaylzer_watchdog);
    watchdog_check(printer_watchdog);
    watchdog_check(logger_watchdog);

    watchdog_alarm(reader_watchdog);
    watchdog_alarm(anaylzer_watchdog);
    watchdog_alarm(printer_watchdog);
    watchdog_alarm(logger_watchdog);

    pthread_mutex_lock(&watchdog_mutex);
    if (watchdog_flag) {
      char* action = malloc(40);
      if (action == NULL) {
        logger_log(logger_buffer, "ERROR", thread_name, "Problem with mallocing a variable.", logger_message_size);
        return NULL;
      }

      strcpy(action, "Watchdog ");
      strcat(action, dog_name);
      strcat(action, " has raised the alarm.");

      if (strcmp(dog_name, "signal") != 0) {
        logger_log(logger_buffer, "INFO", thread_name, action, logger_message_size);
        free(dog_name);
      }

      free(action);
      pthread_mutex_unlock(&watchdog_mutex);

      watchdog_notify(reader_watchdog); 
      watchdog_notify(anaylzer_watchdog); 
      watchdog_notify(printer_watchdog); 
      watchdog_notify(logger_watchdog); 

      break;
    }
    pthread_mutex_unlock(&watchdog_mutex);

    sleep(1);
  }

  logger_log(logger_buffer, "INFO", thread_name, "Exiting...", logger_message_size);
  printf("Exiting watchdog...\n");
  wake_threads();
  return NULL;
}

void run_threads(void) {
  // CONSTANTS
  cores = (size_t)sysconf(_SC_NPROCESSORS_ONLN);
  reader_packet_size = reader_one_core * (cores + 1);
  read_frequency = 1;
  procstatdata_one_core = sizeof(ProcStatData);
  procstatdata_all_cores = procstatdata_one_core * (cores + 1);
  analyzerpacket_one_core = (sizeof(AnalyzerPacket));
  analyzerpacket_all_cores = analyzerpacket_one_core * (cores + 1);

  // BUFFERS
  reader_analyzer_buffer = buffer_create(reader_packet_size, THREADS_BUFFER_SIZE);
  analyzer_printer_buffer = buffer_create(analyzerpacket_all_cores, THREADS_BUFFER_SIZE);
  logger_buffer = buffer_create(logger_message_size, LOGGER_BUFFER_SIZE);

  // WATCHDOGS
  logger_watchdog = watchdog_create(pthread_self(), "LOGGER", WATCHDOG_TIMER);
  reader_watchdog = watchdog_create(pthread_self(), "READER", WATCHDOG_TIMER);
  anaylzer_watchdog = watchdog_create(pthread_self(), "ANALYZER", WATCHDOG_TIMER);
  printer_watchdog = watchdog_create(pthread_self(), "PRINTER", WATCHDOG_TIMER);

  // WATCHDOG MUTEX
  pthread_mutex_init(&watchdog_mutex, NULL);

  // THREADS
  pthread_create(&tid[0], NULL, reader_thread, NULL);
  pthread_create(&tid[1], NULL, analyzer_thread, NULL);
  pthread_create(&tid[2], NULL, printer_thread, NULL);
  pthread_create(&tid[3], NULL, logger_thread, NULL);
  pthread_create(&tid[4], NULL, watchdog_thread, NULL);

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);
  pthread_join(tid[3], NULL);
  pthread_join(tid[4], NULL);

  buffer_destroy(reader_analyzer_buffer);
  buffer_destroy(analyzer_printer_buffer);
  buffer_destroy(logger_buffer);

  watchdog_destroy(logger_watchdog);
  watchdog_destroy(reader_watchdog);
  watchdog_destroy(anaylzer_watchdog);
  watchdog_destroy(printer_watchdog); 

  pthread_mutex_destroy(&watchdog_mutex);
}
