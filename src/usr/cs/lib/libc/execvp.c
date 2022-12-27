/*
 *****************************************************************************
 * HISTORY
 * $Log:	execvp.c,v $
 * Revision 1.2  89/08/03  13:59:48  mja
 * 	Updated to use <varargs.h>.
 * 	[89/07/20            mja]
 * 
 *****************************************************************************
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)execvp.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 *	execlp(name, arg,...,0)	(like execl, but does path search)
 *	execvp(name, argv)	(like execv, but does path search)
 */
#include <errno.h>
#ifndef	NOVARARGS
#include <varargs.h>
#endif	/* NOVARARGS */
#define	NULL	0

static	char shell[] =	"/bin/sh";
char	*execat(), *getenv();
extern	errno;

#ifndef	NOVARARGS
execlp(name, va_alist)
char *name;
va_dcl
#else	/* NOVARARGS */
execlp(name, argv)
char *name, *argv;
#endif	/* NOVARARGS */
{
#ifndef	NOVARARGS
	int val;
	va_list ap;

	va_start(ap);
	val = execvp(name, ap);
	va_end(ap);
	return(val);
#else	/* NOVARARGS */
	return(execvp(name, &argv));
#endif	/* NOVARARGS */
}

execvp(name, argv)
char *name, **argv;
{
	char *pathstr;
	register char *cp;
	char fname[128];
	char *newargs[256];
	int i;
	register unsigned etxtbsy = 1;
	register eacces = 0;

	if ((pathstr = getenv("PATH")) == NULL)
		pathstr = ":/bin:/usr/bin";
	cp = index(name, '/')? "": pathstr;

	do {
		cp = execat(cp, name, fname);
	retry:
		execv(fname, argv);
		switch(errno) {
		case ENOEXEC:
			newargs[0] = "sh";
			newargs[1] = fname;
			for (i=1; newargs[i+1]=argv[i]; i++) {
				if (i>=254) {
					errno = E2BIG;
					return(-1);
				}
			}
			execv(shell, newargs);
			return(-1);
		case ETXTBSY:
			if (++etxtbsy > 5)
				return(-1);
			sleep(etxtbsy);
			goto retry;
		case EACCES:
			eacces++;
			break;
		case ENOMEM:
		case E2BIG:
			return(-1);
		}
	} while (cp);
	if (eacces)
		errno = EACCES;
	return(-1);
}

static char *
execat(s1, s2, si)
register char *s1, *s2;
char *si;
{
	register char *s;

	s = si;
	while (*s1 && *s1 != ':')
		*s++ = *s1++;
	if (si != s)
		*s++ = '/';
	while (*s2)
		*s++ = *s2++;
	*s = '\0';
	return(*s1? ++s1: 0);
}
