#include <stdio.h>
#include <stdlib.h>

int main(void) {
	FILE *f = fopen("~/hw3/snoopfs.tar.gz","rw");
	int i;
	for (i = 0; i < 10; ++i) {
	  int u;
	  int newOff = (i + 1)*100;
	  printf("set offset: %d\n", newOff);
	  fseek(f, newOff, SEEK_SET);
	  fscanf(f, "%d", &u);
	}
	fflush(f);
	return 0;
}
