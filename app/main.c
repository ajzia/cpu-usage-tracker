#include "../inc/threads.h"
#include <signal.h>
#include <string.h>

int main(void) {
  printf("\n           CPU-USAGE-TRACKER\n");
  printf("             Joanna Kulig\n");
  printf("              20.06.2022\n");

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = signal_exit;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  run_threads();
}
