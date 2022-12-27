#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)scanf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	scanf.c,v $
 * Revision 1.3  89/08/03  14:26:24  mja
 * 	Update to use <varargs.h>
 * 	[89/03/30            mja]
 * 
 * 07-Dec-86  Doug Philips (dwp) at Carnegie-Mellon University
 *	Added dummy arguments to get them to work on RT's.
 *
 **********************************************************************
 */
#include	<stdio.h>
#include	<varargs.h>

scanf(fmt, va_alist)
char *fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	return(_doscan(stdin, fmt, ap));
	va_end(ap);
}

fscanf(iop, fmt, va_alist)
FILE *iop;
char *fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	return(_doscan(iop, fmt, ap));
	va_end(ap);
}

sscanf(str, fmt, va_alist)
register char *str;
char *fmt;
va_dcl
{
	FILE _strbuf;
	va_list ap;

	_strbuf._flag = _IOREAD|_IOSTRG;
	_strbuf._ptr = _strbuf._base = str;
	_strbuf._cnt = 0;
	while (*str++)
		_strbuf._cnt++;
	_strbuf._bufsiz = _strbuf._cnt;

	va_start(ap);
	return(_doscan(&_strbuf, fmt, ap));
	va_end(ap);
}
