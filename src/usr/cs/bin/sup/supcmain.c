/*
 *	sup -- Software Upgrade Protocol client process
 *
 *	Usage:  sup [ flags ] [ supfile ] [ collection ... ]
 *
 *	The only required argument to sup is the name of a supfile.  It
 *	must either be given explicitly on the command line, or the -s
 *	flag must be specified.  If the -s flag is given, the system
 *	supfile will be used and a supfile command argument should not be
 *	specified.  The list of collections is optional and if specified
 *	will be the only collections upgraded.  The following flags affect
 *	all collections specified.
 *
 *	-s	"system upgrade" flag
 *			As described above.
 *
 *	-t	"upgrade time" flag
 *			When this flag is given, Sup will print the time
 *			that each collection was last upgraded, rather than
 *			performing actual upgrades.
 *
 *	-R	"resource pause" flag
 *			Sup will not disable resource pausing and will not
 *			make filesystem space checks.
 *
 *	-N	"debug network" flag
 *			Sup will trace messages sent and received that
 *			implement the Sup network protocol.
 *
 *	-P	"debug ports" flag
 *	    		Sup will use a set of non-privileged network
 *			ports reserved for debugging purposes.
 *
 *	-X	"crosspatch" flag
 *	    		Sup is being run remotely with a crosspatch.
 *			Need to be carefull as we may be running as root
 *			instead of collection owner.
 *
 *	The remaining flags affect all collections unless an explicit list
 *	of collections are given with the flags.  Multiple flags may be
 *	specified together that affect the same collections.  For the sake
 *	of convience, any flags that always affect all collections can be
 *	specified with flags that affect only some collections.  For
 *	example, "sup -sde=coll1,coll2" would perform a system upgrade,
 *	and the first two collections would allow both file deletions and
 *	command executions.  Note that this is not the same command as
 *	"sup -sde=coll1 coll2", which would perform a system upgrade of
 *	just the coll2 collection and would ignore the flags given for the
 *	coll1 collection.
 *
 *	-a	"all files" flag
 *			All files in the collection will be copied from
 *			the repository, regardless of their status on the
 *			current machine.  Because of this, it is a very
 *			expensive operation and should only be done for
 *			small collections if data corruption is suspected
 *			and been confirmed.  In most cases, the -o flag
 *			should be sufficient.
 *
 *	-b	"backup files" flag
 *			If the -b flag if given, or the "backup" supfile
 *			option is specified, the contents of regular files
 *			on the local system will be saved before they are
 *			overwritten with new data.  The data will be saved
 *			in a subdirectory called "BACKUP" in the directory
 *			containing the original version of the file, in a
 *			file with the same non-directory part of the file
 *			name.  The files to backup are specified by the
 *			list file on the repository.
 *
 *	-B	"don't backup files" flag
 *			The -B flag overrides and disables the -b flag and
 *			the "backup" supfile option.
 *
 *	-d	"delete files" flag
 *			Files that are no longer in the collection on the
 *			repository will be deleted if present on the local
 *			machine.  This may also be specified in a supfile
 *			with the "delete" option.
 *
 *	-D	"don't delete files" flag
 *			The -D flag overrides and disables the -d flag and
 *			the "delete" supfile option.
 *
 *	-e	"execute files" flag
 *			Sup will execute commands sent from the repository
 *			that should be run when a file is upgraded.  If
 *			the -e flag is omitted, Sup will print a message
 *			that specifies the command to execute.  This may
 *			also be specified in a supfile with the "execute"
 *			option.
 *
 *	-E	"don't execute files" flag
 *			The -E flag overrides and disables the -e flag and
 *			the "execute" supfile option.
 *
 *	-f	"file listing" flag
 *			A "list-only" upgrade will be performed.  Messages
 *			will be printed that indicate what would happen if
 *			an actual upgrade were done.
 *
 *	-l	"local upgrade" flag
 *			Normally, Sup will not upgrade a collection if the
 *			repository is on the same machine.  This allows
 *			users to run upgrades on all machines without
 *			having to make special checks for the repository
 *			machine.  If the -l flag is specified, collections
 *			will be upgraded even if the repository is local.
 *
 *	-m	"mail" flag
 *			Normally, Sup used standard output for messages.
 *			If the -m flag if given, Sup will send mail to the
 *			user running Sup, or a user specified with the
 *			"notify" supfile option, that contains messages
 *			printed by Sup.
 *
 *	-o	"old files" flag
 *			Sup will normally only upgrade files that have
 *			changed on the repository since the last time an
 *			upgrade was performed.  The -o flag, or the "old"
 *			supfile option, will cause Sup to check all files
 *			in the collection for changes instead of just the
 *			new ones.
 *
 *	-O	"not old files" flag
 *			The -O flag overrides and disables the -o flag and
 *			the "old" supfile option.
 *
 *	-v	"verbose" flag
 *			Normally, Sup will only print messages if there
 *			are problems.  This flag causes Sup to also print
 *			messages during normal progress showing what Sup
 *			is doing.
 *
 **********************************************************************
 * HISTORY
 * 27-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added crosspatch support (is currently ignored).
 *
 * 28-Jun-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code for "release" support.
 *
 * 25-May-87  Doug Philips (dwp) at Carnegie-Mellon University
 *	Split into several files.  This is the main program and
 *	command line processing and old history log. [V5.21]
 *
 * 21-May-87  Chriss Stephens (chriss) at Carnegie Mellon University
 *	Merged divergent CS and ECE versions. ifdeffed out the resource
 *	pausing code - only compiled in if CMUCS defined. [V5.21a]
 *
 * 20-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed support for version 3 of SUP protocol.  Added changes
 *	to make lint happy.  Added calls to new logging routines. [V5.20]
 *
 * 01-Apr-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added -R switch to reenable resource pausing, which is currently
 *	disabled by default.  Added code to check for free disk space
 *	available on the target filesystem so that sup shouldn't run the
 *	system out of disk space as frequently. [V5.19]
 *
 * 19-Sep-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed default supfile name for system collections when -t
 *	is specified to use FILESUPTDEFAULT; added missing new-line
 *	in retry message. [V5.18]
 *
 * 21-Jun-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Missed a caller to a routine which had an extra argument added
 *	to it last edit. [V5.17]
 *
 * 07-Jun-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed getcoll() so that fatal errors are checked immediately
 *	instead of after sleeping for a little while.  Changed all
 *	rm -rf commands to rmdir since the Mach folks keep deleting
 *	their root and /usr directory trees.  Reversed the order of
 *	delete commands to that directories will possibly empty so
 *	that the rmdir's work. [V5.16]
 *
 * 30-May-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed temporary file names to #n.sup format. [V5.15]
 *
 * 19-Feb-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Moved PGMVERSION to supvers.c module. [V5.14]
 *
 * 06-Feb-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added check for file type before unlink when receiving a
 *	symbolic link.  Now runs "rm -rf" if the file type is a
 *	directory. [V5.13]
 *
 * 03-Feb-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fixed small bug in signon that didn't retry connections if an
 *	error occured on the first attempt to connect. [V5.12]
 *
 * 26-Jan-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	New command interface.  Added -bBDEO flags and "delete",
 *	"execute" and "old" supfile options.  Changed -d to work
 *	correctly without implying -o. [V5.11]
 *
 * 21-Jan-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fix incorrect check for supfile changing.  Flush output buffers
 *	before restart. [V5.10]
 *
 * 17-Jan-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Add call to requestend() after connection errors are retried to
 *	free file descriptors. [V5.9]
 *
 * 15-Jan-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fix SERIOUS merge error from previous edit.  Added notify
 *	when execute command fails. [V5.8]
 *
 * 11-Jan-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed ugconvert to clear setuid/setgid bits if it doesn't use
 *	the user and group specified by the remote system.  Changed
 *	execute code to invalidate collection if execute command returns
 *	with a non-zero exit status.  Added support for execv() of
 *	original arguments of supfile is upgraded sucessfully.  Changed
 *	copyfile to always use a temp file if possible. [V5.7]
 *
 * 04-Jan-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added support for fileserver busy messages and new nameserver
 *	protocol to support multiple repositories per collection.
 *	Added code to lock collections with lock files. [V5.6]
 *
 * 29-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Major rewrite for protocol version 4. [V4.5]
 *
 * 12-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed to check for DIFFERENT mtime (again). [V3.4]
 *
 * 08-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Replaced [ug]convert routines with ugconvert routine so that an
 *	appropriate group will be used if the default user is used.
 *	Changed switch parsing to allow multiple switches to be specified
 *	at the same time. [V3.3]
 *
 * 04-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added test to request a new copy of an old file that already
 *	exists if the mtime is different. [V3.2]
 *
 * 24-Nov-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added -l switch to enable upgrades from local repositories.
 *
 * 03-Nov-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Minor change in order -t prints so that columns line up.
 *
 * 22-Oct-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to implement retry flag and pass this on to request().
 *
 * 22-Sep-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Merged 4.1 and 4.2 versions together.
 *
 * 04-Jun-85  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for 4.2 BSD.
 *
 **********************************************************************
 */

