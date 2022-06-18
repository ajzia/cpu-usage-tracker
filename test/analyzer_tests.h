#include "../inc/analyzer.h"
#include <assert.h>
#include <string.h>

static void analyzer_create_test(void) {
  AnalyzerPacket* analyzer = analyzer_create("cpu0");
  assert(analyzer != NULL);

  analyzer_destroy(analyzer);
}

static void analyzer_get_test(void) {
  AnalyzerPacket* analyzer = analyzer_create("cpu0");
  assert(analyzer != NULL);

  assert(strcmp(analyzer_get_core_name(analyzer), "cpu0") == 0);
  assert(analyzer_get_percentage(analyzer) == 0.0);

  analyzer_destroy(analyzer);
}

void analyzer_tests(void) {
  analyzer_create_test();
  analyzer_get_test();
}
