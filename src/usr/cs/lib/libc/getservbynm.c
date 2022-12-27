/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getservbyname.c	5.3 (Berkeley) 5/19/86";
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

extern int _serv_stayopen;

struct servent *
getservbyname(name, proto)
	char *name, *proto;
{
	register struct servent *p;
	register char **cp;

	setservent(_serv_stayopen);
	while (p = getservent()) {
#if	CMUCS
		if (strcasecmp(name, p->s_name) == 0)
#else	CMUCS
		if (strcmp(name, p->s_name) == 0)
#endif	CMUCS
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
#if	CMUCS
			if (strcasecmp(name, *cp) == 0)
#else	CMUCS
			if (strcmp(name, *cp) == 0)
#endif	CMUCS
				goto gotname;
		continue;
gotname:
#if	CMUCS
		if (proto == 0 || strcasecmp(p->s_proto, proto) == 0)
#else	CMUCS
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
#endif	CMUCS
			break;
	}
	if (!_serv_stayopen)
		endservent();
	return (p);
}
