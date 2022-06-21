#include "../inc/procstat_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

ProcStatData* procstatdata_create(void) {
  ProcStatData* const data = malloc(sizeof(*data));
  if (data == NULL)
    return NULL;

  *data = (ProcStatData){
                         .user = 0.0,
                         .nice = 0.0,
                         .system = 0.0,
                         .idle = 0.0,
                         .iowait = 0.0,
                         .irq = 0.0,
                         .softirq = 0.0,
                         .steal = 0.0,
                         .guest = 0.0,
                         .guest_nice = 0.0,
                        };

  return data;
}

void procstatdata_destroy(ProcStatData* const data) {
  if (data == NULL)
    return;

  free(data);
}
