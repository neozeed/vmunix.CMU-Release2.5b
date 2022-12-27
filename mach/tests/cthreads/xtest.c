/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	xtest.c,v $
 * Revision 1.2  89/05/05  19:04:06  mrt
 * 	Cleanup for Mach 2.5
 * 
 */

#include <stdio.h>
#include <cthreads.h>

mutex_t lock;

proc(name)
	char name;
{
	int i;

#ifdef	DEBUG
	if (cthread_debug) {
		char buf[2];

		buf[0] = name;
		buf[1] = '\0';
		cthread_set_name(cthread_self(), buf);
	}
#endif	DEBUG
	for (i = 0; i < 1000; i++) {
		mutex_lock(lock);
		if (!cthread_debug) {
			printf("%c", name);
			(void) fflush(stdout);
		}
		mutex_unlock(lock);
		cthread_yield();
	}
	cthread_exit(0);
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
