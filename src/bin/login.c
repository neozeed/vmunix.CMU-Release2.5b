/*
 **********************************************************************
 * HISTORY
 *  7-Dec-89  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Added pwd argument to call to ruserok in doremotelogin. The
 *	included version of ruserok takes 5 arguments unlike the library
 *	version.
 *
 *  9-Oct-89  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Commented out definition of AFS, so that this will build a
 *	version of login that does not do AFS authentication.
 *	Note: since a non-default signal handler (timedout) is set
 *	for alarm, oklogin will skip the AFS authentication.
 *
 * $Log:	login.c,v $
 * Revision 1.16  89/09/11  15:51:27  mja
 * 	Relax the controlling terminal device match requirement when
 * 	the table(U_TTYD) call is not supported.
 * 	[89/04/21            mja]
 * 
 * Revision 1.15  89/05/13  18:21:29  gm0w
 * 	Always preserve TERM in the environment if set.  Don't clear
 * 	window size if -f is given.
 * 	[89/05/01            gm0w]
 * 
 * Revision 1.14  89/02/25  12:31:50  gm0w
 * 	Updated to Tahoe release.  Made new -f mode more "captive".
 * 	[89/02/25            gm0w]
 * 
 * Revision 1.13  89/01/24  21:53:45  gm0w
 * 	Minor changes to printing of AFS errors.  Added :noafs suffix
 * 	for login name to disable AFS authentication.
 * 	[89/01/24            gm0w]
 * 
 * Revision 1.12  89/01/22  12:33:29  gm0w
 * 	Added code to print AFS failure messages.
 * 	[89/01/22            gm0w]
 * 
 * 	Removed AFS authentication code.  This now resides in oklogin.
 * 	This corresponds to the version of 05-Aug-87 (rev 1.6) with
 * 	the 30-Jun-88 and 16-Aug-88 lstat bugfixes.
 * 	[88/12/13            gm0w]
 * 
 * 16-Aug-88  David VanRyzin (vanryzin) at Carnegie-Mellon University
 *	Used lstat instead of stat when determining if plan
 *	file exists.  This allows users to point their plan
 *	files into AFS and not worry about login removing the
 *	link if AFS is down.  Also change to invoke the AUTH
 *	program (log) with the "-pipe" switch.
 *
 * 30-Jun-88  Joshua Raiff (raiff) at Carnegie-Mellon University
 *	Changed test for "> 0" to "!= NULL" to to fix compilation under HC for
 * 	 the IBMRT
 *
 * 01-Feb-88  Imre Magyar (magyar) at Carnegie-Mellon University
 *	Login now tries to automatically authenticate a user to VICE,
 *	by running the "log" program.
 *
 * 05-Aug-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to clear loggedin state before exiting.
 *
 * 20-Jul-87  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed chkattach() to pass a pointer to an integer instead of
 *	a pointer to a short to the TIOCCHECK ioctl; flushed unused
 *	variable in reattach(); corrected getchar() type assignment
 *	problem in getname().
 *
 * 14-Jul-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed old FE ioctl's.  Added TIOCLOGINDEV.
 *
 * 01-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Get TERM from environment for login -p -h host.
 *
 * 01-Jun-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to break links to lastlog file.
 *
 * 31-May-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Finally changed /usr/cmu to /usr/cs in all default paths.
 *
 * 28-May-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fixed password prompting code for rlogin connections.
 *
 * 22-Mar-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Restored missing "major()" call around the FE device number check to
 *	fix (we hope for the last time) the TEMPORARY ioctl compatibility
 *	problem.
 *
 * 21-Mar-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	One more try... Only do OLD FE ioctl's if on an OLD FE.
 *
 * 21-Mar-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed problem with TEMPORARY code that expected FEIOCGETT
 *	to fail on a non-FE line - the chat server also implements
 *	this one!
 *
 * 20-Mar-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added TEMPORARY code to do old FE ioctl's because the kernel
 *	doesn't understand the new ones yet.
 *
 * 19-Mar-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed default stty characters to CMU 4.1 defaults.
 *
 * 05-Mar-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Recoded bad pty attach code to give option to not attach.
 *
 * 25-Feb-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Merged CMU CS changes with 4.3 version of login.
 *
 * 27-Oct-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Merged CMU CS changes with 4.2 version of login.
 *
 **********************************************************************
 */
/*
 * Copyright (c) 1980,1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)login.c	5.20 (Berkeley) 10/1/87";
#endif not lint

/*
 * login [ name ]
 * login -r hostname	(for rlogind)
 * login -h hostname	(for telnetd, etc.)
 * login -f name	(for pre-authenticated login: datakit, xterm, etc.)
 */

#include <sys/param.h>
#include <sys/quota.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>

#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <lastlog.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <grp.h>
#if	CMUCS
#include <c.h>
#include <libc.h>
#include <acc.h>
#include <sys/table.h>
#include <sys/ttyloc.h>
#include <access.h>
/* #define	AFS	1 */
#ifdef	AFS
#include <sys/wait.h>
#endif	AFS
#endif	CMUCS

#define TTYGRPNAME	"tty"		/* name of group to own ttys */
#define TTYGID(gid)	tty_gid(gid)	/* gid that owns all ttys */

#define	SCMPN(a, b)	strncmp(a, b, sizeof(a))
#define	SCPYN(a, b)	strncpy(a, b, sizeof(a))

#define NMAX	sizeof(utmp.ut_name)
#define HMAX	sizeof(utmp.ut_host)

#if	CMUCS
#define LMAX	sizeof(utmp.ut_line)
#define isprogram(p)	((p)->pw_passwd[0] == '\0' && (p)->pw_uid < 10)
#define	NARGP		15		/* number of arguments to command */
#define	NOTVALID	-2		/* special non-false value */
#define	OPRGID		13		/* group ID of Operator group */
#define	DEFSHELL	"sh"		/* default shell */
#define	BINSHELL	"/bin/sh"	/* last resort shell */
#define	CTY		"/dev/console"	/* console terminal */
#define	PATH	":/usr/cs/bin:/usr/ucb:/bin:/usr/bin"
#define	CPATH	":/usr/cs/include:/usr/include"
#define	LPATH	":/usr/cs/lib:/lib:/usr/lib"
#define	MPATH	":/usr/cs/man:/usr/man"
#define	EPATH	":/usr/cs/maclib"
#else	CMUCS
#define	FALSE	0
#define	TRUE	-1
#endif	CMUCS