#define MSGFILE
#include "supcdefs.h"

/*********************************************
 ***    G L O B A L   V A R I A B L E S    ***
 *********************************************/

char program[] = "SUP";			/* program name for SCM messages */
int progpid = -1;			/* and process id */

COLLECTION *firstC,*thisC;		/* collection list pointer */

extern int dontjump;			/* disable longjmp */
extern int scmdebug;			/* SCM debugging flag */

int sysflag;				/* system upgrade flag */
int timeflag;				/* print times flag */
#if	CMUCS
int rpauseflag;				/* don't disable resource pausing */
#endif	CMUCS
int xpatchflag;				/* crosspatched with remote system */
int portdebug;				/* network debugging ports */

/*************************************
 ***    M A I N   R O U T I N E    ***
 *************************************/

main (argc,argv)
int argc;
char **argv;
{
	char *init ();
	char *progname,*supfname;
	int restart,sfdev,sfino,sfmtime;
	struct stat sbuf;
	struct sigvec ignvec,oldvec;

	/* initialize global variables */
	pgmversion = PGMVERSION;	/* export version number */
	server = FALSE;			/* export that we're not a server */
	collname = NULL;		/* no current collection yet */
	dontjump = TRUE;		/* clear setjmp buffer */
	progname = salloc (argv[0]);

	supfname = init (argc,argv);
	restart = -1;			/* don't make restart checks */
	if (*progname == '/' && *supfname == '/') {
		if (stat (supfname,&sbuf) < 0)
			logerr ("Can't stat supfile %s",supfname);
		else {
			sfdev = sbuf.st_dev;
			sfino = sbuf.st_ino;
			sfmtime = sbuf.st_mtime;
			restart = 0;
		}
	}
	if (timeflag) {
		for (thisC = firstC; thisC; thisC = thisC->Cnext)
			prtime ();
	} else {
		/* ignore network pipe signals */
		ignvec.sv_handler = SIG_IGN;
		ignvec.sv_onstack = 0;
		ignvec.sv_mask = 0;
		(void) sigvec (SIGPIPE,&ignvec,&oldvec);
		getnams ();		/* find unknown repositories */
		for (thisC = firstC; thisC; thisC = thisC->Cnext) {
			getcoll ();	/* upgrade each collection */
			if (restart == 0) {
				if (stat (supfname,&sbuf) < 0)
					logerr ("Can't stat supfile %s",
						supfname);
				else if (sfmtime != sbuf.st_mtime ||
					 sfino != sbuf.st_ino ||
					 sfdev != sbuf.st_dev) {
					restart = 1;
					break;
				}
			}
		}
		endpwent ();		/* close /etc/passwd */
		(void) endgrent ();	/* close /etc/group */
		if (restart == 1) {
			int fd;
			loginfo ("SUP Restarting %s with new supfile %s",
				progname,supfname);
			for (fd = getdtablesize (); fd > 3; fd--)
				(void) close (fd);
			execv (progname,argv);
			logquit (1,"Restart failed");
		}
	}
	while (thisC = firstC) {
		firstC = firstC->Cnext;
		free (thisC->Cname);
		Tfree (&thisC->Chtree);
		free (thisC->Cbase);
		if (thisC->Chbase)  free (thisC->Chbase);
		if (thisC->Cprefix)  free (thisC->Cprefix);
		if (thisC->Crelease)  free (thisC->Crelease);
		if (thisC->Cnotify)  free (thisC->Cnotify);
		if (thisC->Clogin)  free (thisC->Clogin);
		if (thisC->Cpswd)  free (thisC->Cpswd);
		if (thisC->Ccrypt)  free (thisC->Ccrypt);
		free ((char *)thisC);
	}
	exit (0);
}

