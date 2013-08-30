
/* read-only version of t.c             */
/* for testing HW4, ecs150, winter 2010 */

#include <stdio.h>
#include <stdlib.h>

int
main
(int argc, char *argv[])
{
  FILE *target = NULL;
  int i, value;

  if (argc != 2)
    {
      fprintf(stderr, "usage: $ %s <downloading-snoopfs-file-name>\n",
	      argv[0]);
      exit(0);
    }

  target = fopen(argv[1], "r");
  if (target == NULL)
    {
      fprintf(stderr, "$ %s %s File open error!\n",
	      argv[0], argv[1]);
      exit(-1);
    }

  for (i = 0; i < 1000; i++)
    {
      fseek(target, rand(), SEEK_SET);
      fscanf(target, "%d", &value);
      if (i % 100 == 0)
	{
	  fprintf(stderr, ".");
	  sleep(1);
	}
    }
  fprintf(stderr, "\n");
  fclose(target);
  return 0;
}
