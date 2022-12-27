/*
 **********************************************************************
 * HISTORY
 * $Log:	rewinddir.c,v $
 * Revision 2.2  89/05/14  11:52:45  gm0w
 * 	Created.
 * 	[89/05/04            gm0w]
 * 
 **********************************************************************
 */
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>

/*
 * Since seekdir is unsafe, we need an explicit routine for rewinddir
 */
void
rewinddir(dirp)
	register DIR *dirp;
{
	long curloc, base, offset;
	struct direct *dp;
	extern long lseek();

	if (dirp == NULL || dirp->dd_buf == NULL)
		return;
	dirp->dd_loc = 0;
	(void) lseek(dirp->dd_fd, (off_t)0, L_SET);
}
