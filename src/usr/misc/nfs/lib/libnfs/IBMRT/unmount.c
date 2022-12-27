/*
 **********************************************************************
 * HISTORY
 * 24-Jan-89  Paul Parker (parker) at Carnegie-Mellon University
 *	Created for IBMRT from vax/sys/unmount.c
 * $Log:	unmount.c,v $
 * Revision 2.0  89/06/15  16:45:44  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 **********************************************************************
 */
#include "SYS.h"

SYSCALL(unmount)
   .align 2
   .ltorg
