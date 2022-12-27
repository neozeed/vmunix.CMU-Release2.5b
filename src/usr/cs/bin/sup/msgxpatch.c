/*
 **********************************************************************
 * HISTORY
 * 11-Feb-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added libc.h to fix calloc() lint warnings.
 *
 * 27-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */
#include <c.h>
#include <libc.h>
#include <stdio.h>
#include "sup.h"
#define MSGSUBR
#define MSGFILE
#include "supmsg.h"

extern int	xargc;			/* arg count for crosspatch */
extern char	**xargv;		/* arg array for crosspatch */

int msgfxpatch ()
{
	register int x;
	register int i;

	if (server) {
		x = readmsg (MSGFXPATCH);
		if (x != SCMOK)  return (x);
		x = readint (&xargc);
		if (x != SCMOK)  return (x);
		xargc += 2;
		xargv = (char **)calloc (sizeof (char *),(unsigned)xargc+1);
		if (xargv == NULL)
			return (SCMERR);
		for (i = 2; i < xargc; i++) {
			x = readstring (&xargv[i]);
			if (x != SCMOK)  return (x);
		}
		x = readmend ();
	} else {
		x = writemsg (MSGFXPATCH);
		if (x != SCMOK)  return (x);
		x = writeint (xargc);
		if (x != SCMOK)  return (x);
		for (i = 0; i < xargc; i++) {
			x = writestring (xargv[i]);
			if (x != SCMOK)  return (x);
		}
		x = writemend ();
	}
	return (x);
}