char	nolog[] =	"/etc/nologin";
char	qlog[]  =	".hushlogin";
char	maildir[30] =	"/usr/spool/mail/";
char	lastlog[] =	"/usr/adm/lastlog";
#if	CMUCS
char	lasttlc[] =	"/usr/adm/lasttlc";
#endif	CMUCS
struct	passwd nouser = {"", "nope", -1, -1, -1, "", "", "", "" };
struct	sgttyb ttyb;
struct	utmp utmp;
#if	CMUCS
char	minusnam[128] = "-";
#else	CMUCS
char	minusnam[16] = "-";
#endif	CMUCS
char	*envinit[1];			/* now set by setenv calls */
/*
 * This bounds the time given to login.  We initialize it here
 * so it can be patched on machines where it's too small.
 */
#if	CMUCS
/* a minute still seems to be sufficient */
int	timeout = 60;
#else	CMUCS
int	timeout = 300;
#endif	CMUCS

char	term[64];

#if	CMUCS
struct	account *acc;
struct	group *grp;
int	timeleft;
int	code;
char	*namep;
char	*llinep;
char	lline[1024];
char	*group;
char	*account;
int	*groups;
char	*fgetpass();
char	*index();
char	*execsh();
char	*parse();
char	*ttyloc();
char	*okpassword();
#endif	CMUCS
struct	passwd *pwd;
char	*strcat(), *rindex(), *index();
int	timedout();
char	*ttyname();
char	*crypt();
char	*getpass();
char	*stypeof();
extern	int errno;
#if	CMUCS
#ifdef	AFS
int	noafs;
extern union wait afs_log_status; /* exit status or errno value */
extern int afs_log_errcode;
#define AFSERR_NOERR	0	/* no setup errors, see status */
#define AFSERR_NOPASS	1	/* no password, no authentication */
#define AFSERR_SIGUSED	2	/* signals in use, no authentication */
#define AFSERR_PIPE	3	/* pipe syscall, no authentication */
#define AFSERR_FORK	4	/* fork syscall, no authentication */
#define AFSERR_SETPAG	5	/* setpag syscall, no authentication */
#define AFSERR_TIMEOUT	6	/* timed out log, possible authentication */
#endif	AFS
#endif	CMUCS

struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

struct winsize win = { 0, 0, 0, 0 };

int	rflag;
int	usererr = -1;
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	name[NMAX+1];
#if	CMUCS
/* we use ip addresses, not host names */
#else	CMUCS
char	me[MAXHOSTNAMELEN];
#endif	CMUCS
char	*rhost;

