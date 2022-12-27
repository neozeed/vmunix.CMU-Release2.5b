/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)gethostnamadr.c	6.31 (Berkeley) 3/14/88";
#endif /* LIBC_SCCS and not lint */

/*
 **********************************************************************
 * HISTORY
 * $Log:	gethostnmad.c,v $
 * Revision 1.14  89/08/18  14:17:23  gm0w
 * 	Added code to detect/remove trailing dots in hostnames when
 * 	falling back to the host table.
 * 	[89/08/18            gm0w]
 * 
 * 13-Apr-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Updated CMU changes for release 4.8 of BIND.
 *
 * 29-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added global _res_qname buffer which holds the query name
 *	returned by res_send() response.
 *
 * 12-Oct-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added checks for NULL pointer arguments.
 *
 * 24-Sep-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Don't use CS changes if RES_DEFNAMES is set.
 *
 * 11-Jan-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Use case-insensitive string comparison and uppercase host names
 *	and alias at CMU CS.  Don't return immediately after receiving
 *	T_PTR RR's since there may be additional information available
 *	in subsequent RR's.
 *
 **********************************************************************
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#define	MAXALIASES	35
#define	MAXADDRS	35

static char *h_addr_ptrs[MAXADDRS + 1];

static struct hostent host;
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ+1];
static struct in_addr host_addr;
static char HOSTDB[] = "/etc/hosts";
static FILE *hostf = NULL;
static char hostaddr[MAXADDRS];
static char *host_addrs[2];
static int stayopen = 0;
static char *any();

#if PACKETSZ > 1024
#define	MAXPACKET	PACKETSZ
#else
#define	MAXPACKET	1024
#endif

typedef union {
    HEADER hdr;
    u_char buf[MAXPACKET];
} querybuf;

static union {
    long al;
    char ac;
} align;


#if	CMU
#ifdef	DEBUG
int h_trace;
#endif	/* DEBUG */
#endif	/* CMU */
int h_errno;
extern errno;

