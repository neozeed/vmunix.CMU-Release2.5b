/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	csw.s,v $
 * Revision 1.2  89/05/07  11:47:56  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 * vax/csw.s
 *
 * Context switch and cproc startup for VAX COROUTINE implementation.
 */

	.text

/*
 * Suspend the current thread and resume the next one.
 *
 *	void
 *	cproc_switch(cur, next)
 *		int *cur;
 *		int *next;
 */
	.align	1
	.globl	_cproc_switch
_cproc_switch:
	.word	0xfc0		/* save current registers */
	movl	fp,*4(ap)	/* save current fp */
	movl	*8(ap),fp	/* restore next fp */
	movl	fp,sp		/* sp = fp */
	ret			/* return to next thread */

/*
 *	void
 *	cproc_start(parent_context, child, stackp)
 *		int *parent_context;
 *		cproc_t child;
 *		int stackp;
 */
	.align	1
	.globl	_cproc_start
_cproc_start:
	.word	0xfc0			/* save parent registers */
	movl	fp,*4(ap)		/* save parent fp */
	movl	12(ap),sp		/* sp = stackp */
	movl	sp,fp			/* fp = sp */
	pushl	8(ap)
	calls	$1,_cthread_body	/* cthread_body(child) */
	/*
	 * Control never returns from cthread_body().
	 */
