
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

int test_setProcessTickets
(int p, int t, int syscall_num)
{
  return syscall(syscall_num, p, t);
}

int test_getProcessTickets
(int p, int syscall_num)
{
  return syscall(syscall_num, p);
}

int test_setLotteryMode(int t, int syscall_num)
{
  return syscall(syscall_num, t);
}

int test_getLotteryMode(int syscall_num)
{
  return syscall(syscall_num);
}

int main
(void)
{
  int syscall_num[4];
  struct module_stat stat;
  int i;
  stat.version = sizeof(stat);

// these two lines look up the syscall number given
// to the system call in question, so we can later
// call it using this number. 
  modstat(modfind("setProcessTickets"), &stat);
  syscall_num[0] = stat.data.intval;

  for(i=40; i<60; i+=2) {
    test_setProcessTickets(i, i+1, syscall_num[0]);
  }

  modstat(modfind("getProcessTickets"), &stat);
  syscall_num[1] = stat.data.intval;
  int k=0;
  for(i=40; i<60; i+=2) {
    k = test_getProcessTickets(i, syscall_num[1]);
    if( k != -1)
    printf("pid=%d exists, tickets=%d\n", i, k);
      else
          printf("pid=%d does not exists, tickets=%d\n", i, k);
  }

  modstat(modfind("setLotteryMode"), &stat);
  syscall_num[2] = stat.data.intval;
  modstat(modfind("getLotteryMode"), &stat);
  syscall_num[3] = stat.data.intval;

  test_setLotteryMode(2, syscall_num[2]);
  k = test_getLotteryMode(syscall_num[3]);
  printf("lotterymode is set to %d\n", k);
  test_setLotteryMode(99, syscall_num[2]);
  k = test_getLotteryMode(syscall_num[3]);
  printf("lotterymode is set to %d\n", k);
  test_setLotteryMode(9, syscall_num[2]);
  k = test_getLotteryMode(syscall_num[3]);
  printf("lotterymode is set to %d\n", k);
  test_setLotteryMode(1, syscall_num[2]);
  k = test_getLotteryMode(syscall_num[3]);
  printf("lotterymode is set to %d\n", k);

}

