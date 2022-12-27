/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getnetbyname.c	5.3 (Berkeley) 5/19/86";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * 10-Jan-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed to case-insensitive string comparison.
 *
 **********************************************************************
 */
#include <netdb.h>

extern int _net_stayopen;

struct netent *
getnetbyname(name)
	register char *name;
{
	register struct netent *p;
	register char **cp;

	setnetent(_net_stayopen);
	while (p = getnetent()) {
#if	CMUCS
		if (strcasecmp(p->n_name, name) == 0)
#else	CMUCS
		if (strcmp(p->n_name, name) == 0)
#endif	CMUCS
			break;
		for (cp = p->n_aliases; *cp != 0; cp++)
#if	CMUCS
			if (strcasecmp(*cp, name) == 0)
#else	CMUCS
			if (strcmp(*cp, name) == 0)
#endif	CMUCS
				goto found;
	}
found:
	if (!_net_stayopen)
		endnetent();
	return (p);
}
