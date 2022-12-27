/****************************************
 *
 * HISTORY
 *
 * $Log:	rwallxdr.c,v $
 * Revision 2.0  89/06/15  16:43:15  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)rwallxdr.c	1.2 86/11/17 NFSSRC */
#ifndef lint
static  char sccsid[] = "@(#)rwallxdr.c 1.1 86/09/25 Copyr 1985 Sun Micro";
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/rwall.h>

rwall(host, msg)
	char *host;
	char *msg;
{
	return (callrpc(host, WALLPROG, WALLVERS, WALLPROC_WALL,
	    xdr_wrapstring, &msg,  xdr_void, NULL));
}
