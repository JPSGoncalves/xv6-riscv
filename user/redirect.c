/*
 * redirect.c: redirect stdin from a command before exec
 *
 */

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main()
{
  int pid, status;

  pid = fork();
  if (pid == 0) {
    char *argv[2] = {"cat", 0};
    printf("Child: PID %d\n", pid);

    //close parent std input
    close(0);

    //open file as std input for exec cat command
    int fd = open("/test", O_RDONLY);
    if (fd < 0)
      exit(1);

    exec("cat", argv);
    printf("\nexec failed\n");

    exit(1);
  } else if (pid > 0) {
    pid = wait(&status);
    printf("Parent: child PID %d, return status %d\n", pid, status);
  } else {
    printf("fork error\n");
  }

  printf("Parent: exit\n");
  exit(0);
}
