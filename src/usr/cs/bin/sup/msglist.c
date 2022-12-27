/*
 **********************************************************************
 * HISTORY
 * 20-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed support for version 3 of SUP protocol.  Added type
 *	casting information for scantime to please lint.
 *
 * 29-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <c.h>
#include "sup.h"
#define MSGSUBR
#define MSGFILE
#include "supmsg.h"

extern TREE	*listT;			/* tree of files to list */
extern long	scantime;		/* time that collection was scanned */

static int listone (t)
register TREE *t;
{
	register int x;

	x = writestring (t->Tname);
	if (x == SCMOK)  x = writeint ((int)t->Tmode);
	if (x == SCMOK)  x = writeint ((int)t->Tflags);
	if (x == SCMOK)  x = writeint (t->Tmtime);
	return (x);
}

int msgflist ()
{
	register int x;
	if (server) {
		x = writemsg (MSGFLIST);
		if (x == SCMOK)  x = Tprocess (listT,listone);
		if (x == SCMOK)  x = writestring ((char *)NULL);
		if (x == SCMOK)  x = writeint ((int)scantime);
		if (x == SCMOK)  x = writemend ();
	} else {
		char *name;
		int mode,flags,mtime;
		register TREE *t;
		x = readmsg (MSGFLIST);
		if (x == SCMOK)  x = readstring (&name);
		while (x == SCMOK) {
			if (name == NULL)  break;
			x = readint (&mode);
			if (x == SCMOK)  x = readint (&flags);
			if (x == SCMOK)  x = readint (&mtime);
			if (x != SCMOK)  break;
			t = Tinsert (&listT,name,TRUE);
			free (name);
			t->Tmode = mode;
			t->Tflags = flags;
			t->Tmtime = mtime;
			x = readstring (&name);
		}
		if (x == SCMOK)  x = readint ((int *)&scantime);
		if (x == SCMOK)  x = readmend ();
	}
	return (x);
}
