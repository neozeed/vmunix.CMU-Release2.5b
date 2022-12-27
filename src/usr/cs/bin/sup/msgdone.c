/*
 **********************************************************************
 * HISTORY
 * 27-May-87  Doug Philips (dwp) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */
#include "sup.h"
#define MSGSUBR
#define MSGFILE
#include "supmsg.h"

extern int	doneack;
extern char	*donereason;

int msgfdone ()
{
	register int x;

	if (protver < 6) {
		printf ("Error, msgfdone should not have been called.");
		return (SCMERR);
	}
	if (server) {
		x = readmsg (MSGFDONE);
		if (x == SCMOK)  x = readint (&doneack);
		if (x == SCMOK)  x = readstring (&donereason);
		if (x == SCMOK)  x = readmend ();
	} else {
		x = writemsg (MSGFDONE);
		if (x == SCMOK)  x = writeint (doneack);
		if (x == SCMOK)  x = writestring (donereason);
		if (x == SCMOK)  x = writemend ();
	}
	return (x);
}