/*****************************************
 ***    I N I T I A L I Z A T I O N    ***
 *****************************************/
/* Set up collection list from supfile.  Check all fields except
 * hostname to be sure they make sense.
 */

#define Toflags	Tflags
#define Taflags	Tmode
#define Twant	Tuid
#define Tcount	Tgid

doswitch (argp,collTp,oflagsp,aflagsp)
char *argp;
register TREE **collTp;
int *oflagsp,*aflagsp;
{
	register TREE *t;
	register char *coll;
	register int oflags,aflags;

	oflags = aflags = 0;
	for (;;) {
		switch (*argp) {
		default:
			logerr ("Invalid flag '%c' ignored",*argp);
			break;
		case '\0':
		case '=':
			if (*argp++ == '\0' || *argp == '\0') {
				*oflagsp |= oflags;
				*oflagsp &= ~aflags;
				*aflagsp |= aflags;
				*aflagsp &= ~oflags;
				return;
			}
			do {
				coll = nxtarg (&argp,", \t");
				t = Tinsert (collTp,coll,TRUE);
				t->Toflags |= oflags;
				t->Toflags &= ~aflags;
				t->Taflags |= aflags;
				t->Taflags &= ~oflags;
				argp = skipover (argp,", \t");
			} while (*argp);
			return;
		case 'N':
			scmdebug++;
			break;
		case 'P':
			portdebug = TRUE;
			break;
		case 'R':
#if	CMUCS
			rpauseflag = TRUE;
#endif	CMUCS
			break;
		case 'X':
			xpatchflag = TRUE;
			break;
		case 's':
			sysflag = TRUE;
			break;
		case 't':
			timeflag = TRUE;
			break;
		case 'a':
			oflags |= CFALL;
			break;
		case 'b':
			oflags |= CFBACKUP;
			aflags &= ~CFBACKUP;
			break;
		case 'B':
			oflags &= ~CFBACKUP;
			aflags |= CFBACKUP;
			break;
		case 'd':
			oflags |= CFDELETE;
			aflags &= ~CFDELETE;
			break;
		case 'D':
			oflags &= ~CFDELETE;
			aflags |= CFDELETE;
			break;
		case 'e':
			oflags |= CFEXECUTE;
			aflags &= ~CFEXECUTE;
			break;
		case 'E':
			oflags &= ~CFEXECUTE;
			aflags |= CFEXECUTE;
			break;
		case 'f':
			oflags |= CFLIST;
			break;
		case 'l':
			oflags |= CFLOCAL;
			break;
		case 'm':
			oflags |= CFMAIL;
			break;
		case 'o':
			oflags |= CFOLD;
			aflags &= ~CFOLD;
			break;
		case 'O':
			oflags &= ~CFOLD;
			aflags |= CFOLD;
			break;
		case 'v':
			oflags |= CFVERBOSE;
			break;
		}
		argp++;
	}
}

