#include "../inc/analyzer.h"
#include <string.h>
#include <stdlib.h>

AnalyzerPacket* analyzer_create(const char* restrict core_name) {
  if (core_name == NULL)
    return NULL;

  size_t name_length = strlen(core_name);
  if (name_length > NAME_LEN || name_length == 0)
    return NULL;

  AnalyzerPacket* packet = malloc(sizeof(*packet));

  strcpy(&packet->core_name[0], core_name);
  packet->percentage = 0.0;

  return packet;
}

AnalyzerPacket* analyzer_analyze_cpu_usage() {

  return NULL;
}

void analyzer_destroy(AnalyzerPacket* const analyzer) {
  free(analyzer);
}

