/****************************************
 *
 * HISTORY
 *
 * $Log:	util.c,v $
 * Revision 2.0  89/06/15  16:45:18  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)util.c	1.2 86/11/17 NFSSRC */
#ifndef lint
static  char sccsid[] = "@(#)util.c 1.1 86/09/25 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <netdb.h>
#include <sys/socket.h>

getrpcport(host, prognum, versnum, proto)
	char *host;
{
	struct sockaddr_in addr;
	struct hostent *hp;

	if ((hp = gethostbyname(host)) == NULL)
		return (0);
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port =  0;
	return (pmap_getport(&addr, prognum, versnum, proto));
}
