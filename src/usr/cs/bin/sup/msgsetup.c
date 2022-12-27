/*
 **********************************************************************
 * HISTORY
 * 27-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added crosspatch support.
 *
 * 28-Jun-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code for "release" support.
 *
 * 20-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added type casting information to lasttime for lint.
 *
 * 29-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */
#include <c.h>
#include <stdio.h>
#include "sup.h"
#define MSGSUBR
#define MSGFILE
#include "supmsg.h"

extern int	xpatch;			/* setup crosspatch to a new client */
extern char	*xuser;			/* user,group,acct for crosspatch */
extern char	*collname;		/* base directory */
extern char	*basedir;		/* base directory */
extern int	basedev;		/* base directory device */
extern int	baseino;		/* base directory inode */
extern long	lasttime;		/* time of last upgrade */
extern int	listonly;		/* only listing files, no data xfer */
extern int	newonly;		/* only send new files */
extern char	*release;		/* release name */
extern int	setupack;		/* ack return value for setup */

int msgfsetup ()
{
	register int x;

	if (server) {
		x = readmsg (MSGFSETUP);
		if (x != SCMOK)  return (x);
		if (protver >= 7) {
			x = readint (&xpatch);
			if (x != SCMOK)  return (x);
		} else
			xpatch = FALSE;
		if (xpatch) {
			x = readstring (&xuser);
			if (x != SCMOK)  return (x);
			return (readmend ());
		}
		x = readstring (&collname);
		if (x == SCMOK)  x = readint ((int *)&lasttime);
		if (x == SCMOK)  x = readstring (&basedir);
		if (x == SCMOK)  x = readint (&basedev);
		if (x == SCMOK)  x = readint (&baseino);
		if (x == SCMOK)  x = readint (&listonly);
		if (x == SCMOK)  x = readint (&newonly);
		if (x == SCMOK)
			if (protver < 6)
				release = (char *)NULL;
			else
				x = readstring (&release);
		if (x == SCMOK)  x = readmend ();
	} else {
		x = writemsg (MSGFSETUP);
		if (x != SCMOK)  return (x);
		if (protver >= 7) {
			x = writeint (xpatch);
			if (x != SCMOK)  return (x);
		}
		if (xpatch) {
			x = writestring (xuser);
			if (x != SCMOK)  return (x);
			return (writemend ());
		}
		if (x == SCMOK)  x = writestring (collname);
		if (x == SCMOK)  x = writeint ((int)lasttime);
		if (x == SCMOK)  x = writestring (basedir);
		if (x == SCMOK)  x = writeint (basedev);
		if (x == SCMOK)  x = writeint (baseino);
		if (x == SCMOK)  x = writeint (listonly);
		if (x == SCMOK)  x = writeint (newonly);
		if (x == SCMOK && protver >= 6)  x = writestring (release);
		if (x == SCMOK)  x = writemend ();
	}
	return (x);
}

int msgfsetupack ()
{
	if (server)
		return (writemint (MSGFSETUPACK,setupack));
	return (readmint (MSGFSETUPACK,&setupack));
}
