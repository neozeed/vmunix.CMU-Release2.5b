/*
 **********************************************************************
 * HISTORY
 * 27-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed nameserver support.
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

extern int	pgmver;			/* program version of partner */
extern int	pgmversion;		/* my program version */
extern char	*scmver;		/* scm version of partner */
extern int	fspid;			/* process id of fileserver */

int msgfsignon ()
{
	register int x;

	if (server) {
		x = readmsg (MSGFSIGNON);
		if (x == SCMOK)  x = readint (&protver);
		if (x == SCMOK)  x = readint (&pgmver);
		if (x == SCMOK)  x = readstring (&scmver);
		if (x == SCMOK)  x = readmend ();
	} else {
		x = writemsg (MSGFSIGNON);
		if (x == SCMOK)  x = writeint (PROTOVERSION);
		if (x == SCMOK)  x = writeint (pgmversion);
		if (x == SCMOK)  x = writestring (scmversion);
		if (x == SCMOK)  x = writemend ();
	}
	return (x);
}

int msgfsignonack ()
{
	register int x;

	if (server) {
		x = writemsg (MSGFSIGNONACK);
		if (x == SCMOK)  x = writeint (PROTOVERSION);
		if (x == SCMOK)  x = writeint (pgmversion);
		if (x == SCMOK)  x = writestring (scmversion);
		if (x == SCMOK)  x = writeint (fspid);
		if (x == SCMOK)  x = writemend ();
	} else {
		x = readmsg (MSGFSIGNONACK);
		if (x == SCMOK)  x = readint (&protver);
		if (x == SCMOK)  x = readint (&pgmver);
		if (x == SCMOK)  x = readstring (&scmver);
		if (x == SCMOK)  x = readint (&fspid);
		if (x == SCMOK)  x = readmend ();
	}
	return (x);
}
