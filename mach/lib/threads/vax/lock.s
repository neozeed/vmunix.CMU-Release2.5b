/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	lock.s,v $
 * Revision 1.3  89/05/07  11:47:50  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 * vax/lock.s
 *
 * Mutex implementation for VAX.
 */

	.text
/*
 *	int
 *	mutex_try_lock(m)
 *		mutex_t m;	(= int *m for our purposes)
 */
	.align	1
	.globl	_mutex_try_lock
_mutex_try_lock:
	.word	0
	bbssi	$0,*4(ap),1f
	movl	$1,r0		/* yes */
	ret
1:
	clrl	r0		/* no */
	ret

/*
 *	void
 *	mutex_unlock(m)
 *		mutex_t m;	(= int *m for our purposes)
 */
	.align	1
	.globl	_mutex_unlock
_mutex_unlock:
	.word	0
	bbcci	$0,*4(ap),1f
1:	ret
