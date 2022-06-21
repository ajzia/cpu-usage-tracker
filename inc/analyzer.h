#ifndef ANALYZER_H
#define ANALYZER_H
#include "../inc/procstat_data.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct AnalyzerPacket {
  char core_name[NAME_LEN];
  double percentage;
} AnalyzerPacket;

AnalyzerPacket* analyzer_create(const char* core_name);
void analyzer_destroy(AnalyzerPacket* analyzer);

char* analyzer_get_core_name(const AnalyzerPacket* analyzer);
double analyzer_get_percentage(const AnalyzerPacket* analyzer);

AnalyzerPacket* analyzer_count_cpu_usage(const ProcStatData* restrict prev, const ProcStatData* restrict curr);

#endif
