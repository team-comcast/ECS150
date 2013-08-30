/*
 * Copyright (c) 1997-2005 Erez Zadok <ezk@cs.stonybrook.edu>
 * Copyright (c) 2001-2005 Stony Brook University
 *
 * For specific licensing information, see the COPYING file distributed with
 * this package, or get one from ftp://ftp.filesystems.org/pub/fistgen/COPYING.
 *
 * This Copyright notice must be kept intact and distributed with all
 * fistgen sources INCLUDING sources generated by fistgen.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#ifdef FISTGEN
# include "fist_snoopfs.h"
#endif /* FISTGEN */
#include <fist.h>
#include <snoopfs.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <snoopfs.h>

int
main(int argc, char *argv[])
{
  int fd;

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s file [val]\n", argv[0]);
    exit(1);
  }

  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror(argv[1]);
    exit(1);
  }

  close(fd);
  exit(0);
}
