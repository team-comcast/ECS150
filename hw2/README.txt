README.txt

Joshua Vaughen, 996696191
jshvn, jshvn@ucdavis.edu
hw1.c kern_fork.c kern_switch.c Makefile proc.h README.txt
HW2
ECS150 - Operating systems
4/27/13

General logic: we are adding 4 new kernel modules that will get and set a global variable (which
will later be used to change scheduling behavior) and the number of tickets for each process.
this only required modification of kern_switch.c and proc.h, and of course the definition
of the kernel modules that we wanted to load in. 

The scheduling logic works as follows: if lottery mode is off, use the normal BSD scheduling 
methods. Otherwise, if it is on, and the particular entity which we are looking at has
a priority between 40 and 55, meaning in the userspace, go ahead and use lottery scheduling.
Iterate through all of entities and sum up the total number of tickets to be used in the
lottery computation. Then generate a random number, and iterate through the entities again
and continuously subtract the number of tickets of those individual processes from the
total number of tickets we have. This accomplishes a randomized lottery style scheduling
because it will randomly stop on a particular process once the number of tickets have run out
and give it CPU time. 

hw1.c :

	includes all of the modules that were necessary for this assignment.

	modules:
		getProcessTickets: grabs a particular process's (based on passed in PID) number of 
		tickets and returns to the thread that called the module the number of tickets
		that particular process has. return -1 if that particular PID could not be found

		setProcesstickets: based on the passed in process PID, change that particular 
		process's number of tickets to the number that is passed in as a second argument.
		then return to the thread that called the module the number of tickets they just
		set. return -1 if the particular PID couldnt be found

		getLotteryMode: check the global variable lottery_mode and return its current value 
		to the calling thread.

		setLotteryMode: change the current value of lottery_mode to the passed in value

kern_switch.c :
	
	this has been heavily documented inline, but the basic logic is the same as what was above.
	used TAILQ macros to iterate through the list of entities, use a random number and subtraction
	to accomplish the lottery style scheduling. please read the inline comments.

kern_fork.c :
	
	added one line that will set the initial number of tickets for a given process (via fork)
	to four. 

proc.h :
	
	added int tickets to the process structure for use, and linked to the external value
	lottery_mode. the tickets will be used in the later homework for lottery mode
	scheduling.

Makefile :
	
	this simply defines the file to compile and generate the kernel module from. i simply
	included all four kernel modules in the same file, so my makefile is rather simple and just
	loads from that particular file.