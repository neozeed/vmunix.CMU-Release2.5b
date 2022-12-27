/*
 **********************************************************************
 * HISTORY
 * $Log:	opendir.c,v $
 * Revision 1.2  89/05/14  11:52:36  gm0w
 * 	Added code to use a dynamically allocated buffer for entries.
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
static char sccsid[] = "@(#)opendir.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/param.h>
#include <sys/dir.h>
#if	CMU
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/errno.h>
extern int errno;
#endif	/* CMU */

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
#if	CMU
	struct stat st;
	int pid;
#endif	/* CMU */

	if ((fd = open(name, 0)) == -1)
		return NULL;
#if	CMU
	(void) fcntl(fd, F_GETOWN, &pid);
	errno = 0;
	if (fstat(fd, &st) < 0 || (st.st_mode&S_IFMT) != S_IFDIR) {
		close (fd);
		errno = ENOTDIR;
		return NULL;
	}
#endif	/* CMU */
	if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) {
		close (fd);
		return NULL;
	}
#if	CMU
	dirp->dd_bufsiz = MAX(st.st_blksize, MAXBSIZE);
	if ((dirp->dd_buf = (char *)malloc(dirp->dd_bufsiz)) == NULL) {
		(void) free((char *)dirp);
		close (fd);
		return NULL;
	}
#endif	/* CMU */
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return dirp;
}
