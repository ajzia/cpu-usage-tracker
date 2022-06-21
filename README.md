## CPU USAGE TRACKER

Recruitment task for [Tietoevry](https://www.tietoevry.com/).
Author: Joanna Kulig

### Compilation:

To run the program you have to be in a project directory.

```sh
# main program
$ make
$ ./main.out
```

```sh
# tests
$ make MODE=test
$ ./test/test.out
```

### About:

The task was to make a CPU usage tracker - a /proc/stat analyzer with multiple threads.

The program had to run on 5 threads:

- **Reader** reads /proc/stat and sends read characters as raw data to the **Analyzer**.

- **Analyzer** processes data and computes CPU usage for every core in /proc/stat and sends it to the **Printer**.

- **Printer** prints formatted average use of CPU per second.

- **Watchdog** makes sure that program does not get stuck. To acomplish that, every thread has to send information to the Watchdog. If they did not do that for more than 2 seconds, the program ends with appropriate error message.

- **Logger** receives debug prints from each thread and saves it to a file in a synchronized manner.


It also had to fulfill some requirements:

- The project had to be written in *C* language in standard *C99* or higher.

- There had to be no warnings while compiling (clang with -Weverything, gcc with -Wall -Wextra).

- It had to have building system, preferably *Makefile* or *CMake*.

- The project had to have a github repository with "nice" commit history.

- It had to be tested for leaks with *valgrind*.
