/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	types.h,v $
 * Revision 2.0  89/06/15  15:43:58  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *  89/03/09  21:00:01  rpd
 * 	More cleanup.
 * 
 *  89/02/25  19:15:54  gm0w
 * 	Changes for cleanup.
 * 
 *  89/01/27  10:21:39  rvb
 * 	#ifndef FALSE || !defined(KERNEL) ==>
 * 	#ifndef FALSE
 * 	[89/01/24            rvb]
 * 
 *  89/01/18  01:05:18  jsb
 * 	Include file references.
 * 	[89/01/17  14:36:36  jsb]
 * 
 * 28-Oct-87  Peter King (king) at NeXT, Inc.
 *	Original Sun source, ported to Mach
 *
 */

/* @(#)types.h	1.1 87/06/19 3.2/4.3NFSSRC */
/* @(#)types.h	1.3 86/10/28 NFSSRC */
/*      @(#)types.h 1.1 86/09/24 SMI      */

/*
 * Rpc additions to <sys/types.h>
 */
#ifndef	__TYPES_RPC_HEADER__
#define __TYPES_RPC_HEADER__

#define bool_t	int
#define enum_t	int
#ifndef	FALSE
#define FALSE	(0)
#endif	FALSE
#ifndef	TRUE
#define TRUE	(1)
#endif	TRUE
#define __dontcare__	-1
#ifndef	NULL
#define NULL 0
#endif

#ifdef	KERNEL
extern char *kalloc();
#define mem_alloc(bsize)	kalloc((u_int)bsize)
#define mem_free(ptr, bsize)	kfree((caddr_t)(ptr), (u_int)(bsize))
#else	KERNEL
extern char *malloc();
#define mem_alloc(bsize)	malloc(bsize)
#define mem_free(ptr, bsize)	free(ptr)
#endif	KERNEL

#include <sys/types.h>

#endif	!__TYPES_RPC_HEADER__
