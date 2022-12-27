/****************************************
 *
 * HISTORY
 *
 * $Log:	nfssvc.c,v $
 * Revision 2.0  89/06/15  17:06:18  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)nfssvc.c	1.3 87/03/03 NFSSRC */
/* @(#)nfssvc.c 1.1 86/09/24 SMI; from UCB 4.1 82/12/04 */

#include "SYS.h"

#if	CMUCS
SYSCALL(nfs_svc)
#else	/* CMUCS */
SYSCALL(nfssvc)
#endif	/* CMUCS */
	ret