main(argc, argv)
	char *argv[];
{
	extern	char **environ;
#if	CMUCS
	/* needs to be global */
#else	CMUCS
	register char *namep;
#endif	CMUCS
	int pflag = 0, hflag = 0, fflag = 0, t, f, c;
	int invalid, quietlog;
	FILE *nlfd;
	char *ttyn, *tty;
	int ldisc = 0, zero = 0, i;
#if	CMUCS
	char *argp[NARGP+1];
	int narg;
	int cty = 0;
	struct stat st;
	struct ttyloc tlc;
	dev_t u_ttyd;
#else	CMUCS
	char *p, *domain, *index();
#endif	CMUCS

#if	CMUCS
	/* if we are not invoked by root, parent must be init */
	if (getuid() && getppid() != 1)
		exit(-1);
	namep = NULL;
#endif	CMUCS
	signal(SIGALRM, timedout);
#if	CMUCS
	timeleft = timeout;
#else	CMUCS
	alarm(timeout);
#endif	CMUCS
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
#if	CMUCS
	/* leave at high priority for a while longer */
#else	CMUCS
	setpriority(PRIO_PROCESS, 0, 0);
#endif	CMUCS
	quota(Q_SETUID, 0, 0, 0);
	/*
	 * -p is used by getty to tell login not to destroy the environment
	 * -r is used by rlogind to cause the autologin protocol;
 	 * -f is used to skip a second login authentication 
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in utmp and wtmp
	 */
#if	CMUCS
	/* we don't record names, just ip addresses */
#else	CMUCS
	(void) gethostname(me, sizeof(me));
	domain = index(me, '.');
#endif	CMUCS
	while (argc > 1) {
		if (strcmp(argv[1], "-r") == 0) {
			if (rflag || hflag || fflag) {
				printf("Other options not allowed with -r\n");
				exit(1);
			}
			if (argv[2] == 0)
				exit(1);
			rflag = 1;
			usererr = doremotelogin(argv[2]);
#if	CMUCS
			/* we don't record names, just ip addresses */
#else	CMUCS
			if ((p = index(argv[2], '.')) && strcmp(p, domain) == 0)
				*p = 0;
#endif	CMUCS
			SCPYN(utmp.ut_host, argv[2]);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-h") == 0) {
			if (getuid() == 0) {
				if (rflag || hflag) {
				    printf("Only one of -r and -h allowed\n");
				    exit(1);
				}
				hflag = 1;
#if	CMUCS
				/* we don't record names, just ip addresses */
#else	CMUCS
				if ((p = index(argv[2], '.')) &&
				    strcmp(p, domain) == 0)
					*p = 0;
#endif	CMUCS
				SCPYN(utmp.ut_host, argv[2]);
			}
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-f") == 0 && argc > 2) {
			if (rflag) {
				printf("Only one of -r and -f allowed\n");
				exit(1);
			}
			fflag = 1;
#if	CMUCS
			namep = argv[2];
			SCPYN(utmp.ut_name, argv[2]);
#else	CMUCS
			SCPYN(utmp.ut_name, argv[2]);
#endif	CMUCS
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-p") == 0) {
			argc--;
			argv++;
			pflag = 1;
			continue;
		}
		break;
	}
#if	CMUCS
	/*
	 * 
	 */
	ioctl(0, TIOCLOGINDEV, (char *)0);
	zero = 1;
	ioctl(0, TIOCCLOG, &zero);
	zero = 0;
#endif	CMUCS
	ioctl(0, TIOCLSET, &zero);
	ioctl(0, TIOCNXCL, 0);
	ioctl(0, FIONBIO, &zero);
	ioctl(0, FIOASYNC, &zero);
	ioctl(0, TIOCGETP, &ttyb);
	/*
	 * If talking to an rlogin process,
	 * propagate the terminal type and
	 * baud rate across the network.
	 */
	if (rflag)
		doremoteterm(term, &ttyb);
#if	CMUCS
	if (term[0] == '\0' && (ttyn = getenv("TERM")) != NULL)
		strcpy(term, ttyn);
#endif	CMUCS
	ttyb.sg_erase = CERASE;
	ttyb.sg_kill = CKILL;
	ioctl(0, TIOCSLTC, &ltc);
	ioctl(0, TIOCSETC, &tc);
	ioctl(0, TIOCSETP, &ttyb);
	for (t = getdtablesize(); t > 2; t--)
		close(t);
	ttyn = ttyname(0);
	if (ttyn == (char *)0 || *ttyn == '\0')
#if	CMUCS
	/*
	 * We must have a terminal to get terminal location.
	 */
		logout(-2);
	if (fstat(0, &st) < 0)
		logout(-2);
	errno = 0;
	if (table(TBL_U_TTYD, 0, &u_ttyd, 1, sizeof(u_ttyd)) != 1 &&
	    errno != EINVAL &&
	    st.st_rdev != u_ttyd)
		logout(-2);
        if (table(TBL_TTYLOC, st.st_rdev, &tlc, 1, sizeof(tlc)) < 0) {
		tlc.tlc_hostid = TLC_UNKHOST;
		tlc.tlc_ttyid = TLC_UNKTTY;
	}
	cty = (strcmp(ttyn, CTY) == 0);
#else	CMUCS
		ttyn = "/dev/tty??";
#endif	CMUCS
	tty = rindex(ttyn, '/');
	if (tty == NULL)
		tty = ttyn;
	else
		tty++;
	openlog("login", LOG_ODELAY, LOG_AUTH);
	t = 0;
	invalid = FALSE;
#if	CMUCS
	/*
	 * Name specified, take it.
	 */
	if (argc > 1) {
		if (namep != NULL) {
			if (argv[1][0] != '-') {
				if (strcmp(namep, argv[1]) != 0) {
					puts("login names mismatch.");
					logout(1);
				}
				argc--;
				argv++;
			}
		} else {
			namep = argv[1];
			argc--;
			argv++;
		}
	}
	if (namep != NULL) {
		namep = strncpy(lline, namep, sizeofS(lline));
		llinep = parse(&namep);
		if (namep == NULL && fflag) {
			puts("null login name.");
			logout(1);
		} else if (namep != NULL)
			SCPYN(utmp.ut_name, namep);
	} else {
		SCPYN(utmp.ut_name, "");
		strncpy(lline, "", sizeofS(lline));
		llinep = NULL;
	}
#endif	CMUCS
	do {
		ldisc = 0;
		ioctl(0, TIOCSETD, &ldisc);
#if	CMUCS
		if (invalid) {
			if (fflag)
				logout(1);
			argc = 0;
			SCPYN(utmp.ut_name, "");
			strncpy(lline, "", sizeofS(lline));
			namep = NULL;
			llinep = NULL;
		}
#else	CMUCS
		if (fflag == 0)
			SCPYN(utmp.ut_name, "");
		/*
		 * Name specified, take it.
		 */
		if (argc > 1) {
			SCPYN(utmp.ut_name, argv[1]);
			argc = 0;
		}
#endif	CMUCS
		/*
		 * If remote login take given name,
		 * otherwise prompt user for something.
		 */
		if (rflag && !invalid)
			SCPYN(utmp.ut_name, lusername);
		else {
			getloginname(&utmp);
			if (utmp.ut_name[0] == '-') {
				puts("login names may not start with '-'.");
				invalid = TRUE;
				continue;
			}
		}
		invalid = FALSE;
#if	CMUCS
		argp[0] = pwd->pw_shell;
		argp[1] = parse(&argp[0]);
		if ((pwd->pw_shell = argp[0]) == NULL)
			argp[0] = pwd->pw_shell = DEFSHELL;
		narg = 1;
		if (argp[1] != NULL)
			for (narg++; narg < NARGP; narg++)
				if ((argp[narg] = parse(&argp[narg-1])) == NULL)
					break;
		if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
			namep = pwd->pw_shell;
		else
			namep++;
		if (!strcmp(namep, "csh")) {
#else	CMUCS
		if (!strcmp(pwd->pw_shell, "/bin/csh")) {
#endif	CMUCS
			ldisc = NTTYDISC;
			ioctl(0, TIOCSETD, &ldisc);
		}
		if (fflag) {
			int uid = getuid();

			if (uid != 0 && uid != pwd->pw_uid)
				fflag = 0;
			/*
			 * Disallow automatic login for root.
			 */
			if (pwd->pw_uid == 0)
				fflag = 0;
#if	CMUCS
			if (!fflag) {
				invalid = TRUE;
				continue;
			}
#endif	CMUCS
		}
#if	CMUCS
		if (pwd == &nouser) {
			code = ACCESS_CODE_NOUSER;
			invalid = TRUE;
		} else {
			i = okaccess(pwd->pw_name, ACCESS_TYPE_LOGIN,
				     pwd->pw_uid, st.st_rdev, tlc);
			if (i == 1)
				code = ACCESS_CODE_OK;
			else {
				code = ACCESS_CODE_DENIED;
				invalid = TRUE;
			}
		}
#endif	CMUCS
#if	CMUCS
		if (invalid ||
		    (usererr == -1 && fflag == 0 && *pwd->pw_passwd != '\0')) {
			if (!cty) setpriority(PRIO_PROCESS, 0, -4);
			contalarm();
			namep = fgetpass("Password:", stdin);
			stopalarm();
			if (!cty) setpriority(PRIO_PROCESS, 0, 0);
		} else
			namep = NULL;
#else	CMUCS
		/*
		 * If no remote login authentication and
		 * a password exists for this user, prompt
		 * for one and verify it.
		 */
		if (usererr == -1 && fflag == 0 && *pwd->pw_passwd != '\0') {
			char *pp;

			setpriority(PRIO_PROCESS, 0, -4);
			pp = getpass("Password:");
			namep = crypt(pp, pwd->pw_passwd);
			setpriority(PRIO_PROCESS, 0, 0);
			if (strcmp(namep, pwd->pw_passwd))
				invalid = TRUE;
		}
#endif	CMUCS
#if	CMUCS
		ttyb.sg_flags |= ECHO;	/* restore echo */
		ioctl(0, TIOCSETN, &ttyb);
		/* validate login attempt */
#ifdef	AFS
		afs_log_errcode = AFSERR_NOPASS;
		if (!noafs)
			signal(SIGALRM, SIG_DFL);
#endif	AFS
	        invalid = notvalid(namep, group, account) || invalid;
#ifdef	AFS
		signal(SIGALRM, timedout);
#endif	AFS
#endif	CMUCS
#if	CMUCS
		/* logins are disabled except for root and operator */
		if (!invalid && (nlfd = fopen(nolog, "r")) != NULL) {
			contalarm();
			while ((c = getc(nlfd)) != EOF)
				putchar(c);
			fflush(stdout);
			stopalarm();
		        if (pwd->pw_uid != 0 && pwd->pw_gid != OPRGID) {
				sleep(5);
				logout(0);
			}
			fclose(nlfd);
		}
#else	CMUCS
		/*
		 * If user not super-user, check for logins disabled.
		 */
		if (pwd->pw_uid != 0 && (nlfd = fopen(nolog, "r")) > 0) {
			while ((c = getc(nlfd)) != EOF)
				putchar(c);
			fflush(stdout);
			sleep(5);
			exit(0);
		}
#endif	CMUCS
#if	CMUCS
		/* don't need to announce root failures to the console */
#else	CMUCS
		/*
		 * If valid so far and root is logging in,
		 * see if root logins on this terminal are permitted.
		 */
		if (!invalid && pwd->pw_uid == 0 && !rootterm(tty)) {
			if (utmp.ut_host[0])
				syslog(LOG_CRIT,
				    "ROOT LOGIN REFUSED ON %s FROM %.*s",
				    tty, HMAX, utmp.ut_host);
			else
				syslog(LOG_CRIT,
				    "ROOT LOGIN REFUSED ON %s", tty);
			invalid = TRUE;
		}
#endif	CMUCS
		if (invalid) {
#if	CMUCS
			/* done in notvalid */
#else	CMUCS
			printf("Login incorrect\n");
#endif	CMUCS
			if (++t >= 5) {
				if (utmp.ut_host[0])
					syslog(LOG_ERR,
			    "REPEATED LOGIN FAILURES ON %s FROM %.*s, %.*s",
					    tty, HMAX, utmp.ut_host,
					    NMAX, utmp.ut_name);
				else
					syslog(LOG_ERR,
				    "REPEATED LOGIN FAILURES ON %s, %.*s",
						tty, NMAX, utmp.ut_name);
				ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
				close(0), close(1), close(2);
				sleep(10);
#if	CMUCS
				logout(1);
#else	CMUCS
				exit(1);
#endif	CMUCS
			}
		}
#if	CMUCS
		/* done above */
#else	CMUCS
		if (*pwd->pw_shell == '\0')
			pwd->pw_shell = "/bin/sh";
#endif	CMUCS
#if	CMUCS
#ifdef	AFS
		if (!invalid && (noafs || chdir(pwd->pw_dir) < 0)) {
#else	AFS
		if (!invalid && chdir(pwd->pw_dir) < 0) {
#endif	AFS
#else	CMUCS
		if (chdir(pwd->pw_dir) < 0 && !invalid ) {
#endif	CMUCS
			if (chdir("/") < 0) {
				printf("No directory!\n");
				invalid = TRUE;
#if	CMUCS
				code = ACCESS_CODE_NODIR;
#endif	CMUCS
			} else {
#if	CMUCS
#ifdef	AFS
				if (noafs)
					printf("Logging in with home=/\n");
				else
#endif	AFS
#endif	CMUCS
				printf("No directory! %s\n",
				   "Logging in with home=/");
				pwd->pw_dir = "/";
			}
		}
#if	CMUCS
		logaccess(utmp.ut_name, ACCESS_TYPE_LOGIN, code, 0,
			st.st_rdev, tlc);
#endif	CMUCS
		/*
		 * Remote login invalid must have been because
		 * of a restriction of some sort, no extra chances.
		 */
		if (!usererr && invalid)
#if	CMUCS
			logout(1);
#else	CMUCS
			exit(1);
#endif	CMUCS
	} while (invalid);
#if	CMUCS
	if (cty && pwd->pw_uid != 0 && pwd->pw_gid != OPRGID)
		setpriority(PRIO_PROCESS, 0, 0);
	else if (cty)
		printf("[ WARNING: Logged-in on console at highest (-20) priority ]\n");
#else	CMUCS
/* committed to login turn off timeout */
	alarm(0);
#endif	CMUCS

	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
		if (errno == EUSERS)
			printf("%s.\n%s.\n",
			   "Too many users logged on already",
			   "Try again later");
		else if (errno == EPROCLIM)
			printf("You have too many processes running.\n");
		else
			perror("quota (Q_SETUID)");
		sleep(5);
#if	CMUCS
		logout(0);
#else	CMUCS
		exit(0);
#endif	CMUCS
	}
	time(&utmp.ut_time);
	t = ttyslot();
#if	CMUCS
	SCPYN(utmp.ut_line, tty);
	if (t > 0 && (f = open("/etc/utmp", O_RDWR)) >= 0) {
		alarm(timeout);
		reattach(pwd->pw_name, f);
		alarm(0);
#else	CMUCS
	if (t > 0 && (f = open("/etc/utmp", O_WRONLY)) >= 0) {
#endif	CMUCS
		lseek(f, (long)(t*sizeof(utmp)), 0);
#if	CMUCS
		/* done above */
#else	CMUCS
		SCPYN(utmp.ut_line, tty);
#endif	CMUCS
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND)) >= 0) {
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	quietlog = access(qlog, F_OK) == 0;
#if	CMUCS
	if (!quietlog) quietlog = isprogram(pwd);
#endif	CMUCS
	if ((f = open(lastlog, O_RDWR)) >= 0) {
		struct lastlog ll;
#if	CMUCS
		struct ttyloc lt;
		int ft;
#endif	CMUCS

		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
#if	CMUCS
		if (read(f, (char *) &ll, sizeof(ll)) != sizeof(ll))
			ll.ll_time = 0;
		if ((ft = open(lasttlc, O_RDWR)) >= 0) {
			lseek(ft, (long)pwd->pw_uid * sizeof(lt), 0);
			if (read(ft, (char *) &lt, sizeof(lt)) != sizeof(lt)) {
				lt.tlc_hostid = 0;
				lt.tlc_ttyid = 0;
			}
		}
		if (!quietlog) {
			printf("Last login: ");
			printll(&ll);
			if (ft >= 0)
				printtlc(&lt);
			printf("\n");
		}
#else	CMUCS
		if (read(f, (char *) &ll, sizeof ll) == sizeof ll &&
		    ll.ll_time != 0 && !quietlog) {
			printf("Last login: %.*s ",
			    24-5, (char *)ctime(&ll.ll_time));
			if (*ll.ll_host != '\0')
				printf("from %.*s\n",
				    sizeof (ll.ll_host), ll.ll_host);
			else
				printf("on %.*s\n",
				    sizeof (ll.ll_line), ll.ll_line);
		}
#endif	CMUCS
		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
#if	CMUCS
		ll.ll_time = utmp.ut_time;
#else	CMUCS
		time(&ll.ll_time);
#endif	CMUCS
		SCPYN(ll.ll_line, tty);
		SCPYN(ll.ll_host, utmp.ut_host);
		write(f, (char *) &ll, sizeof ll);
		close(f);
#if	CMUCS
		if (ft >= 0) {
			lseek(ft, (long)pwd->pw_uid * sizeof(tlc), 0);
			write(ft, (char *) &tlc, sizeof(tlc));
			close(ft);
		}
		if (!quietlog) {
			printf("This login: ");
			printll(&ll);
			printtlc(&tlc);
			printf("\n");
		}
#endif	CMUCS
	}
	chown(ttyn, pwd->pw_uid, TTYGID(pwd->pw_gid));
#if	CMUCS
	if (!hflag && !rflag && !fflag)				/* XXX */
#else	CMUCS
	if (!hflag && !rflag)					/* XXX */
#endif	CMUCS
		ioctl(0, TIOCSWINSZ, &win);
	chmod(ttyn, 0620);
	setgid(pwd->pw_gid);
#if	CMUCS
	setgroups(*groups, groups+1);
#else	CMUCS
	strncpy(name, utmp.ut_name, NMAX);
	name[NMAX] = '\0';
	initgroups(name, pwd->pw_gid);
#endif	CMUCS
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
	setuid(pwd->pw_uid);
#if	CMUCS
#ifdef	AFS
	if (!noafs && !fflag && *pwd->pw_passwd != '\0')
#ifndef	allusersinafs
		(void) print_afs_status(0);
#else	/* allusersinafs */
		(void) print_afs_status();
#endif	/* allusersinafs */
#endif	AFS
#endif	CMUCS

	/* destroy environment unless user has asked to preserve it */
	if (!pflag)
		environ = envinit;
	setenv("HOME", pwd->pw_dir, 1);
#if	CMUCS
	pwd->pw_shell = execsh(pwd->pw_shell);
#endif	CMUCS
	setenv("SHELL", pwd->pw_shell, 1);
	if (term[0] == '\0')
		strncpy(term, stypeof(tty), sizeof(term));
	setenv("TERM", term, 0);
	setenv("USER", pwd->pw_name, 1);
#if	CMUCS
	setenv("PATH", PATH, 0);
	setenv("CPATH", CPATH, 0);
	setenv("LPATH", LPATH, 0);
	setenv("MPATH", MPATH, 0);
	setenv("EPATH", EPATH, 0);
#else	CMUCS
	setenv("PATH", ":/usr/ucb:/bin:/usr/bin", 0);
#endif	CMUCS

	if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;
	strcat(minusnam, namep);
#if	CMUCS
	/* leave this until we are sure it is already being done elsewhere */
	umask(022);
#else	CMUCS
	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
#endif	CMUCS
	if (pwd->pw_uid == 0)
		if (utmp.ut_host[0])
			syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %.*s",
			    tty, HMAX, utmp.ut_host);
		else
			syslog(LOG_NOTICE, "ROOT LOGIN %s", tty);
	if (!quietlog) {
		struct stat st;

		showmotd();
		strcat(maildir, pwd->pw_name);
		if (stat(maildir, &st) == 0 && st.st_size != 0)
			printf("You have %smail.\n",
				(st.st_mtime > st.st_atime) ? "new " : "");
	}
	signal(SIGALRM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
#if	CMUCS
	/* done below after mkplan */
#else	CMUCS
	signal(SIGINT, SIG_DFL);
#endif	CMUCS
	signal(SIGTSTP, SIG_IGN);
#if	CMUCS
	if (argc > 1) {
		for (i=1; narg < NARGP; narg++)
			if ((argp[narg] = argv[i++]) == NULL)
				break;
		argp[narg] = 0;
	} else if (argp[narg++] = llinep) {
		for (; narg < NARGP; narg++)
			if ((argp[narg] = parse(&argp[narg-1])) == NULL)
				break;
		argp[narg] = NULL;
	}
	if (!isshell(pwd)) {
		int sendhup = CDETHUP;

		ioctl(0, TIOCCSET, &sendhup);
	} else
		mkplan();
	if (isprogram(pwd)) {
		minusnam[0] = '+';
		alarm(120);
	}
	argp[0] = minusnam;
	/*
	 *  Normally, LOGIN will close any files it opens.  However, the
	 *  call on the ttyloc() routine will leave the terminal locations
	 *  file open so we have to close down all file descriptors again
	 *  here.  Ideally the routine ought to have provided for this some
	 *  way itself, but ...
	 */
	for (t = getdtablesize(); t > 2; t--)
		close(t);
	signal(SIGINT, SIG_DFL);
	execvp(pwd->pw_shell, argp);
#else	CMUCS
	execlp(pwd->pw_shell, minusnam, 0);
#endif	CMUCS
	perror(pwd->pw_shell);
	printf("No shell\n");
#if	CMUCS
	logout(0);
#else	CMUCS
	exit(0);
#endif	CMUCS
}

getloginname(up)
	register struct utmp *up;
{
#if	CMUCS
	/* use global name pointer */
	int c;		/* getchar() returns an 'int' */
	int i;
#else	CMUCS
	register char *namep;
	char c;
#endif	CMUCS

#if	CMUCS
	contalarm();
	while (namep == NULL) {
		llinep = lline;
		ttyb.sg_flags |= ECHO;
		ioctl(0, TIOCSETN, &ttyb);
		printf("login: ");
		while ((c = getchar()) != '\n') {
			if (llinep < &lline[sizeofS(lline)])
				*llinep++ = c;
			if (c == EOF)
				logout(0);
			if (c == ' ')
				c = '\0';
		}
		*llinep = '\0';
		namep = lline;
		llinep = parse(&namep);
	}
	stopalarm();
	ttyb.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETN, &ttyb);
#ifdef	AFS
	noafs = FALSE;
	/*
	 * look for trailing ":noafs" in login name
	 */
	if ((i = strlen(namep)) > 6 && strcmp(namep+i-6, ":noafs") == 0) {
		noafs = TRUE;
		namep[i-6] = '\0';
	}
#endif	AFS
	/*
	 *  Pick up any group, account specified after a comma in the
	 *  login name.  Treat a null group, account as if none were
	 *  specified.
	 */
	if ((account = group = index(namep, ',')) != NULL) {
		*group++ = '\0';
		if ((account = index(group, ',')) != NULL) {
			*account++ = '\0';
			if (*account == '\0')
				account = NULL;
		}
		if (*group == '\0')
			group = NULL;
	}
	SCPYN(up->ut_name, namep);
#else	CMUCS
	while (up->ut_name[0] == '\0') {
		namep = up->ut_name;
		printf("login: ");
		while ((c = getchar()) != '\n') {
			if (c == ' ')
				c = '_';
			if (c == EOF)
				exit(0);
			if (namep < up->ut_name+NMAX)
				*namep++ = c;
		}
	}
#endif	CMUCS
	strncpy(lusername, up->ut_name, NMAX);
	lusername[NMAX] = 0;
	if ((pwd = getpwnam(lusername)) == NULL)
		pwd = &nouser;
}

#if	CMUCS
logout(status)
{
	int zero = 0;

	/* mark us as no longer logged in */
	ioctl(0, TIOCCLOG, &zero);
	exit(status);
}
#endif	CMUCS

timedout()
{

	printf("Login timed out after %d seconds\n", timeout);
#if	CMUCS
	logout(0);
#else	CMUCS
	exit(0);
#endif	CMUCS
}

int	stopmotd;
catch()
{

	signal(SIGINT, SIG_IGN);
	stopmotd++;
}

#if	CMUCS
/* handled by okaccess */
#else	CMUCS
rootterm(tty)
	char *tty;
{
	register struct ttyent *t;

	if ((t = getttynam(tty)) != NULL) {
		if (t->ty_status & TTY_SECURE)
			return (1);
	}
	return (0);
}
#endif	CMUCS

showmotd()
{
	FILE *mf;
	register c;

	signal(SIGINT, catch);
	if ((mf = fopen("/etc/motd", "r")) != NULL) {
		while ((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
	}
	signal(SIGINT, SIG_IGN);
}

#undef	UNKNOWN
#if	CMUCS
#define UNKNOWN "unknown"
#else	CMUCS
#define UNKNOWN "su"
#endif	CMUCS

char *
stypeof(ttyid)
	char *ttyid;
{
	register struct ttyent *t;

	if (ttyid == NULL || (t = getttynam(ttyid)) == NULL)
		return (UNKNOWN);
	return (t->ty_type);
}

doremotelogin(host)
	char *host;
{
#if	CMUCS
	contalarm();
#endif	CMUCS
	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
#if	CMUCS
	stopalarm();
#endif	CMUCS
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		return(-1);
	}
	return(ruserok(host, (pwd->pw_uid == 0), rusername, lusername, pwd));
}

#if	CMUCS
char *
#endif	CMUCS
getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
#if	CMUCS
			logout(1);
#else	CMUCS
			exit(1);
#endif	CMUCS
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
#if	CMUCS
			logout(1);
#else	CMUCS
			exit(1);
#endif	CMUCS
		}
		*buf++ = c;
	} while (c != 0);
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term, tp)
	char *term;
	struct sgttyb *tp;
{
	register char *cp = index(term, '/'), **cpp;
	char *speed;

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
				tp->sg_ispeed = tp->sg_ospeed = cpp-speeds;
				break;
			}
	}
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}

