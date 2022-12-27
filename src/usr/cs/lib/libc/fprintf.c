/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)fprintf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	fprintf.c,v $
 * Revision 1.3  89/08/03  14:13:37  mja
 * 	Updated to use <varargs.h>.
 * 	[89/04/18            mja]
 * 
 * 07-Dec-86  Doug Philips (dwp) at Carnegie-Mellon University
 *	Added dummy argument to get it to work on RT's.
 *
 **********************************************************************
 */
#include	<stdio.h>
#include	<varargs.h>


#if	!defined(vax) && !defined(sun3) && !defined(ibmrt)
/* 
 *  The only portable way to use this function is via <varargs.h> and
 *  vfprintf().  No new architectures make _doprnt() visible.
 */
#define	_doprnt	_doprnt_va
#endif

fprintf(iop, fmt, va_alist)
register FILE *iop;
char *fmt;
va_dcl
{
	char localbuf[BUFSIZ];
	va_list ap;

	va_start(ap);
	if (iop->_flag & _IONBF) {
		iop->_flag &= ~_IONBF;
		iop->_ptr = iop->_base = localbuf;
		iop->_bufsiz = BUFSIZ;
		_doprnt(fmt, ap, iop);
		fflush(iop);
		iop->_flag |= _IONBF;
		iop->_base = NULL;
		iop->_bufsiz = NULL;
		iop->_cnt = 0;
	} else
		_doprnt(fmt, ap, iop);
	va_end(ap);
	return(ferror(iop)? EOF: 0);
}
