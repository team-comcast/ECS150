
/* test program 2 for hw1, ecs150, winter 2006 */
/* gcc call.c -o call */

/*
 * sample test output:
 * rc_get before set = -1
 * rc_set            = -1 (rand = 17284)
 * rc_get after  set = -1
 */

#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

int get_num = 0;
int set_num = 0;

int
getProcessTickets
(int pid)
{
  if (get_num == 0)
    return -2;
  else
    return syscall(get_num, pid);
}

int
setProcessTickets
(int pid, int tickets)
{
  if (set_num == 0)
    return -2;
  else
    return syscall(set_num, pid, tickets);
}

int
main(void)
{
  int my_pid = getpid();
  int parent = getppid();
  int rc_get, rc_set;
  int myrand = rand();
  struct module_stat stat_get;
  struct module_stat stat_set;

  stat_get.version = sizeof(stat_get);
  modstat(modfind("getProcessTickets"), &stat_get);
  get_num = stat_get.data.intval;

  stat_set.version = sizeof(stat_set);
  modstat(modfind("setProcessTickets"), &stat_set);
  set_num = stat_set.data.intval;


  printf("get_num %d set_num %d \n", get_num, set_num);

  rc_get = getProcessTickets(my_pid);
  printf("rc_get before set = %d\n", rc_get);

  rc_set = setProcessTickets(my_pid, myrand);
  printf("rc_set            = %d (pid = %d)\n", rc_set, myrand);

  rc_get = getProcessTickets(my_pid);
  printf("rc_get after  set = %d\n", rc_get);

  return 0;
}
