/*
 **********************************************************************
 * HISTORY
 * 20-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed support for version 3 of SUP protocol.
 *
 * 29-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 */
#include <stdio.h>
#include <c.h>
#include "sup.h"
#define MSGSUBR
#define MSGFILE
#include "supmsg.h"

extern TREE	*upgradeT;		/* pointer to file being upgraded */

int msgfsend ()
{
	if (server)
		return (readmnull (MSGFSEND));
	return (writemnull (MSGFSEND));
}

static int writeone (t)
register TREE *t;
{
	return (writestring (t->Tname));
}

/* VARARGS1 */
int msgfrecv (xferfile,args)
int (*xferfile) ();
int args;
{
	register int x;
	register TREE *t = upgradeT;
	if (server) {
		x = writemsg (MSGFRECV);
		if (t == NULL) {
			if (x == SCMOK)  x = writestring ((char *)NULL);
			if (x == SCMOK)  x = writemend ();
			return (x);
		}
		if (x == SCMOK)  x = writestring (t->Tname);
		if (x == SCMOK)  x = writeint (t->Tmode);
		if (t->Tmode == 0) {
			if (x == SCMOK)  x = writemend ();
			return (x);
		}
		if (x == SCMOK)  x = writeint (t->Tflags);
		if (x == SCMOK)  x = writestring (t->Tuser);
		if (x == SCMOK)  x = writestring (t->Tgroup);
		if (x == SCMOK)  x = writeint (t->Tmtime);
		if (x == SCMOK)  x = Tprocess (t->Tlink,writeone);
		if (x == SCMOK)  x = writestring ((char *)NULL);
		if (x == SCMOK)  x = Tprocess (t->Texec,writeone);
		if (x == SCMOK)  x = writestring ((char *)NULL);
		if (x == SCMOK)  x = (*xferfile) (t,&args);
		if (x == SCMOK)  x = writemend ();
	} else {
		char *linkname,*execcmd;
		if (t == NULL)  return (SCMERR);
		x = readmsg (MSGFRECV);
		if (x == SCMOK)  x = readstring (&t->Tname);
		if (x == SCMOK && t->Tname == NULL) {
			x = readmend ();
			if (x == SCMOK)  x = (*xferfile) (NULL,&args);
			return (x);
		}
		if (x == SCMOK)  x = readint (&t->Tmode);
		if (t->Tmode == 0) {
			x = readmend ();
			if (x == SCMOK)  x = (*xferfile) (t,&args);
			return (x);
		}
		if (x == SCMOK)  x = readint (&t->Tflags);
		if (x == SCMOK)  x = readstring (&t->Tuser);
		if (x == SCMOK)  x = readstring (&t->Tgroup);
		if (x == SCMOK)  x = readint (&t->Tmtime);
		t->Tlink = NULL;
		if (x == SCMOK)  x = readstring (&linkname);
		while (x == SCMOK) {
			if (linkname == NULL)  break;
			(void) Tinsert (&t->Tlink,linkname,FALSE);
			free (linkname);
			x = readstring (&linkname);
		}
		t->Texec = NULL;
		if (x == SCMOK)  x = readstring (&execcmd);
		while (x == SCMOK) {
			if (execcmd == NULL)  break;
			(void) Tinsert (&t->Texec,execcmd,FALSE);
			free (execcmd);
			x = readstring (&execcmd);
		}
		if (x == SCMOK)  x = (*xferfile) (t,&args);
		if (x == SCMOK)  x = readmend ();
	}
	return (x);
}
