/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	jtest.c,v $
 * Revision 1.2  89/05/05  19:03:42  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  18-May-88 Mary Thompson
 *	fixed to always return a status value
 */
/*
 * This program has basically 3 options, to fork and then join, to fork and
 * then detach, or to simply call the procedure null() and never fork.  Timing
 * is done 
 * What is cthread_set_limit? the l switch.
 *
 */

#include <stdio.h>
#include <cthreads.h>
#include <sys/types.h>
#include <sys/time.h>

int repeat_count = 0;
struct timeval before, after;

null()
{
	printf("null()\n");
}

qnull()
{
}

main(argc, argv)
	int argc;
	char **argv;
{
	register int (*proc)();
	register int i = 0;
	double total;
	int call_only = FALSE;
	int fork_only = FALSE;
	char *op;

	cthread_init();
	while (--argc && (++argv)[0][0] == '-') {
		switch (argv[0][1]) {
#ifdef	DEBUG
		    case 'd':
			cthread_debug = TRUE;
			break;
#endif	DEBUG
		    case 'c':
			call_only = TRUE;
			break;
		    case 'f':
			fork_only = TRUE;
			break;
		    case 'l':
			i = atoi(*++argv);
			argc -= 1;
			break;
		    case 'n':
			repeat_count = atoi(*++argv);
			argc -= 1;
			break;
		    case 'h':
			printf("Switches possible are: \n");
			printf("	c: do not do a fork\n");
			printf("	f: fork, then detach otherwise ");
			printf("join after the fork\n");
			printf("	n <number>: number of times to");
			printf(" fork if c switch not used\n");
			printf("	l <number>: sets max_cprocs??\n");
			exit(0);
		    default:
			break;
		}
	}
	if (i > 0)
		cthread_set_limit(i);
	if (repeat_count <= 0) {
		if (call_only)
			null();
		else {
			cthread_t t = cthread_fork(null);

			printf("Forked.\n");
			if (fork_only) {
				cthread_detach(t);
				printf("Detached.\n");
			} else {
				cthread_join(t);
				printf("Joined.\n");
			}
		}
		exit(0);
	}
#ifdef	DEBUG
	proc = cthread_debug ? null : qnull;
#else
	proc = qnull;
#endif	DEBUG
	(void) gettimeofday(&before, (struct timezone *) 0);
	if (call_only) {
		for (i = 1; i <= repeat_count; i += 1)
			(*proc)();
	} else if (fork_only) {
		for (i = 1; i <= repeat_count; i += 1)
			cthread_detach(cthread_fork(proc));
	} else {
		for (i = 1; i <= repeat_count; i += 1)
			cthread_join(cthread_fork(proc));
	}
	(void) gettimeofday(&after, (struct timezone *) 0);
	total = (after.tv_sec - before.tv_sec)*1000000.0
		+ (after.tv_usec - before.tv_usec);
	op = call_only ? "call" : (fork_only ? "fork" : "fork-join");
	printf("%d %s operations\n", repeat_count, op);
	printf("%.2f milliseconds elapsed time\n", total/1000.0);
	printf("%.2f microseconds per %s\n", total/(double)repeat_count, op);
	cthread_exit(0);
}
