#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define CORE_NAME_PAD 5
#define HISTOGRAM_WIDTH 22
#define PERCENTAGE_PAD 6    // and % at the end
#define HISTOGRAM_SPACES 4
#define HISTOGRAM_CORNER "+"

static void printer_horizontal_bound(void) {
  printf("%s", HISTOGRAM_CORNER);

  size_t characters = CORE_NAME_PAD + HISTOGRAM_WIDTH + PERCENTAGE_PAD + HISTOGRAM_SPACES;
  for (size_t i = 0; i < characters; ++i) {
    printf("-");
  }

  printf("%s\n", HISTOGRAM_CORNER);
}

static void printer_date(void) {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  printf("|      DATE: %d-%02d-%02d %02d:%02d:%02d      |\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  printer_horizontal_bound();
}

static void printer_histogram(const double percent) {
  register double temp = 0.0;
  while (temp <= percent - 0.05) {
    printf("#");
    temp += 0.05;
  }

  while (temp <= 0.95) {
    printf(" ");
    temp += 0.05;
  }
}

void printer_print(register const uint8_t packet[], const size_t packet_size) {
  register const size_t cores = (size_t)sysconf(_SC_NPROCESSORS_ONLN);

  printf("\n");
  printer_horizontal_bound();
  printer_date();
  for (size_t i = 0; i <= cores; ++i) {
    AnalyzerPacket* analyzed_core = malloc(sizeof(*analyzed_core));
    memcpy(analyzed_core, &packet[i * packet_size], sizeof(AnalyzerPacket));

    register double cpu_usage = analyzer_get_percentage(analyzed_core);
    char* name = analyzer_get_core_name(analyzed_core);
    printf("| %*s [", -CORE_NAME_PAD, name);                    

    printer_histogram(cpu_usage);
    cpu_usage *= 100;
    printf("] %*.2lf%% |\n", PERCENTAGE_PAD, cpu_usage);

    free(name);
    free(analyzed_core);
  }

  printer_horizontal_bound(); 
  printf("\n");
}
