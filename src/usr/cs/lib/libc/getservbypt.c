/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getservbyport.c	5.3 (Berkeley) 5/19/86";
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
getservbyport(port, proto)
	int port;
	char *proto;
{
	register struct servent *p;

	setservent(_serv_stayopen);
	while (p = getservent()) {
		if (p->s_port != port)
			continue;
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
