/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	macherr.c,v $
 * Revision 1.3  89/05/05  18:22:26  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  11-May-88  Douglas Orr (dorr) at Carnegie Mellon
 *	Added ability to interpret hex error codes
 *
 *  25-Mar-87  Mary Thompson @ Carnegie Mellon
 *	Started
 *******************************************************/
/*
 * macherr.c
 *
 *  Usage: macherr error_number
 *	prints out a message for the given mach error code
 */

#include <stdio.h>
#include <mach_error.h>

#define true	(1)
#define false	(0)

main(argc,argv)
  int 	argc;
  char  * * argv;
{
	mach_error_t	err;

	if (argc == 1 ) {
		printf("Usage is %s [x]errnum ...\n", *argv);
		exit (1);
	}
	while( --argc ) {
		++argv;
		if( (*argv)[0] == 'x' ) {
		    if( !atox( *argv, &err ) )
			continue;
		}
		else
			err = atoi( *argv );
		printf("%s\n",mach_error_string(err));
	}
}


atox( a, val )
	char	* a;
	int	* val;
{
	char	* oa = a;
	int	x;

	x = 0;
	a++;
	while( *a ) {
		if ( (*a >= '0') && (*a <= '9') )
			x = (x << 4) | (*a - '0');
		else
		if ( (*a >= 'a') && (*a <= 'f') )
			x = (x << 4) | (*a - 'a' + 10);
		else
		if ( (*a >= 'A') && (*a <= 'F') )
			x = (x << 4) | (*a - 'A' + 10);
		else {
			fprintf( stderr, "invalid hex number %s\n", oa );
			return( false );
		}
		++a;
	}

	*val = x;
	return( true );
}


