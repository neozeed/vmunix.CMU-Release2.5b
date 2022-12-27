/*
 * Copyright (c) 1988 Regents of the University of California.
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
static char sccsid[] = "@(#)res_query.c	5.3 (Berkeley) 4/5/88";
#endif /* LIBC_SCCS and not lint */

/*
 **********************************************************************
 * HISTORY
 * $Log:	res_query.c,v $
 * Revision 1.3  89/03/10  06:45:38  parker
 * 	Fixed pointer bug in res_query().  Dimitris Varotsis both found
 * 	and fixed this one.
 * 	[89/02/22            parker]
 * 
 * 13-Apr-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added CMU changes to release 4.8 of BIND.  Record result of
 *	query in _res_qname.  Set and clear RES_RESPORT.  Uppercase
 *	queries and implement "localhost" support within resolver.
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
#include <strings.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#if PACKETSZ > 1024
#define MAXPACKET	PACKETSZ
#else
#define MAXPACKET	1024
#endif

#if	CMU
char _res_qname[BUFSIZ+1];
#ifdef DEBUG
extern int h_trace;
#endif	/* DEBUG */
#endif	/* CMU */
extern int errno;
int h_errno;

/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 * Caller must parse answer and determine whether it answers the question.
 */
res_query(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer buffer */
{
	char buf[MAXPACKET];
	HEADER *hp;
	int n;

#if	CMU
	_res_qname[0] = '\0';
#endif	/* CMU */
	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		return (-1);
#ifdef DEBUG
	if (_res.options & RES_DEBUG)
		printf("res_query(%s, %d, %d)\n", name, class, type);
#endif
	n = res_mkquery(QUERY, name, class, type, (char *)NULL, 0, NULL,
	    buf, sizeof(buf));

	if (n <= 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("res_query: mkquery failed\n");
#endif
		h_errno = NO_RECOVERY;
#if	CMU
		_res.options &= ~RES_RESPORT;
#ifdef	DEBUG
		h_trace |= (1<<11);
#endif	/* DEBUG */
#endif	/* CMU */
		return (n);
	}
	n = res_send(buf, n, answer, anslen);
#if	CMU
	_res.options &= ~RES_RESPORT;
#endif	/* CMU */
	if (n < 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("res_query: send error\n");
#endif
		h_errno = TRY_AGAIN;
		return(n);
	}

	hp = (HEADER *) answer;
	if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("rcode = %d, ancount=%d\n", hp->rcode,
			    ntohs(hp->ancount));
#endif
		switch (hp->rcode) {
			case NXDOMAIN:
				h_errno = HOST_NOT_FOUND;
				break;
			case SERVFAIL:
				h_errno = TRY_AGAIN;
				break;
			case NOERROR:
				h_errno = NO_DATA;
				break;
			case FORMERR:
			case NOTIMP:
			case REFUSED:
			default:
				h_errno = NO_RECOVERY;
				break;
		}
#if	CMU
		if (h_errno == NO_DATA && ntohs(hp->qdcount)) {
			if (dn_expand(	(char *) answer,
					(char *) answer + n,
					(char *) answer + sizeof(HEADER),
					_res_qname, sizeof(_res_qname)) < 0)
				_res_qname[0] = '\0';
		}
#endif	/* CMU */
		return (-1);
	}
	return(n);
}

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error number is left in h_errno.
 * Only useful for queries in the same name hierarchy as the local host
 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
 */
res_search(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	register char *cp, **domain;
	int n, ret, got_nodata = 0;
	char *hostalias();

	if ((_res.options & RES_INIT) == 0 && res_init() == -1)
		return (-1);

	errno = 0;
	h_errno = HOST_NOT_FOUND;		/* default, if we never query */
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<12);
#endif	/* DEBUG */
#endif	/* CMU */
	if (n == 0 && (cp = hostalias(name)))
#if	CMU
		return (res_querydomain(cp, (char *)NULL, class, type,
			answer, anslen));
#else	/* CMU */
		return (res_query(cp, class, type, answer, anslen));
#endif	/* CMU */

#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<13);
#endif	/* DEBUG */
#endif	/* CMU */
	if ((n == 0 || *--cp != '.') && (_res.options & RES_DEFNAMES))
