/*
 **********************************************************************
 * HISTORY
 * 20-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed support for version 3 of SUP protocol.  Changed
 *	logerr variable to logerror since logerr() is a new routine.
 *
 * 29-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */
#include "sup.h"
#define MSGSUBR
#define MSGFILE
#include "supmsg.h"

extern char	*logcrypt;		/* login encryption test */
extern char	*loguser;		/* login username */
extern char	*logpswd;		/* password for login */
extern int	logack;			/* login ack status */
extern char	*logerror;		/* error from login */

int msgflogin ()
{
	register int x;
	if (server) {
		x = readmsg (MSGFLOGIN);
		if (x == SCMOK)  x = readstring (&logcrypt);
		if (x == SCMOK)  x = readstring (&loguser);
		if (x == SCMOK)  x = readstring (&logpswd);
		if (x == SCMOK)  x = readmend ();
	} else {
		x = writemsg (MSGFLOGIN);
		if (x == SCMOK)  x = writestring (logcrypt);
		if (x == SCMOK)  x = writestring (loguser);
		if (x == SCMOK)  x = writestring (logpswd);
		if (x == SCMOK)  x = writemend ();
	}
	return (x);
}

int msgflogack ()
{
	register int x;
	if (server) {
		x = writemsg (MSGFLOGACK);
		if (x == SCMOK)  x = writeint (logack);
		if (x == SCMOK)  x = writestring (logerror);
		if (x == SCMOK)  x = writemend ();
	} else {
		x = readmsg (MSGFLOGACK);
		if (x == SCMOK)  x = readint (&logack);
		if (x == SCMOK)  x = readstring (&logerror);
		if (x == SCMOK)  x = readmend ();
	}
	return (x);
}
