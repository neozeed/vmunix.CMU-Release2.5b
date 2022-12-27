/*
 **********************************************************************
 * HISTORY
 * 26-May-89  Paul Parker (parker) at Carnegie-Mellon University
 *	Created for IBMRT.
 * $Log:	getfh.c,v $
 * Revision 2.0  89/06/15  16:46:01  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 **********************************************************************
 */
#include "SYS.h"

SYSCALL(nfs_getfh)
   .align 2
   .ltorg
