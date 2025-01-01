/*
 * pipe2.c: comunicate between two process with a pipe
 *
 */

#include "kernel/types.h"
#include "user/user.h"

int main()
{
  uint8 buf[256];
  int pid, n = 0;
  int fds[2];

  if( pipe(fds) < 0)
    exit(1);

  pid = fork();
  if (pid == 0) {
    n = read(fds[0], buf, sizeof(buf));
    printf("Child: \n");
    write(1, buf, n);
  } else if (pid > 0) {
    write(fds[1], "hello child, I am your father!\n", 31);
    pid = wait(0);
    printf("Parent: child PID %d return\n", pid);
  } else {
    printf("fork error\n");
  }

  exit(0);
}
