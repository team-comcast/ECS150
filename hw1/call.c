#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>
#include <sys/unistd.h>


int test_getProcessTickets
( int pid, int syscall_num)
{
	return syscall(syscall_num, pid);
}
int main()
{
	int syscallnum;
	struct module_stat stat;
	int x;
	int retval= 0;
	x = getpid();
	stat.version = sizeof(stat);
	modstat(modfind("getProcessTickets"), &stat);
	syscallnum = stat.data.intval;
	printf("x = %d\n", x);
	printf("syscall num = %d\n", syscallnum);
	retval = test_getProcessTickets(x, syscallnum);
	printf("return value = %d\n", retval);
	return 0;
}
