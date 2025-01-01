/*
 * pipe1.c: reading and writing from a pipe
 *
 */

#include "kernel/types.h"
#include "user/user.h"

int main()
{
  uint8 buf[256];
  int fds[2];
  int n = 0;

  if( pipe(fds) < 0)
    exit(1);

  write(fds[1], "hello pipe!\n", 13);

  n = read(fds[0], buf, sizeof(buf));

  write(1, buf, n);

  exit(0);
}
