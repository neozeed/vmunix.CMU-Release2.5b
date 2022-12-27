/*
 **********************************************************************
 * HISTORY
 * $Log:	log.c,v $
 * Revision 1.5  89/08/15  17:04:42  mja
 * 	Fix to use vfprintf() rather than _doprnt().
 * 	[89/04/18            mja]
 * 
 * 14-Sep-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added resource pause disable code.
 *
 * 05-Nov-86  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Remove some non-obvious macros. Speed up some code by
 *	using better suited routines for the purpose. Put
 *	common time code into prfx() routine.
 *
 **********************************************************************
 */
#include <varargs.h>

#include "nanny.h"

/**** lock:
 * This function will do an advisory lock on the file associated 
 * with the given descriptor. Return 0 if the file is locked,
 * -1 otherwise.
 */
lock(file)
int file;
{
    int     i;

    for (i = 0; i < 60; i++) {
	if (flock(file, LOCK_EX|LOCK_NB) < 0) {
	    if (errno != EWOULDBLOCK) {
		fprintf(stderr,"nanny: can not lock: %s\n", errmsg(-1));
		return(-1);
	    }
	}
	else
	    return(0);	/* lock on */
	sleep(1);
    }
    fprintf(stderr, "nanny: file busy too long.\n");
    return(-1);
}

static FILE *logp = NULL;		/* pointer to the open log file */

/*
 * create a logging prefix that is concise and still understandable
 * and includes time and nanny number.
 */
char *prfx()
{
    long    tim;
    struct tm *tm;
    static char mybuf[32];

    tim = time((long *)NULL);
    tm = localtime(&tim);
    (void)sprintf(mybuf, "%2.2d %2.2d:%2.2d:%2.2d #%d: "
	    ,tm->tm_mday
	    ,tm->tm_hour
	    ,tm->tm_min
	    ,tm->tm_sec
	    ,nanny_num);
    return(mybuf);
}

/**** print_log:
 *  The argument format to this function is similar to printf. The logging
 * file is set exclusive and the data printed into the log file.
 */
/*VARARGS1*/
print_log(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list ap;

    va_start(ap);
    if (logp) {
	if (lock(fileno(logp)) == -1)
	    return;

	(void)fseek(logp, 0L, 2);
	fputs(prfx(), logp);
	vfprintf(logp, fmt, ap);

	(void)fflush(logp);
	(void)flock(fileno(logp), LOCK_UN);
    }
    else {
	vfprintf(stdout, fmt, ap);
	(void)fflush(stdout);
    }
    va_end(ap);
    return;
}			       /* print_log */

/**** print_err:
 * /
/*VARARGS1*/
print_err(fmt, va_alist)
char   *fmt;
va_dcl
{
    va_list ap;

    va_start(ap);
    if (logp) {
	if (lock(fileno(logp)) < 0)
	    return;

	(void)fseek(logp, 0L, 2);
	fputs(prfx(),logp);
	vfprintf(logp, fmt, ap);

	(void)fflush(logp);
	(void)flock(fileno(logp), LOCK_UN);
    }
    else
	vfprintf(stderr, fmt, ap);
    va_end(ap);
}

/**** open_log:
 *   A directory is created and the global logging path variable is set to 
 * that directory's path.
 */
open_log()
{
    char    tmp[DEV_BSIZE];
    FILE   *fp;
    int     tmask;
    int	    fd;
    int     iocarg;

    (void)sprintf(tmp, "%s/nanny.log", logdir);
    tmask = umask(LOGMASK);
    fd = open(tmp, O_WRONLY|O_APPEND|O_CREAT, 0644);
    (void)umask(tmask);
    if (fd < 0) {
	print_log("Unable to open %s: %s\n", tmp, errmsg(errno));
	return;
    }
    if ((fp = fdopen(fd, "a")) == NULL) {
	print_log("Unable to fdopen %s fd=%d\n",tmp,fd);
	(void)close(fd);
	return;
    }

    if (logp)
	(void)fclose(logp);
    logp = fp;

    /* disable resource pausing */
    iocarg = FIOCNOSPC_ERROR;
    if (ioctl(fileno(logp), FIOCNOSPC, &iocarg) < 0)
	print_log("Unable to disable resource pausing: %s\n", errmsg(errno));

    /* redirect stderr and stdout to /dev/null */
    if ((fd = open("/dev/null", O_RDWR, 0)) >= 0) {
	(void)dup2(fd, 2);
	(void)dup2(fd, 1);
	(void)close(fd);
    }
}				/* open_log */
