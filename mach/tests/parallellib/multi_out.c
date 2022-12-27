/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	multi_out.c,v $
 * Revision 2.1  89/08/29  23:26:16  mrt
 * Created.
 * 
 */

/*
 * multi_out.c
 *
 * Code to test stdio output locks.
 *
 * Michael B. Jones
 *
 * 12-Aug-87
 */

#include <stdio.h>
#include <cthreads.h>

void thread_body(thread) int thread;
{
    int i;

    printf("thread_body(%d)\n", thread);

    for (i = 1; i <= 1000; i++) {
	printf("Thread %d out\n", thread);
	cthread_yield();
/*	for (j = 0; j < 10000; j++) {} */
    }
}

#define THREADS 6

main()
{
    cthread_t children[THREADS];
    register int i;

    for (i = 1; i < THREADS; i++)
	children[i] = cthread_fork(thread_body, i);
    thread_body(0);

    for (i = 1; i < THREADS; i++)
	cthread_join(children[i]);

    printf("That's all folks!\n");
}
