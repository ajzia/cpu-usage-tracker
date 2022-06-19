#include "../inc/procstat_data.h"
#include <assert.h>

static void procstatdata_create_test(void);
static void procstatdata_create_test(void) {
  ProcStatData* data = procstatdata_create();
  assert(data != NULL);

  procstatdata_destroy(data);
}

void procstat_data_tests(void);
void procstat_data_tests(void) {
  procstatdata_create_test();
}
