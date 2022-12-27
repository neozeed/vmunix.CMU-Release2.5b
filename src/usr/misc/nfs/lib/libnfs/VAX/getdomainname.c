/****************************************
 *
 * HISTORY
 *
 * $Log:	getdomainname.c,v $
 * Revision 2.0  89/06/15  17:06:06  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)getdomainname.c	1.3 87/03/03 NFSSRC */
/* @(#)getdomainname.c 1.1 86/09/24 SMI */

#include "SYS.h"

SYSCALL(getdomainname)
	ret		/* len = getdomainname(buf, buflen) */
