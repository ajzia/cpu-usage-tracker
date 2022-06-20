#ifndef THREADS_H
#define THREADS_H

#include "buffer.h"
#include "reader.h"
#include "analyzer.h"
#include "printer.h"
#include "logger.h"
#include "watchdog.h"

void* logger_thread(void* arg);

void* reader_thread(void* arg);

void* analyzer_thread(void* arg);

void* printer_thread(void* arg);

void run_threads(void);

#endif
