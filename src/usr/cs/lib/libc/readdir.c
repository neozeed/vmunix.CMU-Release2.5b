/*
 **********************************************************************
 * HISTORY
 * $Log:	readdir.c,v $
 * Revision 1.2  89/05/14  11:52:40  gm0w
 * 	Added code to use a dynamically allocated buffer for entries.
 * 	Changed to use getdirentries instead of read.
 * 	[89/05/14            gm0w]
 * 
 **********************************************************************
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)readdir.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/param.h>
#include <sys/dir.h>

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct direct *dp;
#if	CMU
	long offset;
#endif	/* CMU */

#if	CMU
	if (dirp == NULL || dirp->dd_buf == NULL)
		return NULL;
#endif	/* CMU */
	for (;;) {
		if (dirp->dd_loc == 0) {
#if	CMU
			dirp->dd_size = getdirentries(dirp->dd_fd,
						      dirp->dd_buf, 
						      dirp->dd_bufsiz,
						      &offset);
#else	/* CMU */
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
#endif	/* CMU */
			if (dirp->dd_size <= 0)
				return NULL;
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen <= 0 ||
#if	CMU
		    dp->d_reclen > dirp->dd_bufsiz + 1 - dirp->dd_loc)
#else	/* CMU */
		    dp->d_reclen > DIRBLKSIZ + 1 - dirp->dd_loc)
#endif	/* CMU */
			return NULL;
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}
