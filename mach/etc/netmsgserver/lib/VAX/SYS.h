/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * HISTORY
 * $Log:	SYS.h,v $
 * Revision 1.4  89/05/02  11:05:42  dpj
 * 	Fixed all files to conform to standard copyright/log format
 * 
 */

#include "syscall.h"

#ifdef PROF
#define	ENTRY(x)	.globl _/**/x; .align 2; _/**/x: .word 0; \
			.data; 1:; .long 0; .text; moval 1b,r0; jsb mcount
#else
#define	ENTRY(x)	.globl _/**/x; .align 2; _/**/x: .word 0
#endif PROF
#define	SYSCALL(x)	err_/**/x: jmp cerror; ENTRY(x); chmk $SYS_/**/x; jcs err_/**/x
#define	PSEUDO(x,y)	ENTRY(x); chmk $SYS_/**/y
#define	CALL(x,y)	calls $x, _/**/y

	.globl	cerror
