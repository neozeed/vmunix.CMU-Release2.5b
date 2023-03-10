/****************************************
 *
 * HISTORY
 *
 * $Log:	dbx_rpc.c,v $
 * Revision 2.0  89/06/15  16:40:58  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)dbx_rpc.c	1.2 86/10/28 NFSSRC */
#ifndef lint
static	char sccsid[] = "@(#)dbx_rpc.c 1.1 86/09/24 Copyr 1985 Sun Micro";
#endif

/*
 * This file is optionally brought in by including a
 * "psuedo-device dbx" line in the config file.  It is
 * compiled using the "-g" flag to generate structure
 * information which is used by dbx with the -k flag.
 */

#include "../h/param.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../netinet/in.h"

#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../rpc/rpc_msg.h"
#include "../rpc/svc.h"
#include "../rpc/svc_auth.h" 
#include "../rpc/auth_unix.h"
#include "../rpc/pmap_prot.h"
#include "../rpc/pmap_clnt.h"
