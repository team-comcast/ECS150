#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

int test_btDequeue(int syscall_num) {
	return syscall(syscall_num);
}

int main(void) {
	int syscall_num;
	struct module_stat stat;
	int i;
	stat.version = sizeof(stat);

	modstat(modfind("btDequeue"), &stat);
	syscall_num = stat.data.intval;
	int j = 0;
	for (j = 0; j < 10; j++) {
	  i = test_btDequeue(syscall_num);
	  printf("ctorrent: btDequeue=%d\n", i);
	}
}