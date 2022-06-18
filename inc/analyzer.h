#ifndef ANALYZER_H
#define ANALYZER_H
#include "../inc/procstat_data.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct AnalyzerPacket {
  char core_name[NAME_LEN];
  double percentage;
} AnalyzerPacket;

AnalyzerPacket* analyzer_create(const char* restrict core_name);

char* analyzer_get_core_name(const AnalyzerPacket* restrict analyzer);

double analyzer_get_percentage(const AnalyzerPacket*  restrict analyzer);

AnalyzerPacket* analyzer_count_cpu_usage(const ProcStatData* prev, const ProcStatData* curr);

void analyzer_destroy(AnalyzerPacket* analyzer);

#endif
