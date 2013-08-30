README.txt

Joshua Vaughen
996696191
HW1
ECS150 - Operating systems
4/16/13

General logic: we are adding 4 new kernel modules that will get and set a global variable (which
will later be used to change scheduling behavior) and the number of tickets for each process.
this only required modification of kern_switch.c and proc.h, and of course the definition
of the kernel modules that we wanted to load in. 

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
	
	all that was added to this file was the lottery_mode integer as a global variable. 
	the lottery_mode will be used in a future homework assignment to set process
	scheduling behavior

proc.h :
	
	added int tickets to the process structure for use, and linked to the external value
	lottery_mode. the tickets will be used in the later homework for lottery mode
	scheduling.

Makefile :
	
	this simply defines the file to compile and generate the kernel module from. i simply
	included all four kernel modules in the same file, so my makefile is rather simple and just
	loads from that particular file.