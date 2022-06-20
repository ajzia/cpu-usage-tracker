#include "../inc/threads.h"
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#define one_core_size 250 
#define THREADS_COUNT 5

static size_t cores;
static size_t reader_packet_size;
static size_t read_frequency;
static size_t analyzerpacket_one_core;
static size_t analyzerpacket_all_cores;
static size_t procstatdata_one_core;
static size_t procstatdata_all_cores;

static pthread_t tid[THREADS_COUNT];

static Buffer* restrict reader_analyzer_buffer;
static Buffer* restrict analyzer_printer_buffer;

void* reader_thread(void* arg) {

  (void)arg;

  register Reader* restrict reader = reader_create("/proc/stat", read_frequency);

  size_t counter = 0;
  while(counter != 3) {
    counter++;
    reader_reset(reader);
    uint8_t* packet = reader_read(reader, cores, one_core_size);

    buffer_lock(reader_analyzer_buffer);

    if (buffer_is_full(reader_analyzer_buffer)) {
      buffer_wait_for_consumer(reader_analyzer_buffer);
    }
    
    buffer_put(reader_analyzer_buffer, packet, reader_packet_size);
    buffer_call_consumer(reader_analyzer_buffer);

    buffer_unlock(reader_analyzer_buffer);

    free(packet);
    sleep((unsigned int)reader->read_frequency);
  }

  reader_destroy(reader);
  return NULL;
}

void* analyzer_thread(void* arg) {

  (void)arg;

  bool prev_flag = false;
  uint8_t* prev = malloc(procstatdata_all_cores);

  size_t counter = 0;
  while(counter != 3) {
    counter++;
    buffer_lock(reader_analyzer_buffer);

    if (buffer_is_empty(reader_analyzer_buffer)) {
      buffer_wait_for_producer(reader_analyzer_buffer);
    }
    uint8_t* curr = buffer_get(reader_analyzer_buffer);

    buffer_call_producer(reader_analyzer_buffer);

    buffer_unlock(reader_analyzer_buffer);


    uint8_t* restrict analyzed_packet = malloc(analyzerpacket_all_cores);

    for (size_t i = 0; i <= cores; ++i) {
      ProcStatData* restrict curr_data = procstatdata_create();
      ProcStatData* restrict prev_data = procstatdata_create();

      sscanf((char*)&curr[i * one_core_size], 
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
      buffer_wait_for_consumer(analyzer_printer_buffer);
    }
    
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

  (void)arg;

  size_t counter = 0;
  while(counter != 3) {
    counter++;

    buffer_lock(analyzer_printer_buffer);

    if (buffer_is_empty(analyzer_printer_buffer)) {
      buffer_wait_for_producer(analyzer_printer_buffer);
    }
    uint8_t* packet = buffer_get(analyzer_printer_buffer);

    buffer_call_producer(analyzer_printer_buffer);

    buffer_unlock(analyzer_printer_buffer);

    printer_print(packet, analyzerpacket_one_core);

    free(packet);
  }

  return NULL;
}


void run_threads(void) {
  // CONSTANTS
  cores = (size_t)sysconf(_SC_NPROCESSORS_ONLN);
  reader_packet_size = one_core_size * (cores + 1);
  read_frequency = 1;
  procstatdata_one_core = sizeof(ProcStatData);
  procstatdata_all_cores = procstatdata_one_core * (cores + 1);
  analyzerpacket_one_core = (sizeof(AnalyzerPacket));
  analyzerpacket_all_cores = analyzerpacket_one_core * (cores + 1);

  // BUFFERS
  reader_analyzer_buffer = buffer_create(reader_packet_size, 10);
  analyzer_printer_buffer = buffer_create(analyzerpacket_all_cores, 10);

  // THREADS
  pthread_create(&tid[0], NULL, reader_thread, NULL);
  pthread_create(&tid[1], NULL, analyzer_thread, NULL);
  pthread_create(&tid[2], NULL, printer_thread, NULL);

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);

  buffer_destroy(reader_analyzer_buffer);
  buffer_destroy(analyzer_printer_buffer);
}
