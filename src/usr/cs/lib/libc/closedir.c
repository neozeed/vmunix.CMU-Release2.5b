/*
 **********************************************************************
 * HISTORY
 * $Log:	closedir.c,v $
 * Revision 1.2  89/08/16  17:04:45  gm0w
 * 	Added code to free dirp->dd_buf.
 * 	[89/08/16            gm0w]
 * 
 **********************************************************************
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)closedir.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/param.h>
#include <sys/dir.h>

/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	dirp->dd_fd = -1;
	dirp->dd_loc = 0;
#if	CMU
	free(dirp->dd_buf);
#endif	/* CMU */
	free(dirp);
}
