#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)printf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	printf.c,v $
 * Revision 1.3  89/08/03  14:17:41  mja
 * 	Updated to use <varargs.h>.
 * 	[89/04/18            mja]
 * 
 *
 * 07-Dec-86 Doug Philips (dwp) at Carnegie-Mellon University
 *	Added dummy arguments to make it work for the RT.
 *
 **********************************************************************
 */
#include	<stdio.h>
#include	<varargs.h>

#if	!defined(vax) && !defined(sun3) && !defined(ibmrt)
/* 
 *  The only portable way to use this function is via <varargs.h> and
 *  vprintf().  No new architectures make _doprnt() visible.
 */
#define	_doprnt	_doprnt_va
#endif

printf(fmt, va_alist)
char *fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	_doprnt(fmt, ap, stdout);
	va_end(ap);
	return(ferror(stdout)? EOF: 0);
}
