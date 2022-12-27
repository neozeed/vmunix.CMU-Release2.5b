/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	lock_file.c,v $
 * Revision 2.1  89/08/29  23:26:02  mrt
 * Created.
 * 
 */

/*
 * lock_file.c
 *
 * Test f_lockbuf and f_unlockbuf.
 * 
 * Michael B. Jones  --  30-Jul-87
 */

#include <stdio.h>

FILE *files[1000];

main(argc, argv) int argc; char *argv[];
{
    register int i;
    f_buflock lock;

    if (argc > 1) {
	printf("Preallocating iobs\n");
	f_prealloc();
    }

    for (i = 0; i < 1000; i++) {
	files[i] = fopen("/dev/null", "r");
	if (files[i] == NULL) {
	    printf("file %d was null\n", i);
	    break;
	}
	lock = f_lockbuf(files[i]);
	printf("%d %d %x %x\n", i, fileno(files[i]), files[i], lock);
	fflush(stdout);
	f_unlockbuf(lock);
    }
}