tty_gid(default_gid)
	int default_gid;
{
	struct group *getgrnam(), *gr;
	int gid = default_gid;

	gr = getgrnam(TTYGRPNAME);
	if (gr != (struct group *) 0)
		gid = gr->gr_gid;

	endgrent();

	return (gid);
}

#if	CMUCS
stopalarm()
{
	timeleft = alarm(0);
}

contalarm()
{
	if (timeleft == 0)
		timedout();
	alarm(timeleft);
}

printll(ll)
register struct lastlog *ll;
{
	if (ll->ll_time == 0) {
		printf("information unavailable");
		return;
	}
	printf("%.*s ", 24-5, (char *)ctime(&ll->ll_time));
	if (*(ll->ll_host) != '\0')
		printf("from %.*s", sizeof(ll->ll_host), ll->ll_host);
	else
		printf("on %.*s", sizeof(ll->ll_line), ll->ll_line);
}

printtlc(tlc)
register struct ttyloc *tlc;
{
	register char *loc;

	if (tlc->tlc_hostid == 0 && tlc->tlc_ttyid == 0)
		printf(" (location unavailable)");
	else if ((loc = ttyloc(tlc->tlc_hostid, tlc->tlc_ttyid)) == NULL)
		printf(" (location %X,%X ERROR)",
			tlc->tlc_hostid, tlc->tlc_ttyid);
	else
		printf(" (%s)", loc);
}

