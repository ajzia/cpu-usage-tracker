#include "../inc/analyzer.h"
#include <assert.h>
#include <string.h>

static void analyzer_create_test(void);
static void analyzer_create_test(void) {
  AnalyzerPacket* analyzer = analyzer_create("cpu0");
  assert(analyzer != NULL);

  analyzer_destroy(analyzer);
}

static void analyzer_get_test(void);
static void analyzer_get_test(void) {
  AnalyzerPacket* analyzer = analyzer_create("cpu0");
  assert(analyzer != NULL);
  
  char* name = analyzer_get_core_name(analyzer);
  assert(strcmp(name, "cpu0") == 0);
  assert(analyzer_get_percentage(analyzer) == 0.0);

  free(name);
  analyzer_destroy(analyzer);
}

void analyzer_tests(void);
void analyzer_tests(void) {
  analyzer_create_test();
  analyzer_get_test();
}
