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
 * $Log:	exception_all.h,v $
 * Revision 2.2.1.1  89/10/31  23:52:59  mrt
 * 	Added exceptin_all.h
 * 
 * 
 * Revision 2.2  89/10/31  23:43:04  mrt
 * 	Combined mach/exception, mach/ca/exeption.h, 
 * 	mach/sun3/exception, mach/vax/exception.h
 * 	[89/10/29            mrt]
 * 
 *
 */

#ifndef	_EXCEPTION_ALL_H_
#define _EXCEPTION_ALL_H_

#ifndef _MACH_EXCEPTION_H_
#define _MACH_EXCEPTION_H_

/*
 *	Machine-independent exception definitions.
 */

#define EXC_BAD_ACCESS		1	/* Could not access memory */
		/* Code contains kern_return_t describing error. */
		/* Subcode contains bad memory address. */

#define EXC_BAD_INSTRUCTION	2	/* Instruction failed */
		/* Illegal or undefined instruction or operand */

#define EXC_ARITHMETIC		3	/* Arithmetic exception */
		/* Exact nature of exception is in code field */

#define EXC_EMULATION		4	/* Emulation instruction */
		/* Emulation support instruction encountered */
		/* Details in code and subcode fields	*/

#define EXC_SOFTWARE		5	/* Software generated exception */
		/* Exact exception is in code field. */
		/* Codes 0 - 0xFFFF reserved to hardware */
		/* Codes 0x10000 - 0x1FFFF reserved for OS emulation (Unix) */

#define EXC_BREAKPOINT		6	/* Trace, breakpoint, etc. */
		/* Details in code field. */

#endif _MACH_EXCEPTION_H_

/* 	Machine dependent exceptions for the ibm_rt
 * 	Revision 2.5  89/06/30  22:33:38  rpd
 */

#ifndef	_MACH_CA_EXCEPTION_H_
#define _MACH_CA_EXCEPTION_H_

#include <mach/kern_return.h>

/*
 *	EXC_BAD_ACCESS
 */
/*
 *	Romp has machine-dependent failure modes.  The codes
 *	are negative so as not to conflict with kern_return_t's.
 */
#define EXC_ROMP_MCHECK		((kern_return_t)-1) /* machine check */
#define EXC_ROMP_APC_BUG	((kern_return_t)-2) /* APC hardware bug */

/*
 * EXC_BAD_INSTRUCTION
 */
#define EXC_ROMP_PRIV_INST 	0x01
#define EXC_ROMP_ILLEGAL_INST	0x02

/*
 * EXC_BREAKPOINT
 */
#define EXC_ROMP_TRAP_INST 	0x01
#define EXC_ROMP_INST_STEP  	0x02

/*
 * EXC_ARITHMETIC
 *
 */

/*							   
 * Values for code when type == EXC_ARITHMETIC
 */							    

#define EXC_ROMP_FPA_EMUL	0x01
#define EXC_ROMP_68881		0x02
#define EXC_ROMP_68881_TIMEOUT	0x04
#define EXC_ROMP_FLOAT_SPEC	0x08

#endif	_MACH_CA_EXCEPTION_H_


/*
 *	Machine dependent exception definitions for the Sun3.
 * 	Revision 2.4  89/03/09  20:23:26  rpd
 */

#ifndef	_MACH_SUN3_EXCEPTION_H_
#define _MACH_SUN3_EXCEPTION_H_

/*
 *	EXC_BAD_INSTRUCTION
 */

#define EXC_SUN3_ILLEGAL_INSTRUCTION	0x10
#define EXC_SUN3_PRIVILEGE_VIOLATION	0x20
#define EXC_SUN3_COPROCESSOR		0x34
#define EXC_SUN3_TRAP1			0x84
#define EXC_SUN3_TRAP2			0x88
#define EXC_SUN3_TRAP3			0x8c
#define EXC_SUN3_TRAP4			0x90
#define EXC_SUN3_TRAP5			0x94
#define EXC_SUN3_TRAP6			0x98
#define EXC_SUN3_TRAP7			0x9c
#define EXC_SUN3_TRAP8			0xa0
#define EXC_SUN3_TRAP9			0xa4
#define EXC_SUN3_TRAP10			0xa8
#define EXC_SUN3_TRAP11			0xac
#define EXC_SUN3_TRAP12			0xb0
#define EXC_SUN3_TRAP13			0xb4
#define EXC_SUN3_TRAP14			0xb8
#define EXC_SUN3_FLT_BSUN		0xc0
#define EXC_SUN3_FLT_OPERAND_ERROR	0xd0

/*
 *	NOTE: TRAP0 is syscall, TRAP15 is breakpoint.
 */

/*
 *	EXC_ARITHMETIC
 */

#define EXC_SUN3_ZERO_DIVIDE		0x14
#define EXC_SUN3_FLT_INEXACT		0xc4
#define EXC_SUN3_FLT_ZERO_DIVIDE	0xc8
#define EXC_SUN3_FLT_UNDERFLOW		0xcc
#define EXC_SUN3_FLT_OVERFLOW		0xd4
#define EXC_SUN3_FLT_NOT_A_NUMBER	0xd8

/*
 *	EXC_EMULATION
 */

#define EXC_SUN3_LINE_1010		0x28
#define EXC_SUN3_LINE_1111		0x2c

/*
 *	EXC_SOFTWARE
 */

#define EXC_SUN3_CHK	0x18
#define EXC_SUN3_TRAPV	0x1c

/*
 *	EXC_BREAKPOINT
 */

#define EXC_SUN3_TRACE			0x24
#define EXC_SUN3_BREAKPOINT		0xbc

#endif	_MACH_SUN3_EXCEPTION_H_


/*
 *	Machine dependent exceptions for the vax
 * 	Revision 2.4  89/03/09  20:24:40  rpd
 */

/*
 *	EXC_BAD_INSTRUCTION
 */

#ifndef	_MACH_VAX_EXCEPTION_H_
#define _MACH_VAX_EXCEPTION_H_

#define EXC_VAX_PRIVINST		1
#define EXC_VAX_RESOPND			2
#define EXC_VAX_RESADDR			3
#define EXC_VAX_COMPAT			4

/*
 *	COMPAT subcodes
 */
#define EXC_VAX_COMPAT_RESINST		0
#define EXC_VAX_COMPAT_BPT		1
#define EXC_VAX_COMPAT_IOT		2
#define EXC_VAX_COMPAT_EMT		3
#define EXC_VAX_COMPAT_TRAP		4
#define EXC_VAX_COMPAT_RESOP		5
#define EXC_VAX_COMPAT_ODDADDR		6

/*
 *	EXC_ARITHMETIC
 */

#define EXC_VAX_INT_OVF			1
#define EXC_VAX_INT_DIV			2
#define EXC_VAX_FLT_OVF_T		3
#define EXC_VAX_FLT_DIV_T		4
#define EXC_VAX_FLT_UND_T		5
#define EXC_VAX_DEC_OVF			6

#define EXC_VAX_FLT_OVF_F		8
#define EXC_VAX_FLT_DIV_F		9
#define EXC_VAX_FLT_UND_F		10

/*
 *	EXC_SOFTWARE
 */

#define EXC_VAX_SUB_RNG			7

/*
 *	EXC_BREAKPOINT
 */

#define EXC_VAX_BPT			1
#define EXC_VAX_TRACE			2

#endif	_MACH_VAX_EXCEPTION_H_


#endif	_MACH_EXCEPTION_H_
