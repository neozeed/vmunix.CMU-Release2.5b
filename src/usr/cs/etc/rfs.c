/*
 **********************************************************************
 * HISTORY
 * 26-Aug-89  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Changed include of sys/rfs.h to rfs/rfs.h.
 *
 * $Log:	rfs.c,v $
 * Revision 1.5  89/06/17  16:59:48  gm0w
 * 	Changed to use getgid instead of getaid for account to use.
 * 	[89/06/17            gm0w]
 * 
 * 31-May-87  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added command line switches for user, group, account and identify.
 *
 * 09-Apr-87  Doug Philips (dwp) at Carnegie-Mellon University
 *	Lets be more careful about chasing NULL pointers.
 *
 **********************************************************************
 */

#include <sys/types.h>
#include <sys/time.h>
#include <rfs/rfs.h>

#include <pwd.h>
#include <grp.h>
#include <c.h>



/*
 *  Global data
 */

char *progname;			/* our program name from argv[0] */
char *CONTROL = "/../.CONTROL";
int   fd;
char *user = "";
char *group = "";
char *account = "";
char *identify = "";



/*
 *  Forward declarations
 */
char **parseoptions();

extern uid_t getuid();
extern gid_t getgid();
extern char *getpass();



/*
 *  main program
 *
 *  Save program name and parse options which will set the accounting defaults.
 *
 *  If no errors are encountered, execute the remaining arguments (or a shell
 *  as specified in the environment if none remain).
 */

/*ARGSUSED*/
main(argc, argv)
    int argc;
    char *argv[];
{
    extern char *getenv();
    char buff[80];
    struct passwd *pwd;
    struct group *grp;
    char *p;

    progname = *argv++;
    argv = parseoptions(argv);

    fd = open(CONTROL, 2);
    if (fd < 0)
	quit(-1, "%s: %s: %s\n", progname, CONTROL, errmsg(-1));

    pwd = getpwuid((int)getuid());
    p = user;
    strarg(&p, " \t", "User?", pwd ? pwd->pw_name : "", buff);
    set((rfsRW_t)RFSRW_USER, buff);

    grp = getgrgid((int)getgid());
    p = group;
    strarg(&p, " \t","Group?", grp ? grp->gr_name : "", buff);
    set((rfsRW_t)RFSRW_GROUP, buff);

    grp = getgrgid((int)getgid());
    p = account;
    strarg(&p, " \t", "Account?", grp ? grp->gr_name : buff, buff);
    set((rfsRW_t)RFSRW_ACCOUNT, buff);

    set((rfsRW_t)RFSRW_PASSWORD, getpass("Password:"));

    p = identify;
    if (boolarg(&p, " \t","Identify?", 0))
    {
	if (ioctl(fd, RFSIOCIDENTIFY, (char *)0) < 0)
	    quit(-1, "ioctl(IDENTIFY): %s\n", errmsg(-1));
    }

    if (argv[0] == 0)
    {
	extern char *getenv();

	argv--;
	argv[0] = getenv("SHELL");
	if (argv[0] == 0)
	    quit(-1, "%s: no SHELL variable\n", progname);
    }

    execvp(argv[0], &argv[0]);
    quit(-1, "%s: %s: %s\n", progname, argv[0], errmsg(-1));
}



/*
 *  set - set RFS control default
 *
 *  mode   = mode type
 *  string = value to record for type
 */

set(mode, string)
    rfsRW_t mode;
    char *string;
{
    int j;

    if (ioctl(fd, RFSIOCSETRW, (char *)&mode) < 0)
	quit(-1, "ioctl(SETRW=%d): %s\n",  mode, errmsg(-1));
    j = write(fd, string, strlen(string));
    if (j < 0)
	quit(-1, "write(%d): %s\n", mode, errmsg(-1));
}



/*
 *  Option name table
 */
char *opt_names[] =
{
    "-user",
    "-group",
    "-account",
    "-identify",
    0
};

/*
 *  Option types (these MUST match the order in the above table)
 */
enum opt_types
{
    OPT_UNKNOWN=(-2),
    OPT_AMBIGUOUS,
    OPT_USER,
    OPT_GROUP,
    OPT_ACCOUNT,
    OPT_IDENTIFY,
};



/*
 *  argoption - check for argument to option
 *
 *  argv = argument vector
 *
 *  Return:  the remainder of the argument vector (beginning at the
 *  next argument) if it exists, otherwise exit.
 */

char **
argoption(argv)
   char **argv;
{
    if (argv[1] == 0)
	quit(-3, "%s: missing argument to %s\n", progname, *argv);
    return(++argv);
}



/*
 *  parseoptions - parse options from the argument vector
 *
 *  argv = argument vector
 *
 *  Scan the argument vector until either the end of the list or an argument
 *  which does not begin with the switch character is encountered.  Process
 *  each option appropriately as it is encountered.
 *
 *  Return:  the remainder of the argument vector (at the point option
 *  processing was terminated)
 */

char **
parseoptions(argv)
    char **argv;
{
    for (; *argv; argv++)
    {
	if (*argv[0] != '-')
	    break;
	switch (stablk(*argv, opt_names, TRUE))
	{
	    caseE(OPT_AMBIGUOUS):
	    {
		quit(-2, "%s: unknown option %s\n", progname, *argv);
		break;
	    }
	    caseE(OPT_UNKNOWN):
	    {
		quit(-2, "%s: ambiguous option %s\n", progname, *argv);
		break;
	    }
	    caseE(OPT_USER):
	    {
		argv = argoption(argv);
		user = *argv;
		break;
	    }
	    caseE(OPT_GROUP):
	    {
		argv = argoption(argv);
		group = *argv;
		break;
	    }
	    caseE(OPT_ACCOUNT):
	    {
		argv = argoption(argv);
		account = *argv;
		break;
	    }
	    caseE(OPT_IDENTIFY): 
	    {
		argv = argoption(argv);
		identify = *argv;
		break;
	    }
	}
    }
    return(argv);
}
