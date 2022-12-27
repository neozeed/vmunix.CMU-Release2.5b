/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	Ovfork.c,v $
 * Revision 1.2  89/05/05  18:38:10  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *    Jan-15-87	Mary R. Thompson
 *	Started
 */
/*
 *  Abstract:
 *	Just calls fork, as the vfork kernel call no longer
 *	does anything different from fork.  The fork program calls
 *	mach_init for the child.
 */

  int vfork()
  {  return( fork());
  }