static struct hostent *
getanswer(answer, anslen, iquery)
	querybuf *answer;
	int anslen;
	int iquery;
{
	register HEADER *hp;
	register u_char *cp;
	register int n;
	u_char *eom;
	char *bp, **ap;
	int type, class, buflen, ancount, qdcount;
	int haveanswer, getclass = C_ANY;
#if	CMU
	int haveptr;
#endif	/* CMU */
	char **hap;

#if	CMU
	bzero((char *)&host, sizeof(host));
#ifdef	DEBUG
	h_trace |= (1<<0);
#endif	/* DEBUG */
#endif	/* CMU */

	eom = answer->buf + anslen;
	/*
	 * find first satisfactory answer
	 */
	hp = &answer->hdr;
	ancount = ntohs(hp->ancount);
	qdcount = ntohs(hp->qdcount);
	bp = hostbuf;
	buflen = sizeof(hostbuf);
	cp = answer->buf + sizeof(HEADER);
	if (qdcount) {
		if (iquery) {
			if ((n = dn_expand((char *)answer->buf, eom,
			     cp, bp, buflen)) < 0) {
				h_errno = NO_RECOVERY;
				return ((struct hostent *) NULL);
			}
			cp += n + QFIXEDSZ;
			host.h_name = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
		} else
			cp += dn_skipname(cp, eom) + QFIXEDSZ;
		while (--qdcount > 0)
			cp += dn_skipname(cp, eom) + QFIXEDSZ;
	} else if (iquery) {
		if (hp->aa)
			h_errno = HOST_NOT_FOUND;
		else
			h_errno = TRY_AGAIN;
		return ((struct hostent *) NULL);
	}
	ap = host_aliases;
	host.h_aliases = host_aliases;
	hap = h_addr_ptrs;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
	host.h_addr_list = h_addr_ptrs;
#endif
	haveanswer = 0;
#if	CMU
	haveptr = 0;
#endif	/* CMU */
	while (--ancount >= 0 && cp < eom) {
		if ((n = dn_expand((char *)answer->buf, eom, cp, bp, buflen)) < 0)
			break;
		cp += n;
		type = _getshort(cp);
 		cp += sizeof(u_short);
		class = _getshort(cp);
 		cp += sizeof(u_short) + sizeof(u_long);
		n = _getshort(cp);
		cp += sizeof(u_short);
		if (type == T_CNAME) {
			cp += n;
			if (ap >= &host_aliases[MAXALIASES-1])
				continue;
			*ap++ = bp;
			n = strlen(bp) + 1;
			bp += n;
			buflen -= n;
			continue;
		}
#if	CMU
		if (type == T_PTR) {
#else	/* CMU */
		if (iquery && type == T_PTR) {
#endif	/* CMU */
			if ((n = dn_expand((char *)answer->buf, eom,
			    cp, bp, buflen)) < 0) {
				cp += n;
				continue;
			}
			cp += n;
			host.h_name = bp;
#if	CMU
			haveptr++;
			continue;
#else	/* CMU */
			return(&host);
#endif	/* CMU */
		}
		if (iquery || type != T_A)  {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				printf("unexpected answer type %d, size %d\n",
					type, n);
#endif
			cp += n;
			continue;
		}
		if (haveanswer) {
			if (n != host.h_length) {
				cp += n;
				continue;
			}
			if (class != getclass) {
				cp += n;
				continue;
			}
		} else {
			host.h_length = n;
			getclass = class;
			host.h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
			if (!iquery) {
				host.h_name = bp;
				bp += strlen(bp) + 1;
			}
		}

		bp += sizeof(align) - ((u_long)bp % sizeof(align));

		if (bp + n >= &hostbuf[sizeof(hostbuf)]) {
#ifdef DEBUG
			if (_res.options & RES_DEBUG)
				printf("size (%d) too big\n", n);
#endif
			break;
		}
		bcopy(cp, *hap++ = bp, n);
		bp +=n;
		cp += n;
		haveanswer++;
	}
#if	CMU
	if (haveanswer || haveptr) {
#else	/* CMU */
	if (haveanswer) {
#endif	/* CMU */
		*ap = NULL;
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
		*hap = NULL;
#else
		host.h_addr = h_addr_ptrs[0];
#endif
#if	CMU
#ifdef	DEBUG
		h_trace |= (1<<1);
#endif	/* DEBUG */
#endif	/* CMU */
		return (&host);
	} else {
		h_errno = TRY_AGAIN;
#if	CMU
#ifdef	DEBUG
		h_trace |= (1<<2);
#endif	/* DEBUG */
#endif	/* CMU */
		return ((struct hostent *) NULL);
	}
}

struct hostent *
gethostbyname(name)
	char *name;
{
	querybuf buf;
	register char *cp;
	int n;
	struct hostent *hp, *gethostdomain();
	extern struct hostent *_gethtbyname();

#if	CMU
	h_errno = 0;
#ifdef	DEBUG
	h_trace = 0;
#endif	/* DEBUG */
	if (name == NULL)
		return((struct hostent *) NULL);
#endif	/* CMU */

	/*
	 * disallow names consisting only of digits/dots, unless
	 * they end in a dot.
	 */
	if (isdigit(name[0]))
		for (cp = name;; ++cp) {
			if (!*cp) {
				if (*--cp == '.')
					break;
				h_errno = HOST_NOT_FOUND;
				return ((struct hostent *) NULL);
			}
			if (!isdigit(*cp) && *cp != '.') 
				break;
		}

#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<3);
#endif	/* DEBUG */
#endif	/* CMU */
	if ((n = res_search(name, C_IN, T_A, buf.buf, sizeof(buf))) < 0) {

#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("res_search failed\n");
#endif
		if (errno == ECONNREFUSED)
			return (_gethtbyname(name));
		else
			return ((struct hostent *) NULL);
	}
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<4);
#endif	/* DEBUG */
#endif	/* CMU */
	return (getanswer(&buf, n, 0));
}

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	int n;
	querybuf buf;
	register struct hostent *hp;
	char qbuf[MAXDNAME];
	extern struct hostent *_gethtbyaddr();
	
#if	CMU
	h_errno = 0;
#ifdef	DEBUG
	h_trace = 0;
#endif	/* DEBUG */
	if (addr == NULL)
		return((struct hostent *) NULL);
#endif	/* CMU */
	if (type != AF_INET)
		return ((struct hostent *) NULL);
	(void)sprintf(qbuf, "%d.%d.%d.%d.in-addr.arpa",
		((unsigned)addr[3] & 0xff),
		((unsigned)addr[2] & 0xff),
		((unsigned)addr[1] & 0xff),
		((unsigned)addr[0] & 0xff));
	n = res_query(qbuf, C_IN, T_PTR, (char *)&buf, sizeof(buf));
	if (n < 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("res_query failed\n");
#endif
		if (errno == ECONNREFUSED)
			hp = _gethtbyaddr(addr, len, type);
#if	CMU
		/* one presumes that this was the intent */
		else
			hp = (struct hostent *) NULL;
#ifdef	DEBUG
		h_trace |= (1<<5);
#endif	/* DEBUG */
		return (hp);
#else	/* CMU */
		return ((struct hostent *) NULL);
#endif	/* CMU */
	}
#if	CMU
	hp = getanswer(&buf, n, 0);
#else	/* CMU */
	hp = getanswer(&buf, n, 1);
#endif	/* CMU */
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<6);
#endif	/* DEBUG */
#endif	/* CMU */
	if (hp == NULL)
		return ((struct hostent *) NULL);
