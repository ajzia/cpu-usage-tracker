#ifndef ANALYZER_H
#define ANALYZER_H

#define NAME_LEN 10

typedef struct AnalyzerPacket {
  char core_name[NAME_LEN];
  double percentage;
} AnalyzerPacket;

AnalyzerPacket* analyzer_create(const char* restrict core_name);

void analyzer_destroy(AnalyzerPacket* analyzer);

#endif
