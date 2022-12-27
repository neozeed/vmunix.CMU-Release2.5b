/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	fork.sun.cs,v $
 * Revision 1.2  89/05/05  18:44:23  mrt
 * 	HISTORY
 * 
 * 25-Nov-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Added call to mach_init().
 *
 */ 
/* @(#)fork.c 1.1 86/02/03 SMI; from UCB 4.1 82/12/04 */

#include "SYS.sun.h"

	.globl	_mach_init

SYSCALL(fork)
#if sun
	tstl	d1
	beqs	2$	/* parent, since ...  */
	link	a6,#0
	jsr	_mach_init
	unlk	a6
	clrl	d0
2$:
#endif
	RET		/* pid = fork() */
