/*
 * fork.c: create a file
 *
 */

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int pid;

  pid = fork();
  if (pid == 0) {
    printf("Hello from child: PID %d\n", pid);
  } else if (pid > 0) {
    pid = wait(0);
    printf("Hello from parent: child PID %d return\n", pid);
  } else {
    printf("fork error\n");
  }

  printf("Bye bye from PID %d\n", pid);
  exit(0);
}