/*
 *  Check if the specified program is really a shell (e.g. "sh" or "csh").
 */
char *validsh[] = { "", "sh", "csh", 0 };

isshell()
{
	extern char *rindex();
	register char *cp;
	register char **p;

	cp = rindex(pwd->pw_shell, '/');
	if (cp == NULL)
		cp = pwd->pw_shell;
	else
		cp++;
	for (p = validsh; *p; p++)
		if (strcmp(*p, cp) == 0)
			return(TRUE);
	return(FALSE);
}

isexec(sp)
char *sp;
{
	struct stat sb;

	return(access(sp, 1) || stat(sp, &sb) < 0 || (sb.st_mode&S_IFMT) != S_IFREG);
}

char *execsh(sp)
	char *sp;
{
	static char pathbuf[512];

	/* only check if not specified by absolute pathname */
	if (*sp != '/') {
		/* conservative measure */
		if (sizeof(PATH) + strlen(sp) >= sizeof(pathbuf))
			sp = DEFSHELL;
		if (searchp(PATH, sp, pathbuf, isexec) == 0)
			sp = pathbuf;
	}
	return(sp);
}

/*
 * str is a pointer to a string of the form "cmd arg".
 * places a null after 'cmd' and returns a pointer to 'arg'
 * returns NULL if arg is missing
 */
