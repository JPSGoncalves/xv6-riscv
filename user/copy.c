/*
 * copy.c: copy from std input and write to stdout
 *
 */

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static uint8 buf[512];

int main(int argc, char *argv[])
{
  int n;

  for(;;) {
    n = read(0, buf, sizeof(buf));

    printf("read: %d bytes from stdin, buf[0] %d\n", n, buf[0]);

    if ((n == 0) || ((n == 1) && buf[0] == '\n'))
      break;

    if (n > 0)
      write(1, buf, n);
  }

  exit(0);
}