#if	CMU
	if (hp->h_length != 0) {
#ifdef	DEBUG
		h_trace |= (1<<7);
#endif	/* DEBUG */
		return(hp);
	}
#endif	/* CMU */
	hp->h_addrtype = type;
	hp->h_length = len;
	h_addr_ptrs[0] = (char *)&host_addr;
	h_addr_ptrs[1] = (char *)0;
	host_addr = *(struct in_addr *)addr;
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<8);
#endif	/* DEBUG */
#endif	/* CMU */
	return(hp);
}

_sethtent(f)
	int f;
{
	if (hostf == NULL)
		hostf = fopen(HOSTDB, "r" );
	else
		rewind(hostf);
	stayopen |= f;
}

_endhtent()
{
	if (hostf && !stayopen) {
		(void) fclose(hostf);
		hostf = NULL;
	}
}

struct hostent *
_gethtent()
{
	char *p;
	register char *cp, **q;

	if (hostf == NULL && (hostf = fopen(HOSTDB, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(hostbuf, BUFSIZ, hostf)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
	host.h_addr_list = host_addrs;
#endif
	host.h_addr = hostaddr;
	*((u_long *)host.h_addr) = inet_addr(p);
	host.h_length = sizeof (u_long);
	host.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

struct hostent *
_gethtbyname(name)
	char *name;
{
	register struct hostent *p;
	register char **cp;
#if	CMU
	register char *ptr;
	char buf[MAXHOSTNAMELEN];
#endif	/* CMU */
	
#if	CMU
	if (name == NULL || *name == '\0')
		return((struct hostent *) NULL);
	/*
	 * check for and possibly remove trailing dot(s)
	 */
	for (ptr = name; *ptr != '\0'; ptr++)
		;
	if (*--ptr == '.') {
		while (ptr > name && *(ptr-1) == '.')
			ptr--;
		if (ptr == name)
			return((struct hostent *) NULL);
		(void) bcopy(name, buf, ptr-name);
		buf[ptr-name] = '\0';
		name = buf;
	}
#endif	/* CMU */
	_sethtent(0);
	while (p = _gethtent()) {
		if (strcasecmp(p->h_name, name) == 0)
			break;
		for (cp = p->h_aliases; *cp != 0; cp++)
			if (strcasecmp(*cp, name) == 0)
				goto found;
	}
found:
	_endhtent();
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<9);
#endif	/* DEBUG */
#endif	/* CMU */
	return (p);
}

struct hostent *
_gethtbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	register struct hostent *p;

#if	CMU
	if (addr == NULL)
		return((struct hostent *) NULL);
#endif	/* CMU */
	_sethtent(0);
	while (p = _gethtent())
		if (p->h_addrtype == type && !bcmp(p->h_addr, addr, len))
			break;
	_endhtent();
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<10);
#endif	/* DEBUG */
#endif	/* CMU */
	return (p);
}
