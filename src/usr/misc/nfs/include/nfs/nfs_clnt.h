/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	nfs_clnt.h,v $
 * Revision 2.0  89/06/15  15:42:41  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *  89/03/09  20:54:01  rpd
 * 	More cleanup.
 * 
 *  89/02/25  19:09:13  gm0w
 * 	Changes for cleanup.
 * 
 *  89/01/18  00:55:04  jsb
 * 	Merged from NeXT sources.
 * 	[89/01/17  14:16:41  jsb]
 * 
 */
/* @(#)nfs_clnt.h	1.2 87/09/11 3.2/4.3NFSSRC */
/*      @(#)nfs_clnt.h 1.1 86/09/25 SMI      */

#ifndef	__NFS_CLNT_HEADER__
#define __NFS_CLNT_HEADER__

/*
 * vfs pointer to mount info
 */
#define vftomi(vfsp)	((struct mntinfo *)((vfsp)->vfs_data))

/*
 * vnode pointer to mount info
 */
#define vtomi(vp)	((struct mntinfo *)(((vp)->v_vfsp)->vfs_data))

/*
 * NFS vnode to server's block size
 */
#define vtoblksz(vp)	(vtomi(vp)->mi_bsize)


#define HOSTNAMESZ	32

/*
 * NFS private data per mounted file system
 */
struct mntinfo {
	struct sockaddr_in mi_addr;	/* server's address */
	struct vnode	*mi_rootvp;	/* root vnode */
	u_int		 mi_hard : 1;	/* hard or soft mount */
	u_int		 mi_printed : 1;/* not responding message printed */
	u_int		 mi_int : 1;	/* interrupts allowed on hard mount */
	u_int		 mi_down : 1;	/* server is down */
	int		 mi_refct;	/* active vnodes for this vfs */
	long		 mi_tsize;	/* transfer size (bytes) */
	long		 mi_stsize;	/* server's max transfer size (bytes) */
	long		 mi_bsize;	/* server's disk block size */
	int		 mi_mntno;	/* kludge to set client rdev for stat*/
	int		 mi_timeo;	/* inital timeout in 10th sec */
	int		 mi_retrans;	/* times to retry request */
	char		 mi_hostname[HOSTNAMESZ];	/* server name */
};

/*
 * enum to specifiy cache flushing action when file data is stale
 */
enum staleflush	{NOFLUSH, SFLUSH};

#endif	!__NFS_CLNT_HEADER__
