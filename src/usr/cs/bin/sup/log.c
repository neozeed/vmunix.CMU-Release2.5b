/*
 * Logging support for SUP
 **********************************************************************
 * HISTORY
 * $Log:	log.c,v $
 * Revision 1.4  89/08/03  19:48:59  mja
 * 	Updated to use v*printf() in place of _doprnt().
 * 	[89/04/19            mja]
 * 
 * 27-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added check to allow logopen() to be called multiple times.
 *
 * 20-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */

#include <stdio.h>
#include <sys/syslog.h>
#include <c.h>
#include <varargs.h>
#include "sup.h"

static int opened = 0;

logopen(program)
char *program;
{
	if (opened)  return;
	openlog(program,LOG_PID,LOG_DAEMON);
	opened++;
}

/*VARARGS1*/
logquit(retval,fmt,va_alist)
int retval;
char *fmt;
va_dcl
{
	char buf[STRINGLENGTH];
	va_list ap;

	va_start(ap);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (opened) {
		syslog (LOG_ERR,buf);
		closelog ();
		exit (retval);
	}
	quit (retval,"SUP: %s\n",buf);
}

/*VARARGS1*/
logerr(fmt,va_alist)
char *fmt;
va_dcl
{
	char buf[STRINGLENGTH];
	va_list ap;

	va_start(ap);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (opened) {
		syslog (LOG_ERR,buf);
		return;
	}
	fprintf (stderr,"SUP: %s\n",buf);
	(void) fflush (stderr);
}

/*VARARGS1*/
loginfo(fmt,va_alist)
char *fmt;
va_dcl
{
	char buf[STRINGLENGTH];
	va_list ap;

	va_start(ap);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (opened) {
		syslog (LOG_INFO,buf);
		return;
	}
	printf ("%s\n",buf);
	(void) fflush (stdout);
}