char *parse(str)
register char **str;
{
	register char *p = *str;

	while (*p && (*p == ' ' || *p == '\t'))
		p++;
	if (*p == '\0') {
		*str = NULL;
		return(NULL);
	}
	*str = p;
	while (*p && *p != ' ' && *p != '\t')
		p++;
	if (*p != '\0')
		*p++ = '\0';
	while (*p && (*p == ' ' || *p == '\t'))
		p++;
	if (*p == '\0')
		return (NULL);
	return (p);
}

/*
 *  reattach - attach user to disconnected PTY terminal
 *
 *  user = user's login name
 *  ufd  = file descriptor for /etc/utmp
 */
reattach(user, ufd)
char *user;
int ufd;
{
	int cnt, rdev;
	char *line;
	char ans[16];
	struct sgttyb sg;
	char *getnstr();

	cnt = chkattach(user, NULL, ufd, &line, &rdev);
	if (cnt == 0)
		return;
	printf(" ]\n");
	if (getnstr("Reattach ?", "yes", ans, sizeofS(ans), TRUE) == NULL)
		logout(0);
	if (strcmp(ans, "n") == 0 || strcmp(ans, "no") == 0)
		return;
	while (cnt > 1) {
		if (getnstr("Reattach to ?", line, ans, sizeofS(ans), FALSE) == NULL)
			logout(0);
		ans[sizeofS(ans)] = '\0';
		if (strcmp(line, ans) == 0)
			break;
		if (chkattach(user, ans, ufd, &line, &rdev))
			break;
		printf("[ Cannot attach to %s ]\n", ans);
	}
	printf("[ Reattaching to %s... ]\n", line);
	gtty(0, &sg);		/* Delay until message prints */
	stty(0, &sg);
	if (ioctl(0, TIOCATTACH, &rdev) > 0)
		logout(0);
	printf("[ Reattach failed - continuing with login ]\n");
}

