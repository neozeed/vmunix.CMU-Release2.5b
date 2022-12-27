/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	thread.c,v $
 * Revision 1.2  89/05/05  19:02:59  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 * vax/thread.c
 *
 * Cproc startup for VAX MTHREAD implementation.
 */

#ifndef	lint
static char rcs_id[] = "$Header: thread.c,v 1.2 89/05/05 19:02:59 mrt Exp $";
#endif	not lint



#include <cthreads.h>
#include "cthread_internals.h"

#if	MTHREAD

#include <mach.h>

/*
 * C library imports:
 */
extern bzero();

/*
 * Set up the initial state of a MACH thread
 * so that it will invoke cthread_body(child)
 * when it is resumed.
 */
void
cproc_setup(child)
	register cproc_t child;
{
	register int *top = (int *) (child->stack_base + child->stack_size);
	struct vax_thread_state state;
	register struct vax_thread_state *ts = &state;
	kern_return_t r;
	extern void cthread_body();

	/*
	 * Set up VAX call frame and registers.
	 * See VAX Architecture Handbook.
	 */
	bzero((char *) ts, sizeof(struct vax_thread_state));
	/*
	 * Set pc to location after register save mask at procedure entry.
	 * Inner cast needed since too many C compilers choke on the type void (*)().
	 */
	ts->pc = (int) (int (*)()) cthread_body + 2;
	*--top = (int) child;	/* argument to function */
	*--top = 1;		/* number of arguments */
	ts->ap = (int) top;
	*--top = 0; *--top = 0; *--top = 0;
	*--top = (1 << 29);
	*--top = 0;
	ts->fp = ts->sp = (int) top;

	MACH_CALL(thread_set_state(child->id, \
				   VAX_THREAD_STATE, \
				   (thread_state_t) &state, \
				   VAX_THREAD_STATE_COUNT),
		  r);
}
#endif	MTHREAD
