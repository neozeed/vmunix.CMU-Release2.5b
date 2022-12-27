/*
 **********************************************************************
 * HISTORY
 * 24-Jan-89  Paul Parker (parker) at Carnegie-Mellon University
 *	Created for IBMRT from vax/sys/getdomainname.c
 * $Log:	getdomainname.c,v $
 * Revision 2.0  89/06/15  16:45:55  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 **********************************************************************
 */
#include "SYS.h"

SYSCALL(getdomainname)
   .align 2
   .ltorg