/*
 * return number of detached terminals for user if tty == NULL
 * otherwise, return if check works for line.
 */
chkattach(user, tty, ufd, defp, rdevp)
char *user;
char *tty;
int ufd;
char **defp;
int *rdevp;
{
	struct utmp ut;
	char line[LMAX+1];
	char name[NMAX+1];
	int cnt = 0;
	int prefix = FALSE;
	static char def[LMAX+1];
	char ttname[16];
	struct stat sb;
	int rdev;
	int status;

	lseek(ufd, 0, 0);
	while (read(ufd, &ut, sizeof(struct utmp)) == sizeof(struct utmp)) {
		strncpy(name, ut.ut_name, NMAX);
		name[NMAX] = '\0';
		strncpy(line, ut.ut_line, LMAX);
		line[LMAX] = '\0';
		if (strcmp(name, user) != 0 || strncmp(line, "tty", 3) != 0 ||
		    (tty && strcmp(tty, line) != 0))
			continue;
		strcpy(ttname, "/dev/");
		strcat(ttname, line);
		if (stat(ttname, &sb) < 0)
			continue;
		rdev = sb.st_rdev;
		status = ioctl(0, TIOCCHECK, &rdev);
		if (status > 0) {
			if (tty) {
				*defp = tty;
				*rdevp = rdev;
				return(TRUE);
			}
			cnt++;
			if (!prefix) {
				printf("[ Currently detached from %s", line);
				prefix = TRUE;
				strcpy(def, line);
				*defp = def;
				*rdevp = rdev;
			} else
				printf(", %s", line);
		}
	}
	return(tty ? FALSE : cnt);
}

/*
 *	getnstr - get string (of max length) from user
 *
 *	prompt = string to display to user
 *	def    = default response used if only new-line is typed
 *	buf    = buffer for response
 *	len    = len of buffer
 *
 *	returns NULL on EOF, otherwise the supplied buffer.
 */
