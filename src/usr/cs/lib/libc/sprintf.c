#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sprintf.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	sprintf.c,v $
 * Revision 1.3  89/08/03  14:23:56  mja
 * 	Add snprintf().
 * 	[89/07/12            mja]
 * 
 * 	Update to use <varargs.h>.
 * 	[89/04/18            mja]
 * 
 * 07-Dec-86  Doug Philips (dwp) at Carnegie-Mellon University
 *	Added dummy arguments to get them to work on RT's.
 *
 **********************************************************************
 */
#include	<stdio.h>
#include	<varargs.h>

#if	!defined(vax) && !defined(sun3) && !defined(ibmrt)
/* 
 *  The only portable way to use this function is via <varargs.h> and
 *  vs*printf().  No new architectures make _doprnt() visible.
 */
#define	_doprnt	_doprnt_va
#endif

char *sprintf(str, fmt, va_alist)
char *str, *fmt;
va_dcl
{
	FILE _strbuf;
	va_list ap;

	_strbuf._flag = _IOWRT+_IOSTRG;
	_strbuf._ptr = str;
	_strbuf._cnt = 32767;
	va_start(ap);
	_doprnt(fmt, ap, &_strbuf);
	va_end(ap);
	putc('\0', &_strbuf);
	return(str);
}

int
snprintf(str, n, fmt, va_alist)
char *str;
int n;
char *fmt;
va_dcl
{
	FILE _strbuf;
	va_list ap;

	_strbuf._flag = _IOSTRG;	/* no _IOWRT: avoid stdio bug */
	_strbuf._ptr = str;
	_strbuf._cnt = n-1;
	va_start(ap);
	_doprnt(fmt, ap, &_strbuf);
	va_end(ap);
	_strbuf._cnt++;
	putc('\0', &_strbuf);
	if (_strbuf._cnt<0)
	    _strbuf._cnt = 0;
	return (n-_strbuf._cnt-1);
}
