/*
 *****************************************************************************
 * HISTORY
 * $Log:	printf.c,v $
 * Revision 2.2  89/08/15  15:12:43  mja
 * 	Updated for <varargs.h>
 * 	[89/08/15  12:57:49  mja]
 * 
 *****************************************************************************
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley Software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char *sccsid = "@(#)printf.c	5.2 (Berkeley) 6/6/85";
#endif

#include <varargs.h>

/*
 * Hacked "printf" which prints through putchar.
 * DONT USE WITH STDIO!
 */
printf(fmt, va_alist)
char *fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	_doprnt(fmt, ap, 0);
	va_end(ap);
}
