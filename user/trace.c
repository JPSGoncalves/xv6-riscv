/*
 * trace.c: trace syscalls
 * First agument is bitmask of syscalls to trace
 */

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int tracemask, pid;

  if (argc < 2)
    exit(1);

  tracemask = atoi(argv[1]);

  trace(tracemask);

  pid = fork();
  if (pid == 0) {
    exec(argv[2], &argv[3]);
    printf("exec failed\n");
    exit(1);
  } else if (pid > 0) {
    pid = wait(0);
  } else {
    printf("fork error\n");
  }

  trace(0);
  exit(0);
}