char *
getnstr(prompt, def, buf, len, fold)
char *prompt;
char *def;
char *buf;
int len, fold;
{
	register char *cp = buf;
	int c;

	printf("%s [%s]  ", prompt, def);
	fflush(stdout);
	strncpy(cp, def, len);
	while ((c = getchar()) != '\n') {
		if (c == EOF)
			return(NULL);
		if (len <= 0)
			continue;
		len--;
		if (fold && c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		*cp++ = c;
	}
	if (len && cp != buf)
		*cp = '\0';
	return(buf);
}

/*
 *  Validate the login name.
 *
 *  Returns:	FALSE	 if name, account and password are valid for login.
 *		TRUE	 if name, account and/or password are invalid.
 */
notvalid(password, group, account)
char *password;
char *group;
char *account;
{
	register char *p;

	if (code != ACCESS_CODE_NOUSER) {
		register int val;

		grp = NULL;
		acc = NULL;
		val = oklogin(pwd->pw_name, group, &account, password,
			      &pwd, &grp, &acc, &groups);
		if (grp != NULL)
			pwd->pw_gid = grp->gr_gid;
		if (val != ACCESS_CODE_OK)
			code = val;
	}
	switch (code) {
	case ACCESS_CODE_OK:
		p = okpassword(password, pwd->pw_name, pwd->pw_gecos);
		if (p != NULL) {
			fprintf(stderr, "%s\n", p);
			if (changepwd() <= 0) {
				code = ACCESS_CODE_INSECUREPWD;
				return(TRUE);
			}
		}
		warnlogin(account, acc);
		return(FALSE);
	default:
	case ACCESS_CODE_DENIED:
	case ACCESS_CODE_NOUSER:
	case ACCESS_CODE_BADPASSWORD:
		printf("Login incorrect\n");
		return(TRUE);
	case ACCESS_CODE_ACCEXPIRED:
		printf("Sorry, your %s account has expired",
			foldup(account, account));
		break;
	case ACCESS_CODE_GRPEXPIRED:
		printf("Sorry, your %s group account has expired",
			foldup(grp->gr_name, grp->gr_name));
		break;
	case ACCESS_CODE_ACCNOTVALID:
		if (account)
			printf("Sorry, %s is not a valid account",
				foldup(account, account));
		else
			printf("Sorry, %s is not a valid account group",
				foldup(grp->gr_name, grp->gr_name));
		break;
	case ACCESS_CODE_MANYDEFACC:
		printf("Your accounting file entries are inconsistant.\n");
		printf("Please report this problem to Gripe");
		break;
	case ACCESS_CODE_NOACCFORGRP:
		printf("Sorry, your %s group does not have an account",
			foldup(grp->gr_name, grp->gr_name));
		break;
	case ACCESS_CODE_NOGRPFORACC:
		printf("Sorry, your %s account does not have a group",
			foldup(account, account));
		break;
	case ACCESS_CODE_NOGRPDEFACC:
		printf("Sorry, your default account does not have a group");
		break;
	case ACCESS_CODE_NOTGRPMEMB:
		printf("Sorry, you are not a member of the %s group",
			foldup(group, group));
		break;
	case ACCESS_CODE_NOTDEFMEMB:
		printf("Sorry, you are not a member of your default login\n");
		printf("group.  You must specify an explicit group to login");
		break;
	}
	printf(".\n[ \"pracct %s\" will show your current accounts ]\n",
		pwd->pw_name);
	return(TRUE);
}

/*
 *  Insure that the account has a secure password.  Called to invoke
 *  /bin/passwd when the password that they typed was insecure.
 */
changepwd()
{
	/* Don't force change unless /bin/passwd exists and home isn't "/" */
	if (access("/bin/passwd", 1) < 0 || strcmp(pwd->pw_dir, "/") == 0)
		return(1);
	return(run("/bin/passwd", "passwd(login)", pwd->pw_name, 0) != 0);
}

/*
 *  Insure that the account has a "plan" file.  Invoke the MKPLAN program
 *  if none exists or the current plan file is empty.
 */
mkplan()
{
	struct stat sb;
	static char MKPLAN[] = "/etc/mkplan";
	static char plantemp[] = "mkplan.tmp";
	static char planfile[] = ".plan";

	/*
	 * Don't bother with program invocation if working directory is root,
	 * temporary, or user has no write or search permission in the current
	 * directory. Without write permission a plan file cannot be created,
	 * without search permission the existence test is meaningless.  Also,
	 * don't bother if a plan file exists and is non-empty.  Also not worth
	 * going though all this trouble if /etc/mkplan doesn't exist or isn't
	 * executable.
	 */
	if (access(MKPLAN, 1) < 0 ||
	    strcmp(pwd->pw_dir, "/") == 0 ||
	    strcmp(pwd->pw_dir, "/tmp") == 0 ||
	    access(".", 03) < 0 ||
	    (lstat(planfile, &sb) >= 0 && sb.st_size != 0))
		return;
	unlink(planfile);
	if (run(MKPLAN, "mkplan(login)", 0) < 0 || link(plantemp, planfile) < 0)
		printf("\nProblem creating plan file\n");
	unlink(plantemp);
}

ruserok(rhost, superuser, ruser, luser, pwd)
	char *rhost;
	int superuser;
	char *ruser, *luser;
	struct passwd *pwd;
{
	FILE *hostf;
	char rhosts[128];
	char ahost[128];
	struct stat statb;

	if (superuser)
		return (-1);
	strcpy(rhosts, pwd->pw_dir);
	strcat(rhosts, "/.rhosts");
	if (lstat(rhosts, &statb) < 0 ||
		statb.st_mode != (S_IFREG|S_IREAD|S_IWRITE) ||
		statb.st_nlink != 1 ||
		statb.st_uid != 0)
		return (-1);
	hostf = fopen(rhosts, "r");
	if (hostf) {
		while (fgets(ahost, sizeof (ahost), hostf)) {
			char *user;
			if ((user = index(ahost, '\n')) != 0)
				*user = 0;
			if ((user = index(ahost, ' ')) != 0)
				*user++ = 0;
			if (!strcmp(rhost, ahost) &&
			    !strcmp(ruser, user ? user : luser)) {
				(void) fclose(hostf);
				return (0);
			}
		}
		(void) fclose(hostf);
	}
	return (-1);
}

#ifdef	AFS
#ifndef	allusersinafs
print_afs_status(checkinvalid)
int checkinvalid;
#else	/* allusersinafs */
print_afs_status()
#endif	/* allusersinafs */
{
	switch (afs_log_errcode) {
	case AFSERR_NOERR: /* no setup errors, see status */
		if (afs_log_status.w_stopval == WSTOPPED) {
			fprintf(stderr, "[ %s: process stopped ]\n",
				"AFS authentication failed");
			break;
		}
		if (afs_log_status.w_termsig != 0) {
			fprintf(stderr, "[ %s: signal %d ]\n",
				"AFS authentication failed",
				afs_log_status.w_termsig);
			break;
		}
		switch (afs_log_status.w_retcode) {
		case 0:
#ifndef	allusersinafs
			if (!checkinvalid)
			fprintf(stderr,
				"[ Authenticated to %s fileserver ]\n",
				"cs.cmu.edu");
#endif	/* allusersinafs */
			return(0);
		case 1:
			fprintf(stderr, "[ %s: uid not in passwd ]\n",
				"AFS authentication failed");
			break;
		case 2:
			fprintf(stderr, "[ %s: usage error ]\n",
				"AFS authentication failed");
			break;
		case 3:
			fprintf(stderr, "[ %s: user not in passwd ]\n",
				"AFS authentication failed");
			break;
		case 4:
			fprintf(stderr, "[ %s: InitRPC failure ]\n",
				"AFS authentication failed");
			break;
		case 5:
#ifndef	allusersinafs
			if (checkinvalid)
#endif	/* allusersinafs */
			fprintf(stderr, "[ %s: invalid login ]\n",
				"AFS authentication failed");
			break;
		case 6:
			fprintf(stderr, "[ %s: Cache Manager ]\n",
				"AFS authentication failed");
			break;
		default:
			fprintf(stderr, "[ %s: exit %d ]\n",
				"AFS authentication failed",
				afs_log_status.w_retcode);
			break;
		}
		break;
	case AFSERR_NOPASS: /* no password, no authentication */
		fprintf(stderr, "[ %s: no password ]\n",
			"AFS authentication failed");
		break;
	case AFSERR_SIGUSED: /* signals in use, no authentication */
		fprintf(stderr, "[ %s: signals in use ]\n",
			"AFS authentication failed");
		break;
	case AFSERR_PIPE: /* pipe syscall, no authentication */
		fprintf(stderr, "[ %s: pipe: %s ]\n",
			"AFS authentication failed",
			errmsg(afs_log_status.w_status));
		break;
	case AFSERR_FORK: /* fork syscall, no authentication */
		fprintf(stderr, "[ %s: fork: %s ]\n",
			"AFS authentication failed",
			errmsg(afs_log_status.w_status));
		break;
	case AFSERR_SETPAG: /* setpag syscall, no authentication */
		fprintf(stderr, "[ %s: setpag: %s ]\n",
			"AFS authentication failed",
			errmsg(afs_log_status.w_status));
		break;
	case AFSERR_TIMEOUT: /* timed out log, maybe authenticated */
		fprintf(stderr, "[ AFS authentication timed out ]\n");
		break;
	default:
		fprintf(stderr, "[ %s: unknown errcode %d ]\n",
			"AFS authentication failed",
			afs_log_errcode);
		break;
	}
	return(1);
}
#endif	AFS
#endif	CMUCS
