
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>

static int getProcessTickets(struct thread *t, void *arg) {
  printf("hello getprocess\n");
  int x = ((int *)arg)[0];	
  struct proc *p1 = pfind(x);
  if (p1 != NULL) {
      int y = p1->tickets;
      PROC_UNLOCK(p1);
      t->td_retval[0] = y;
  } else {
      t->td_retval[0] = -1;
  }
  return 0;
}
  

static int setProcessTickets(struct thread *t, void *arg) {
  printf("hello setprocess\n");
  int x = ((int *)arg)[0];
  struct proc *p1 = pfind(x);
  if (p1 != NULL) {
    p1->tickets = 5;
    PROC_UNLOCK(p1); 
    t->td_retval[0] = 5;
  } else {
    t->td_retval[0] = -1;
  }
  return 0;
}

static int setLotteryMode(struct thread *t, void *arg) {
  return 0;
}
static int getLotteryMode(struct thread *t, void *arg) {
  return 0;
}


static struct sysent getProcessTickets_sysent = { 1, getProcessTickets };
static struct sysent setProcessTickets_sysent = { 1, setProcessTickets };
static struct sysent setLotteryMode_sysent = { 1, setLotteryMode };
static struct sysent getLotteryMode_sysent = { 1, getLotteryMode };


static int offset = NO_SYSCALL;
static int offset2 = NO_SYSCALL;
static int offset3 = NO_SYSCALL;
static int offset4 = NO_SYSCALL;

static int load(struct module *module, int cmd, void *arg) {
	int error = 0;
	switch (cmd) {
	case MOD_LOAD:
		printf("syscall loaded at %d\n", offset);
		break;
	case MOD_UNLOAD:
		printf("syscall unloaded at %d\n", offset);
		break;
	default:
		error = EINVAL;
		break;
	}
	return error;
}

static int load2(struct module *module, int cmd, void *arg) {
  int error = 0;
  switch (cmd) {
  case MOD_LOAD:
    printf("syscall loaded at %d\n", offset2);
    break;
  case MOD_UNLOAD:
    printf("syscall unloaded at %d\n", offset2);
    break;
  default:
    error = EINVAL;
    break;
  }
  return error;
}

static int load3(struct module *module, int cmd, void *arg) {
  int error = 0;
  switch (cmd) {
  case MOD_LOAD:
    printf("syscall loaded at %d\n", offset3);
    break;
  case MOD_UNLOAD:
    printf("syscall unloaded at %d\n", offset3);
    break;
  default:
    error = EINVAL;
    break;
  }
  return error;
}

static int load4(struct module *module, int cmd, void *arg) {
  int error = 0;
  switch (cmd) {
  case MOD_LOAD:
    printf("syscall loaded at %d\n", offset4);
    break;
  case MOD_UNLOAD:
    printf("syscall unloaded at %d\n", offset4);
    break;
  default:
    error = EINVAL;
    break;
  }
  return error;
}



SYSCALL_MODULE(getProcessTickets, &offset, &getProcessTickets_sysent, load, NULL);
SYSCALL_MODULE(setProcessTickets, &offset2, &setProcessTickets_sysent, load2, NULL);
SYSCALL_MODULE(setLotteryMode, &offset3, &setLotteryMode_sysent, load3, NULL);
SYSCALL_MODULE(getLotteryMode, &offset4, &getLotteryMode_sysent, load4, NULL);
