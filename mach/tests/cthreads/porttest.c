/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	porttest.c,v $
 * Revision 1.2  89/05/05  19:03:49  mrt
 * 	Cleanup for Mach 2.5
 * 
 */

#include <stdio.h>
#include <cthreads.h>

mutex_t lock;
boolean_t cthread_debug;

printport(errcode,port)
   kern_return_t	errcode;
   port_t		port;

{	/* must avoid malloc here */
		char buf[100];
		int n;
		sprintf(buf, "retcode is %d port is %d\n",
			errcode, (int)port);
		write(1, buf, strlen(buf));
}
proc(name)
	char name;
{
	kern_return_t retcode;
	vm_address_t  data;
	port_t		port;
	int		i;

#ifdef	DEBUG
	if (cthread_debug) {
		char buf[2];

		buf[0] = name;
		buf[1] = '\0';
		cthread_set_name(cthread_self(), buf);
	}
#endif	DEBUG
	for (i=1;i<3;i++) {
		mutex_lock(lock);
		retcode = port_allocate(task_self(),&port);
		printport(retcode,port);
		if (!cthread_debug) {
			printf("%c", name);
			(void) fflush(stdout);
		}
		retcode = port_deallocate(task_self(),port);
		printport(retcode,port);
		mutex_unlock(lock);
		cthread_yield();
	}
}

main(argc, argv)
	int argc;
	char **argv;
{
	int nprocs = 2;
	int i;

	cthread_init();
	while (--argc && (++argv)[0][0] == '-') {
		switch (argv[0][1]) {
#ifdef	DEBUG
		    case 'd':
			cthread_debug = TRUE;
			break;
#endif	DEBUG
		    case 'n':
			argv += 1;
			argc -= 1;
			nprocs = atoi(argv[0]);
			break;
		    default:
			break;
		}
	}
	lock = mutex_alloc();
	for (i = 0; i < nprocs; i += 1)
		cthread_detach(cthread_fork(proc, 'A' + i));
	cthread_exit(0);
}