char *init (argc,argv)
int argc;
char **argv;
{
	char buf[STRINGLENGTH],*p;
	char username[STRINGLENGTH];
	register char *supfname,*q,*arg;
	register COLLECTION *c,*lastC;
	register FILE *f;
	register int bogus;
	register struct passwd *pw;
	register TREE *t;
	TREE *collT;			/* collections we are interested in */
	long timenow;			/* startup time */
	int checkcoll ();
	int oflags,aflags;
	int cwant;
	int (*oldsigsys)();
	char *fmttime();

	sysflag = FALSE;		/* not system upgrade */
	timeflag = FALSE;		/* don't print times */
#if	CMUCS
	rpauseflag = FALSE;		/* don't disable resource pausing */
#endif	CMUCS
	xpatchflag = FALSE;		/* not normally crosspatched */
	scmdebug = 0;			/* level zero, no SCM debugging */
	portdebug = FALSE;		/* no debugging ports */

	collT = NULL;
	oflags = aflags = 0;
	while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
		doswitch (&argv[1][1],&collT,&oflags,&aflags);
		--argc;
		argv++;
	}
	if (argc == 1 && !sysflag)
		logquit (1,"Need either -s or supfile");
#if	CMUCS
	oldsigsys = signal (SIGSYS,SIG_IGN);
	if (!rpauseflag && syscall (-5,ENOSPC,RPAUSE_ALL,RPAUSE_DISABLE) < 0)
		rpauseflag = TRUE;
	(void) signal (SIGSYS,oldsigsys);
