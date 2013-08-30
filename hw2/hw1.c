// Joshua Vaughen
// 996696191
// HW1
// ECS150 - Operating systems
// 4/16/13

#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>

// getProcessTickets which simply accesses the process that was passed in by PID
// and grabs the number of tickets that process currently has. It then places this
// in the thread (that called the function) return value to return to the calling
// program. -1 is returned if the process couldn't be found
static int getProcessTickets(struct thread *t, void *arg) {
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
 
// setProcessTickets which accesses the process that was passed in by the PID and
// sets the current number of tickets to be the number passed in by the second
// argument. It then places this in the thread return value to return to the calling
// program. -1 is returned if the process couldn't be found
static int setProcessTickets(struct thread *t, void *arg) {
  int x = ((int *)arg)[0];
  struct proc *p1 = pfind(x);
  if (p1 != NULL) {
      p1->tickets = ((int *)arg)[1];
      PROC_UNLOCK(p1); 
      t->td_retval[0] = p1->tickets;
  } else {
      t->td_retval[0] = -1;
  }
  return 0;
}

// setLotteryMode which simply modifies the lottery_mode global variable to the 
// given number specified by the only argument. The return value is the value that
// was specified in the argument
static int setLotteryMode(struct thread *t, void *arg) {
    lottery_mode = ((int *)arg)[0];
    t->td_retval[0] = lottery_mode;
    return 0;
}

// getLotteryMode which simply accesses the lottery_mode global variable and returns
// that value to the calling thread for use
static int getLotteryMode(struct thread *t, void *arg) {
    t->td_retval[0] = lottery_mode;
    return 0;
}

// define the sysent structures, which essentially just describe the system call
// number of parameters, the name
static struct sysent getProcessTickets_sysent = { 1, getProcessTickets };
static struct sysent setProcessTickets_sysent = { 2, setProcessTickets };
static struct sysent setLotteryMode_sysent = { 1, setLotteryMode };
static struct sysent getLotteryMode_sysent = { 0, getLotteryMode };

// the offsets represent the slots that the system call will be using. the 
// NO_SYSCALL basically just says let the kernel choose the call number
static int offset = NO_SYSCALL;
static int offset2 = NO_SYSCALL;
static int offset3 = NO_SYSCALL;
static int offset4 = NO_SYSCALL;

// load which just prints out what the kernel selected offset for the particular
// module is on both load and unload (via kldload/kldunload)
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

// same as load, different syscall
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

// same as load, different syscall
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

// same as load, different syscall
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


// SYSCALL_MODULE is a macro that actually installs the module using the name, 
// its offset, the descriptor, a particular onload/unload event, and args
SYSCALL_MODULE(getProcessTickets, &offset, &getProcessTickets_sysent, load, NULL);
SYSCALL_MODULE(setProcessTickets, &offset2, &setProcessTickets_sysent, load2, NULL);
SYSCALL_MODULE(setLotteryMode, &offset3, &setLotteryMode_sysent, load3, NULL);
SYSCALL_MODULE(getLotteryMode, &offset4, &getLotteryMode_sysent, load4, NULL);
