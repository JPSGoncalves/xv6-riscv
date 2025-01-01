/*
 * open.c: create a file
 *
 */

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int fd;

  fd = open("test", O_RDWR | O_CREATE);

  if (fd < 0)
    exit(1);

  write(fd, "hello test file!\n", 18);

  close(fd);
  exit(0);
}
