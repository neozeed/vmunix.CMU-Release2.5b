/*
 **********************************************************************
 * HISTORY
 * $Log:	seekdir.c,v $
 * Revision 1.2  89/05/14  11:52:55  gm0w
 * 	Added code to make telldir difficult to load and impossible to use.
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
static char sccsid[] = "@(#)seekdir.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/param.h>
#include <sys/dir.h>
#if	CMU
#include <stdio.h>
#include <sys/signal.h>
#endif	/* CMU */

/*
 * seek to an entry in a directory.
 * Only values returned by "telldir" should be passed to seekdir.
 */
void
seekdir(dirp, loc)
	register DIR *dirp;
	long loc;
{
#if	CMU
	extern int DONT_USE_SEEKDIR_UNDER_MACH();
#else	/* CMU */
	long curloc, base, offset;
	struct direct *dp;
	extern long lseek();
#endif	/* CMU */

#if	CMU
	fprintf(stderr, "Don't use seekdir under MACH !!!\n");
	sigblock(~0);
	signal(SIGKILL, SIG_DFL);
	sigsetmask(~sigmask(SIGKILL));
	kill(getpid(), SIGKILL);
	abort();
	DONT_USE_SEEKDIR_UNDER_MACH();
#else	/* CMU */
	curloc = telldir(dirp);
	if (loc == curloc)
		return;
	base = loc & ~(DIRBLKSIZ - 1);
	offset = loc & (DIRBLKSIZ - 1);
	(void) lseek(dirp->dd_fd, base, 0);
	dirp->dd_loc = 0;
	while (dirp->dd_loc < offset) {
		dp = readdir(dirp);
		if (dp == NULL)
			return;
	}
#endif	/* CMU */
}
