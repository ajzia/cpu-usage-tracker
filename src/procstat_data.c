#include "../inc/procstat_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

ProcStatData* procstatdata_create(void) {
  ProcStatData* data = malloc(sizeof(*data));
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

void procstatdata_print(register const ProcStatData* const data) {
    printf("%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", 
           data->core_name,
           data->user,           
           data->nice,
           data->system,
           data->idle,
           data->iowait,
           data->irq,
           data->softirq,
           data->steal,
           data->guest,
           data->guest_nice 
           );

}

void procstatdata_destroy(ProcStatData* const data) {
  free(data);
};