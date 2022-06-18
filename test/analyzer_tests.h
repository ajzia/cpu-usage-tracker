#include "../inc/analyzer.h"
#include <assert.h>

static void analyzer_create_test() {
  AnalyzerPacket* analyzer = analyzer_create("cpu0");
  assert(analyzer != NULL);

  analyzer_destroy(analyzer);
}

void analyzer_tests(void) {
  analyzer_create_test();
}
