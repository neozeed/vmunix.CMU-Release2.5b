/****************************************
 *
 * HISTORY
 *
 * $Log:	klm_prot.c,v $
 * Revision 2.0  89/06/15  16:43:33  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 ****************************************/
/* @(#)klm_prot.c	1.2 86/11/17 NFSSRC */
#ifndef lint
static  char sccsid[] = "@(#)klm_prot.c 1.1 86/09/25 Copyr 1986 Sun Micro";
#endif

/*
 * Copyright (c) 1986 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/klm_prot.h>


bool_t
xdr_klm_stats(xdrs,objp)
	XDR *xdrs;
	klm_stats *objp;
{
	if (! xdr_enum(xdrs, (enum_t *) objp)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_lock(xdrs,objp)
	XDR *xdrs;
	klm_lock *objp;
{
	if (! xdr_string(xdrs, &objp->server_name, LM_MAXSTRLEN)) {
		return(FALSE);
	}
	if (! xdr_netobj(xdrs, &objp->fh)) {
		return(FALSE);
	}
	if (! xdr_int(xdrs, &objp->pid)) {
		return(FALSE);
	}
	if (! xdr_u_int(xdrs, &objp->l_offset)) {
		return(FALSE);
	}
	if (! xdr_u_int(xdrs, &objp->l_len)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_holder(xdrs,objp)
	XDR *xdrs;
	klm_holder *objp;
{
	if (! xdr_bool(xdrs, &objp->exclusive)) {
		return(FALSE);
	}
	if (! xdr_int(xdrs, &objp->svid)) {
		return(FALSE);
	}
	if (! xdr_u_int(xdrs, &objp->l_offset)) {
		return(FALSE);
	}
	if (! xdr_u_int(xdrs, &objp->l_len)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_stat(xdrs,objp)
	XDR *xdrs;
	klm_stat *objp;
{
	if (! xdr_klm_stats(xdrs, &objp->stat)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_testrply(xdrs,objp)
	XDR *xdrs;
	klm_testrply *objp;
{
	static struct xdr_discrim choices[] = {
		{ (int) klm_granted, xdr_void },
		{ (int) klm_denied, xdr_klm_holder },
		{ (int) klm_denied_nolocks, xdr_void },
		{ (int) klm_working, xdr_void },
		{ __dontcare__, NULL }
	};

	if (! xdr_union(xdrs, (enum_t *) &objp->stat, (char *) &objp->klm_testrply, choices, NULL)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_lockargs(xdrs,objp)
	XDR *xdrs;
	klm_lockargs *objp;
{
	if (! xdr_bool(xdrs, &objp->block)) {
		return(FALSE);
	}
	if (! xdr_bool(xdrs, &objp->exclusive)) {
		return(FALSE);
	}
	if (! xdr_klm_lock(xdrs, &objp->lock)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_testargs(xdrs,objp)
	XDR *xdrs;
	klm_testargs *objp;
{
	if (! xdr_bool(xdrs, &objp->exclusive)) {
		return(FALSE);
	}
	if (! xdr_klm_lock(xdrs, &objp->lock)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_klm_unlockargs(xdrs,objp)
	XDR *xdrs;
	klm_unlockargs *objp;
{
	if (! xdr_klm_lock(xdrs, &objp->lock)) {
		return(FALSE);
	}
	return(TRUE);
}


