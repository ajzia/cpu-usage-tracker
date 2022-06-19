#ifndef PROCSTATDATA_H
#define PROCSTATDATA_H

#define NAME_LEN 8

typedef struct ProcStatData {
  char core_name[NAME_LEN];
  double user;
  double nice;
  double system;
  double idle;
  double iowait;
  double irq;
  double softirq;
  double steal;
  double guest;
  double guest_nice;
} ProcStatData;

ProcStatData* procstatdata_create(void);

void procstatdata_print(register const ProcStatData* data);

void procstatdata_destroy(ProcStatData* data);

#endif
