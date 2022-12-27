/*
 * Copyright (c) 1980,1983,1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	@(#)netdb.h	5.9 (Berkeley) 4/5/88
 *
 **********************************************************************
 * HISTORY
 * $Log:	netdb.h,v $
 * Revision 2.0  89/06/15  15:38:18  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * Revision 2.1.1.1  89/06/15  14:14:40  dimitris
 * 
 * 	89/01/20  14:47:55  gm0w
 * 	Changed STDC conditional from CMUCS to CMU.
 * 	[88/12/17            gm0w]
 * 
 * 	88/12/14  23:33:01  mja
 * 	Added ANSI-C (and C++) compatible argument declarations.
 * 	[88/01/18            dld@cs.cmu.edu]
 * 
 **********************************************************************
 */

/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	int		n_addrtype;	/* net address type */
	unsigned long	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

struct rpcent {
        char    *r_name;        /* name of server for this rpc program */
        char    **r_aliases;    /* alias list */
        int     r_number;       /* rpc program number */
};

#if defined(CMU) && defined(__STDC__)
extern struct hostent
  *gethostbyname(const char*), *gethostbyaddr(const char*, int, int),
  *gethostent(void);
extern struct netent
  *getnetbyname(const char*), *getnetbyaddr(long, int), *getnetent(void);
extern struct servent
  *getservbyname(const char*, const char*), *getservbyport(int, const char*),
  *getservent(void);
extern struct protoent
  *getprotobyname(const char*), *getprotobynumber(int), *getprotoent(void);
extern void sethostent(int), endhostent(void);
#else
struct hostent	*gethostbyname(), *gethostbyaddr(), *gethostent();
struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
struct servent	*getservbyname(), *getservbyport(), *getservent();
struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();
#endif

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */
