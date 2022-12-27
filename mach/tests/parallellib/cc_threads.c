/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	cc_threads.c,v $
 * Revision 2.1  89/08/29  23:25:24  mrt
 * Created.
 * 
 */

#include <stdio.h>
#include <cthreads.h>

main()
{
    printf("Hello once.\n"); fflush(stdout);
    cthread_init();
    printf("Hello twice.\n"); fflush(stdout);
}
