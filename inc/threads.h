#ifndef THREADS_H
#define THREADS_H

#include "buffer.h"
#include "reader.h"
#include "analyzer.h"
#include "printer.h"
#include "logger.h"
#include "watchdog.h"

void run_threads(void);

void signal_exit(int signum);

#endif
