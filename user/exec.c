/*
 * exec.c: make process to execute another program
 *
 */

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int err = 0;
  char *exec_args[3];

  exec_args[0] = "cat";
  exec_args[1] = argv[2];
  exec_args[2] = 0;

  err = exec("cat", exec_args);
  if (err)
      printf("exec failed");

  exit(err);
}
