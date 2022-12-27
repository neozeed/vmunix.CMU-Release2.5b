/* dosetpath -- setpath built-in command
 *
 **********************************************************************
 * HISTORY
 * 08-May-88  Richard Draves (rpd) at Carnegie-Mellon University
 *	Major changes to remove artificial limits on sizes and numbers
 *	of paths.
 *
 **********************************************************************
 */

#include "sh.h"

static char *syspaths[] = {"PATH","CPATH","LPATH","MPATH","EPATH",0};
#define LOCALSYSPATH	"/usr/cs"

int dosetpath (arglist)
char **arglist;
{
	extern char *getenv();

	char **pathvars, **cmdargs;
	char **paths, **spaths, **cmds;
	unsigned int npaths, ncmds;
	int i, sysflag;

	for (i = 1; arglist[i] && (arglist[i][0] != '-'); i++);
	npaths = i - 1;

	cmdargs = &arglist[i];
	for (; arglist[i]; i++);
	ncmds = i - npaths - 1;

	if (npaths) {
		sysflag = 0;
		pathvars = &arglist[1];
	} else {
		sysflag = 1;
		npaths = (sizeof syspaths / sizeof *syspaths) - 1;
		pathvars = syspaths;
	}

	/* note that npaths != 0 */

	spaths = (char **) malloc(npaths * sizeof *spaths);
	paths = (char **) malloc((npaths + 1) * sizeof *paths);
	cmds = (char **) malloc((ncmds + 1) * sizeof *cmds);
	if (!spaths || !paths || !cmds) {
		printf("setpath: not enough core\n");
		goto abort1;
	}

	/* needs to be all zero, in case we abort */
	bzero((char *) spaths, npaths * sizeof *spaths);

	for (i = 0; i < npaths; i++) {
		char *val = getenv(pathvars[i]);
		if (val == 0)  val = "";

		spaths[i] = malloc(strlen(pathvars[i]) + strlen(val) + 2);
		if (!spaths[i]) {
			printf("setpath: not enough core\n");
			goto abort2;
		}

		(void) strcpy(spaths[i], pathvars[i]);
		(void) strcat(spaths[i], "=");
		(void) strcat(spaths[i], val);
		paths[i] = spaths[i];
	}
	paths[npaths] = 0;

	for (i = 0; i < ncmds; i++) {
		char *val = globone (cmdargs[i]);
		if (val == 0) goto abort2;
		cmds[i] = val;
	}
	cmds[ncmds] = 0;

	if (setpath(paths, cmds, LOCALSYSPATH, sysflag, 1) < 0)
		goto abort2;

	for (i=0; i<npaths; i++) {
		char *val;

		for (val=paths[i]; val && *val && *val != '='; val++) ;
		if (val && *val == '=') {
			*val++ = '\0';
			setenv (paths[i],val);
			if (strcmp(paths[i],"PATH") == 0) {
				importpath (val);
				if (havhash) dohash();
			}
			*--val = '=';
		}

		/* we are relying details of the implementation of setpath */
		xfree(paths[i]);
	}

	/* there is a possible memory leak here. if globone malloc'd the
	   command strings they won't get freed */

      abort2:
	for (i = 0; i < npaths; i++)
		if (spaths[i]) free(spaths[i]);

      abort1:
	if (spaths) free((char *)spaths);
	if (paths) free((char *)paths);
	if (cmds) free((char *)cmds);
}
