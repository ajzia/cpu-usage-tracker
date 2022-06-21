#include "../inc/analyzer.h"
#include "../inc/procstat_data.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

AnalyzerPacket* analyzer_create(const char* core_name) {
  if (core_name == NULL)
    return NULL;

  size_t name_length = strlen(core_name);
  if (name_length > NAME_LEN || name_length == 0)
    return NULL;

  AnalyzerPacket* const packet = malloc(sizeof(*packet));
  if (packet == NULL)
    return NULL;

  strcpy(&packet->core_name[0], core_name);
  packet->percentage = 0.0;

  return packet;
}

char* analyzer_get_core_name(const AnalyzerPacket* const analyzer) {
  if (analyzer == NULL)
    return NULL;

  char* name = malloc(sizeof(analyzer->core_name));
    if (name == NULL)
    return NULL;

  memcpy(name, analyzer->core_name, sizeof(analyzer->core_name));

  return name;
}

double analyzer_get_percentage(const AnalyzerPacket* const analyzer) {
  return analyzer->percentage;
}

AnalyzerPacket* analyzer_count_cpu_usage(register const ProcStatData* const restrict prev, register const ProcStatData* const restrict curr) {
  if (prev == NULL)
    return NULL;

  register AnalyzerPacket* const packet = analyzer_create(curr->core_name);

  if (curr == NULL) {
    packet->percentage = 0.0;
    return packet;
  }

  if (strcmp(prev->core_name, curr->core_name) != 0) 
    return NULL; 

  register const double prev_idle = prev->idle + prev->iowait;
  register const double curr_idle = curr->idle + curr->iowait;

  register const double prev_non_idle = prev->user + prev->nice + prev->system + prev->irq + prev->softirq + prev->steal;
  register const double curr_non_idle = curr->user + curr->nice + curr->system + curr->irq + curr->softirq + curr->steal;

  register const double prev_total = prev_idle + prev_non_idle;
  register const double curr_total = curr_idle + curr_non_idle;

  register const double total_diff = curr_total - prev_total;
  register const double idle_diff = curr_idle - prev_idle;

  packet->percentage = total_diff == 0.0 ? 0.0 : (total_diff - idle_diff) / total_diff;

  return packet;
}

void analyzer_destroy(AnalyzerPacket* const analyzer) {
  if (analyzer == NULL)
    return;

  free(analyzer);
}