#if	CMU
	{
#ifdef	DEBUG
	h_trace |= (1<<14);
#endif	/* DEBUG */
#endif	/* CMU */
	    for (domain = _res.dnsrch; *domain; domain++) {
		ret = res_querydomain(name, *domain, class, type,
		    answer, anslen);
		if (ret > 0)
			return (ret);
		/*
		 * If no server present, give up.
		 * If name isn't found in this domain,
		 * keep trying higher domains in the search list
		 * (if that's enabled).
		 * On a NO_DATA error, keep trying, otherwise
		 * a wildcard entry of another type could keep us
		 * from finding this entry higher in the domain.
		 * If we get some other error (non-authoritative negative
		 * answer or server failure), then stop searching up,
		 * but try the input name below in case it's fully-qualified.
		 */
		if (errno == ECONNREFUSED) {
			h_errno = TRY_AGAIN;
			return (-1);
		}
		if (h_errno == NO_DATA)
			got_nodata++;
		if ((h_errno != HOST_NOT_FOUND && h_errno != NO_DATA) ||
		    (_res.options & RES_DNSRCH) == 0)
			break;
		h_errno = 0;
	}
#if	CMU
	}
#endif	/* CMU */
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 */
#if	CMU
	if (n || (_res.options & RES_DEFNAMES) == 0)
#else	/* CMU */
	if (n)
#endif	/* CMU */
		return (res_querydomain(name, (char *)NULL, class, type,
		    answer, anslen));
	if (got_nodata)
		h_errno = NO_DATA;
#if	CMU
#ifdef	DEBUG
	h_trace |= (1<<15);
#endif	/* DEBUG */
#endif	/* CMU */
	return (-1);
}

/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
res_querydomain(name, domain, class, type, answer, anslen)
	char *name, *domain;
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	char nbuf[2*MAXDNAME+2];
	char *longname = nbuf;
	int n;

#ifdef DEBUG
	if (_res.options & RES_DEBUG)
		printf("res_querydomain(%s, %s, %d, %d)\n",
		    name, domain, class, type);
#endif
	if (domain == NULL) {
#if	CMU
		if ((_res.options & RES_DEFNAMES) == 0) {
#if	CMUCS
			static char localhost[] = "localhost";

#ifdef	DEBUG
			h_trace |= (1<<16);
#endif	/* DEBUG */
			n = strlen(name);
			if (n == sizeof(localhost) - 1 &&
			    strcasecmp(name, localhost) == 0) {
				if (gethostname(nbuf, sizeof(nbuf)) < 0)
					return(-1);
				name = nbuf;
			}
#endif	/* CMUCS */
			while (*name)
				if (islower(*name))
					*longname++ = toupper(*name++);
				else
					*longname++ = *name++;
			if ((longname-1) >= nbuf && *(longname-1) == '.') {
				while ((longname-1) >= nbuf &&
					*(longname-1) == '.')
					longname--;
				_res.options &= ~RES_RESPORT;
			} else
				_res.options |= RES_RESPORT;
			*longname = '\0';
			longname = nbuf;
#ifdef	DEBUG
			h_trace |= (1<<17);
#endif	/* DEBUG */
		} else {
#endif	/* CMU */
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = strlen(name) - 1;
		if (name[n] == '.' && n < sizeof(nbuf) - 1) {
			bcopy(name, nbuf, n);
			nbuf[n] = '\0';
		} else
			longname = name;
#if	CMU
		}
#endif	/* CMU */
	} else
		(void)sprintf(nbuf, "%.*s.%.*s",
		    MAXDNAME, name, MAXDNAME, domain);

	return (res_query(longname, class, type, answer, anslen));
}

char *
hostalias(name)
	register char *name;
{
	register char *C1, *C2;
	FILE *fp;
	char *file, *getenv(), *strcpy(), *strncpy();
	char buf[BUFSIZ];
	static char abuf[MAXDNAME];

#if	CMU
	if (name == NULL)
		return(NULL);
#endif	/* CMU */
	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return (NULL);
	buf[sizeof(buf) - 1] = '\0';
	while (fgets(buf, sizeof(buf), fp)) {
		for (C1 = buf; *C1 && !isspace(*C1); ++C1);
		if (!*C1)
			break;
		*C1 = '\0';
		if (!strcasecmp(buf, name)) {
			while (isspace(*++C1));
			if (!*C1)
				break;
			for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2);
			abuf[sizeof(abuf) - 1] = *C2 = '\0';
			(void)strncpy(abuf, C1, sizeof(abuf) - 1);
			fclose(fp);
			return (abuf);
		}
	}
	fclose(fp);
	return (NULL);
}