#endif	CMUCS
	if (sysflag)
		supfname = sprintf (buf,
				    timeflag?FILESUPTDEFAULT:FILESUPDEFAULT,
				    DEFDIR);
	else {
		supfname = argv[1];
		if (strcmp (supfname,"-") == 0)
			supfname = "";
		--argc;
		argv++;
	}
	cwant = argc > 1;
	while (argc > 1) {
		t = Tinsert (&collT,argv[1],TRUE);
		t->Twant = TRUE;
		--argc;
		argv++;
	}
	if ((p = getlogin()) ||
	    ((pw = getpwuid ((int)getuid())) && (p = pw->pw_name)))
		(void) strcpy (username,p);
	else
		*username = '\0';
	if (*supfname) {
		f = fopen (supfname,"r");
		if (f == NULL)
			logquit (1,"Can't open supfile %s",supfname);
	} else
		f = stdin;
	firstC = NULL;
	lastC = NULL;
	bogus = FALSE;
	while (p = fgets (buf,STRINGLENGTH,f)) {
		q = index (p,'\n');
		if (q)  *q = '\0';
		if (index ("#;:",*p))  continue;
		arg = nxtarg (&p," \t");
		if (*arg == '\0') {
			logerr ("Missing collection name in supfile");
			bogus = TRUE;
			continue;
		}
		if (cwant) {
			register TREE *t;
			if ((t = Tsearch (collT,arg)) == NULL)
				continue;
			t->Tcount++;
		}
		c = (COLLECTION *) malloc (sizeof(COLLECTION));
		if (firstC == NULL)  firstC = c;
		if (lastC != NULL) lastC->Cnext = c;
		lastC = c;
		if (parsecoll(c,arg,p) < 0) {
			bogus = TRUE;
			continue;
		}
		c->Cflags |= oflags;
		c->Cflags &= ~aflags;
		if (t = Tsearch (collT,c->Cname)) {
			c->Cflags |= t->Toflags;
			c->Cflags &= ~t->Taflags;
		}
		if ((c->Cflags&CFMAIL) && c->Cnotify == NULL) {
			if (*username == '\0')
				logerr ("User unknown, notification disabled");
			else
				c->Cnotify = salloc (username);
		}
		if (c->Cbase == NULL) {
			(void) sprintf (buf,FILEBASEDEFAULT,c->Cname);
			c->Cbase = salloc (buf);
		}
	}
	if (bogus)  logquit (1,"Aborted due to supfile errors");
	if (f != stdin)  (void) fclose (f);
	if (cwant)  (void) Tprocess (collT,checkcoll);
	Tfree (&collT);
	if (firstC == NULL)  logquit (1,"No collections to upgrade");
	timenow = time ((long *)NULL);
	if (*supfname == '\0')
		p = "standard input";
	else if (sysflag)
		p = "system software";
	else
		p = sprintf (buf,"file %s",supfname);
	loginfo ("SUP %d.%d (%s) for %s at %s",PROTOVERSION,PGMVERSION,
		scmversion,p,fmttime (timenow));
	return (salloc (supfname));
}

checkcoll (t)
register TREE *t;
{
	if (!t->Twant)  return (SCMOK);
	if (t->Tcount == 0)
		logerr ("Collection %s not found",t->Tname);
	if (t->Tcount > 1)
		logerr ("Collection %s found more than once",t->Tname);
	return (SCMOK);
}
