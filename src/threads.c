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

static size_t cores;
static size_t reader_packet_size;
static size_t read_frequency;
static size_t analyzerpacket_one_core;
static size_t analyzerpacket_all_cores;
static size_t procstatdata_one_core;
static size_t procstatdata_all_cores;

static pthread_t tid[THREADS_COUNT];

static Buffer* restrict logger_buffer;
static Buffer* restrict reader_analyzer_buffer;
static Buffer* restrict analyzer_printer_buffer;

static char* get_current_date(void) {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char* date = malloc(40);
  snprintf(date, 40, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return date;
}

void* logger_thread(void* arg) {

  (void)arg;

  register Logger* restrict logger = logger_create();

  while(true) {
    buffer_lock(logger_buffer);

    if (buffer_is_empty(logger_buffer)) {
      buffer_wait_for_producer(logger_buffer);
    }
    logger_read(logger, logger_buffer);
    buffer_call_consumer(logger_buffer);

    buffer_unlock(logger_buffer);
  }

  logger_destroy(logger);
  return NULL;
}

void* reader_thread(void* arg) {
  char* thread_name = "READER";

  (void)arg;

  register Reader* restrict reader = reader_create("/proc/stat", read_frequency);

  size_t counter = 0;
  while(true) {
    counter++;
    reader_reset(reader);
    uint8_t* packet = reader_read(reader, cores, reader_one_core);
    logger_log(logger_buffer, "INFO", thread_name, "Reading_from_file.", logger_message_size);

    buffer_lock(reader_analyzer_buffer);

    if (buffer_is_full(reader_analyzer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The_buffer_is_full,_waiting_for_consumer.", logger_message_size);
      buffer_wait_for_consumer(reader_analyzer_buffer);
    }
    
    logger_log(logger_buffer, "INFO", thread_name, "Sending_packet.", logger_message_size);
    buffer_put(reader_analyzer_buffer, packet, reader_packet_size);
    buffer_call_consumer(reader_analyzer_buffer);

    buffer_unlock(reader_analyzer_buffer);

    logger_log(logger_buffer, "INFO", thread_name, "Sleeping...", logger_message_size);
    free(packet);
    sleep((unsigned int)reader->read_frequency);
  }

  reader_destroy(reader);
  return NULL;
}

void* analyzer_thread(void* arg) {
  char* thread_name = "ANALYZER";

  (void)arg;

  bool prev_flag = false;
  uint8_t* prev = malloc(procstatdata_all_cores);

  size_t counter = 0;
  while(true) {
    counter++;
    buffer_lock(reader_analyzer_buffer);

    if (buffer_is_empty(reader_analyzer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The_buffer_is_empty,_waiting_for_producer.", logger_message_size);
      buffer_wait_for_producer(reader_analyzer_buffer);
    }

    logger_log(logger_buffer, "INFO", thread_name, "Getting_packet.", logger_message_size);
    uint8_t* curr = buffer_get(reader_analyzer_buffer);
    buffer_call_producer(reader_analyzer_buffer);

    buffer_unlock(reader_analyzer_buffer);

    uint8_t* restrict analyzed_packet = malloc(analyzerpacket_all_cores);

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
    

    buffer_lock(analyzer_printer_buffer);

    if (buffer_is_full(analyzer_printer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The_buffer_is_full,_waiting_for_consumer.", logger_message_size);
      buffer_wait_for_consumer(analyzer_printer_buffer);
    }
    
    logger_log(logger_buffer, "INFO", thread_name, "Sending_packet.", logger_message_size);
    buffer_put(analyzer_printer_buffer, analyzed_packet, analyzerpacket_all_cores);
    buffer_call_consumer(analyzer_printer_buffer);

    buffer_unlock(analyzer_printer_buffer);

    prev_flag = true;

    free(curr);
    free(analyzed_packet);
  }

  free(prev);

  return NULL;
}

void* printer_thread(void* arg) {
  char* thread_name = "READER";

  (void)arg;

  size_t counter = 0;
  while(true) {
    counter++;

    buffer_lock(analyzer_printer_buffer);

    if (buffer_is_empty(analyzer_printer_buffer)) {
      logger_log(logger_buffer, "INFO", thread_name, "The_buffer_is_empty,_waiting_for_producer.", logger_message_size);
      buffer_wait_for_producer(analyzer_printer_buffer);
    }

    logger_log(logger_buffer, "INFO", thread_name, "Getting_packet.", logger_message_size);
    uint8_t* packet = buffer_get(analyzer_printer_buffer);
    buffer_call_producer(analyzer_printer_buffer);

    buffer_unlock(analyzer_printer_buffer);

    logger_log(logger_buffer, "INFO", thread_name, "Printing...", logger_message_size);
    printer_print(packet, analyzerpacket_one_core);

    free(packet);
  }

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

  // THREADS
  pthread_create(&tid[0], NULL, reader_thread, NULL);
  pthread_create(&tid[1], NULL, analyzer_thread, NULL);
  pthread_create(&tid[2], NULL, printer_thread, NULL);
  pthread_create(&tid[3], NULL, logger_thread, NULL);
  // pthread_create(&tid[4], NULL, watchdog_thread, NULL);

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);
  pthread_join(tid[3], NULL);
  // pthread_join(tid[4], NULL);

  buffer_destroy(reader_analyzer_buffer);
  buffer_destroy(analyzer_printer_buffer);
  buffer_destroy(logger_buffer);
}
