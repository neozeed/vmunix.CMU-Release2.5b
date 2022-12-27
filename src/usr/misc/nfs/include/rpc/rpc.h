/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	rpc.h,v $
 * Revision 2.0  89/06/15  15:43:41  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *  89/03/09  20:58:55  rpd
 * 	More cleanup.
 * 
 *  89/02/25  19:14:35  gm0w
 * 	Changes for cleanup.
 * 
 *  89/01/18  01:01:46  jsb
 * 	Include file references.
 * 	[89/01/17  14:30:06  jsb]
 * 
 */
/* @(#)rpc.h	1.1 87/06/19 3.2/4.3NFSSRC */
/* @(#)rpc.h	1.2 86/10/28 NFSSRC */
/*	@(#)rpc.h 1.1 86/09/24 SMI	*/

/*
 * rpc.h, Just includes the billions of rpc header files necessary to 
 * do remote procedure calling.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */
#ifndef	__RPC_HEADER__
#define __RPC_HEADER__

#include <rpc/types.h>		/* some typedefs */
#include <netinet/in.h>

/* external data representation interfaces */
#include <rpc/xdr.h>		/* generic (de)serializer */

/* Client side only authentication */
#include <rpc/auth.h>		/* generic authenticator (client side) */

/* Client side (mostly) remote procedure call */
#include <rpc/clnt.h>		/* generic rpc stuff */

/* semi-private protocol headers */
#include <rpc/rpc_msg.h>	/* protocol for rpc messages */
#include <rpc/auth_unix.h>	/* protocol for unix style cred */

/* Server side only remote procedure callee */
#include <rpc/svc.h>		/* service manager and multiplexer */
#include <rpc/svc_auth.h>	/* service side authenticator */

#endif	!__RPC_HEADER__
