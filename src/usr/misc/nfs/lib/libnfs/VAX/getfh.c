/****************************************
 *
 * HISTORY
 *
 * $Log:	getfh.c,v $
 * Revision 2.0  89/06/15  17:06:13  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)getfh.c	1.3 87/03/03 NFSSRC */
/* @(#)getfh.c 1.1 86/09/24 SMI */

#include "SYS.h"

#if	CMUCS
SYSCALL(nfs_getfh)
#else	/* CMUCS */
SYSCALL(getfh)
#endif	/* CMUCS */
	ret
