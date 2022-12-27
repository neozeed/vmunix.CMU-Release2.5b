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
 * $Log:	vmtpsys.c,v $
 * Revision 1.4  89/05/02  11:05:46  dpj
 * 	Fixed all files to conform to standard copyright/log format
 * 
 * 28-May-87  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Created.
 *
 */

/*
 * Library for VMTP system calls.
 */

#include "SYS.h"
#include "syscall.h"

SYSCALL(invoke);
	ret

SYSCALL(recvreq);
	ret

SYSCALL(sendreply);
	ret

SYSCALL(forward);
	ret

SYSCALL(probeentity);
	ret

SYSCALL(getreply);
	ret
