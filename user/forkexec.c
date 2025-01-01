/*
 * forkexec.c: create a file
 *
 */

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int pid, status;

  if (argc < 3)
    exit(1);

  pid = fork();
  if (pid == 0) {
    printf("Hello from child: PID %d, executing command: ", pid);

    for(int i = 1; i < argc; i++)
      printf("%s ", argv[i]);
    printf("\n");

    exec(argv[1], &argv[2]);
    printf("exec failed\n");
    exit(1);
  } else if (pid > 0) {
    pid = wait(&status);
    printf("Parent: child PID %d, return status %d\n", pid, status);
  } else {
    printf("fork error\n");
  }

  printf("Bye bye from parent \n");
  exit(0);
}
