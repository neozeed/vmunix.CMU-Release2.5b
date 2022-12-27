/***********
 * HISTORY
 * $Log:	main.c,v $
 * Revision 1.9  89/07/24  12:07:14  mikeg
 * 	Commented out use of CkNamedTime in mothernanny() to
 * 	determine value of timeout.tv_sec.
 * 	[89/07/24  10:18:17  mikeg]
 * 
 * Revision 1.8  89/07/18  17:14:05  mikeg
 * 	Commented out restart of named in mothernanny() function.
 * 	nanny was killing named too often.
 * 	[89/07/18  17:10:13  mikeg]
 * 
 * 29-Nov-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added -host switch for remote status checks.
 *
 * 28-Sep-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Call correct internal routine for ck_depend().
 *
 * 05-Aug-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added check of named to main server loop.
 *
 * 03-Apr-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Swapped arguments to signal routine since they were backwards. oops.
 *
 * 30-Mar-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Add a restart time to enable nanny to be more mellow about
 *	restarting servers. If a server dies soon after getting
 *	restarted then nanny will not restart it until the restart
 *	time plus the delay interval has passed. A kill request
 *	will clear the restart time. In addition, show the restart
 *	time.
 *
 * 05-Nov-86  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Add more error checks for easier debugging. Print out
 *	error messages instead of numbers. Simplified some code.
 *
 */
#include "nanny.h"

#define MAXWAIT (15*MINUTES)

/* random global variables */
int	nanny_num;	/* The id of this nanny */
int	verbose;	/* verbose flag */
int	omask;		/* Mask that nanny inheritied */
int	dying;		/* true if shutting down */

char	**attrib;	/* Attributes of this host */
char	*logdir;	/* Directory where log files are to be kept */
char	*config;  	/* Configuration file */
char	*host;		/* Remote host name */
SERVER	*SrvHead;	/* current list of servers */

static int somethingdied = 0;
deathwakeup()
{
    somethingdied++;
}

/*
 * The mother nanny is responsible for handling requests from
 * request nanny's, loading and tracking configuration files,
 * collecting status information from subnanny's and in general
 * maintaining the state of the world. It worries about saturating
 * the communication channel...therefore it has lots of state
 * information.
 */
mothernanny()
{
    int pid, tty;
    int mask;
    struct timeval timout;
    int i;
    long CnfgTime, now, tmp, DpndTime, CkNamedTime;
    SERVER *srv;

    /* detach us */

    if ((pid = fork()) > 0)
	exit(0);
    else if (pid < 0)
	quit(-1,"Unable to fork for detached nanny - %s\n",errmsg(errno));

    if ((tty = open("/dev/tty", O_RDWR, 0644)) >= 0) {
	if (ioctl(tty, TIOCNOTTY, (char *)0) != 0) {
	    print_log("TIOCNOTTY ioctl failed - %s\n", errmsg(errno));
	    (void)close(tty);
	}
    }

    (void)signal(SIGCHLD,deathwakeup);	/* signal notice */
    open_log();

    /* do an initial load of our host name */
    if (getmyhostinfo()) {
	print_err("getmyhostinfo failed before init.\n");
	exit(1);
    }

    /* say hello in the log file */
    now = time((long *)NULL);
    print_log("Nanny version III started at %24.24s on %s\n", ctime(&now),
	      myhostname);

    /* create the request socket */
    open_listen();

    /* process requests until time ends */
    DpndTime = CnfgTime = CkNamedTime = 0L;
    for (;;) {
	/* figure out the next event time */
	now = time((long *)NULL);
	timout.tv_sec = CnfgTime - now;
	tmp = DpndTime - now;
	if (timout.tv_sec > tmp)
	    timout.tv_sec = tmp;
	/*tmp = CkNamedTime - now;*/
	if (timout.tv_sec > tmp)
	    timout.tv_sec = tmp;
	if (timout.tv_sec > MAXWAIT)
	    timout.tv_sec = MAXWAIT;
	if (timout.tv_sec < 0)
	    timout.tv_sec = 0;
	timout.tv_usec = 0;
	if (somethingdied)
	    timout.tv_sec = 0;
	if ((srv=SrvHead) != NULL) do {
	    /* see if we are waiting for a restart */
	    if (srv->valid && srv->status == DEAD) {
		tmp = (srv->resttim + RESTARTMAX) - time((long *)NULL);
		if (tmp >= 0 && tmp < timout.tv_sec)
		    timout.tv_sec = tmp;
	    }		    
	} while ((srv=srv->Next) != SrvHead);

	if (verbose)
	    print_log("Timeout is %d seconds\n",timout.tv_sec);

	/* figure out what file descriptors we care about */
	mask = (nanny_sock < 0) ? 0 : (1 << nanny_sock);
	if (mask == 0) {
	    dying = 1;
	    print_log("nanny_sock is zero - dying\n");
	    if ((srv=SrvHead) != NULL) do {
		kill_srv(srv);
	    } while ((srv=srv->Next) != SrvHead);
	    break;
	}
	else if ((i = select(30, (fd_set *)&mask, (fd_set *)NULL, (fd_set *)NULL, &timout)) < 0) {
	    if (errno != EINTR)
		print_log("Select returned err='%s'\n", errmsg(errno));
	} else if (mask != 0) {
	    for(i=0;i<30;i++) {
		if ((mask & (1 << i)) != 0) {
		    rcv_comm(i);
		}
	    }
	}
	/* else a timeout or interruption */

	/* check and see if we should do something at this time */
	now = time((long *)NULL);
	if (now >= DpndTime) {
	    /* check the files the servers are dependent on */
	    ck_depend();
	    DpndTime = now + DPNDCHK;
	    now = time((long *)NULL);
	}
	if (now >= CnfgTime) {
	    /* check the configuration file */
	    Check_cnfg((char *)NULL);
	    CnfgTime = now + CNFGCHK;
	    now = time((long *)NULL);
	}
/*
 *	Comment out named restart - nanny kills and restarts it 
 *	unnecessarily often.
 *						- mikeg
 */
  	/*if (now >= CkNamedTime) {*/
  	    /* check named status */
  	/*    Check_Named();
  	    CkNamedTime = now + NAMEDCHK;
  	    now = time((long *)NULL);
	}*/
 

	CheckDeaths();
	LifeChecks();

	/* see if we have a reason to live */
	if (dying && anscount >= reqcount) {
	    tmp = 0;
	    if ((srv=SrvHead) != NULL) do {
		if (srv->valid)
		    if (srv->status == RUNNING || srv->status == STOPPED) 
			tmp++;
	    } while ((srv=srv->Next) != SrvHead);
	    if (tmp == 0) {
		print_log("All servers dead\n");
		break;
	    }
	}
    }
    /* here when we are dying */
    if (verbose)
	print_log("Got %d answers to %d requests\n",
		   anscount,reqcount);
    print_log("Bye.\n");
    exit(0);
}

/**** RunServer:
 *   The given pointer is used to create a server child. A pipe will be set
 * up between this process and the child to direct the child's output to this
 * process. The file descriptor for the pipe is placed in the server's 
 * structure.
 *   In addition to opening the output pipe from the server a file is open 
 * for the log output to go to. The path up to the / is assumed to have
 * already been specified.
 *   After the fork we reset the signals to there default states, restore
 * the file creation mask to what it was when nanny was started.
 * We then go close all files other than the stdin, stdout, and stderr.
 * Set the nice for the child and then chdir to /tmp to keep the server's 
 * working directory path in a safe place.
 *   Finally, the new server is exec'ed.
 */
RunServer(srv)
SERVER *srv;
{
    int     fd,
            tmp,
           *id;
    char  **str;

    /* ignore requests for not dead or invalid servers */

    if (srv->status != DEAD || !srv->valid)
	return;

    /* don't restart too frequently */
    if (srv->resttim + RESTARTMAX > time((long *)NULL))
	return;

    /* record the restart attempt */
    srv->resttim = time((long *)NULL);

    /* make sure we have access to all the files */

    for (str = srv->basis->runfile; *str; str++)
	if (access(*str, F_OK)) {
	    print_log("Unable to find file %s for %s: %s\n", 
		*str, srv->basis->name,errmsg(errno));
	    srv->status = ABORTED;
	    return;
	}

    if ((srv->pid = fork()) == 0) {	    /* child */
	(void)umask(omask);
	for (fd = getdtablesize() - 1; fd >= 0; fd--) /* close all files */
	    (void)close(fd);
	(void)open("/dev/null", O_RDONLY, 0644); /* don't give job our stdin */
	(void)dup2(0, 1);			/* redirect stdout & stderr */
	(void)dup2(0, 2);

	/* at this point no special nanny log i/o can be done */

	if (setpriority(PRIO_PROCESS, getpid(), srv->basis->nice))
	    print_err("Unable to set priority to %d: %s\n",
		    srv->basis->nice, errmsg(errno));

	if (chdir(SRVDIR) < 0)
	    print_err("Unable to change directory to %s: %s\n",
		   SRVDIR,errmsg(errno));

	for (id = srv->gid, tmp = 0; *id != -1; tmp++, id++);
	if (setgroups(tmp, srv->gid))
	    quit(2, "Unable to set GIDs: %s\n", errmsg(errno));
	for (id = srv->gid; *id != -1; id++)
	    print_log("Group ID set to (%d)\n", *id);

	if (*srv->gid == -1 || setgid((gid_t)*srv->gid) < 0)
	    quit(2, "Unable to set GID to %s(%d)\n", *srv->basis->gid, *srv->gid);
	print_log("Group ID set to %s(%d)\n", *srv->basis->gid, srv->gid);

	if (setuid((uid_t)srv->uid) < 0)
	    quit(2, "Unable to set UID to %s(%d)\n", *srv->basis->uid, srv->uid);
	print_log("User ID set to %s(%d)\n", *srv->basis->uid, srv->uid);

	print_log("Starting %s with: ", srv->basis->name);
	for (str = srv->basis->script; *str; str++)
	    print_log("\"%s\" ", *str);
	print_log("\n");

	execv(srv->basis->script[0], srv->basis->script);
	quit(2, "Startup failed - %s", errmsg(errno));
    }

    srv->tim = time((long *)NULL);
    print_log("Started server %s\n", srv->basis->name);

    srv->status = RUNNING;
    srv->restarts++;
}

/*
 * make sure that all nannys that are suppose to be running
 * are and any other nannys killed. For the subnannys, check
 * and make sure the servers are running or get rid of them
 * if we don't care about them any more.
 */
int LifeChecks()
{
    SERVER *srv, *NewHead;

    if (dying) 
	return;

    if (verbose)
	print_log("Checking for life creation\n");

    /*
     * this is the key garbage collection code for subnanny
     */

    NewHead = NULL;
    while (SrvHead != NULL) {
	srv = (SERVER *)DeQHead(&SrvHead);
	/* make sure valid servers are running */
	if (verbose)
	    print_log("LifeCheck looking at %s\n",srv->basis->name);
	if (srv->valid) {
	    if (srv->pid <= 0) {
		if (verbose)
		    print_log("running %s\n",srv->basis->name);
		RunServer(srv);
	    } /* else wait for death to be handled */
	}
	else if (srv->status == RUNNING || srv->status == STOPPED) {
	    if (verbose)
		print_log("killing %s\n",srv->basis->name);
	    kill_srv(srv);
	}
	else {
	    /* time to get rid of server */
	    print_log("freeing %s\n",srv->basis->name);
	    FreeSrv(srv);
	    srv = NULL;
	}
	if (srv != NULL)
	    QueueTail(&NewHead,srv);
    }
    SrvHead = NewHead;
}



/*
 * a process died on us -- handle it whether it is nanny or server
 */

CheckDeaths()
{
    SERVER         *srv = NULL, *tsrv;
    int             chld_pid;
    union wait      status;
    char	    tmpbuf[128];

    if (verbose)
	print_log("Checking deaths\n");

    /* clear flag */
    somethingdied = 0;

    /* find dead */
    while (chld_pid = wait3(&status, WNOHANG, (struct rusage *)NULL), chld_pid > 0) {
	print_log("pid %d died err=%d sig=%d%s\n",
		    chld_pid,
		    status.w_retcode,
		    status.w_termsig,
		    status.w_coredump ? " (core dumped)":"");
	/* check primary list */
	srv = NULL;
	if ((tsrv=SrvHead) != NULL) do {
	    if (tsrv->pid == chld_pid) {
		srv = tsrv;
		break;
	    }
	}
	while ((tsrv = tsrv->Next) != SrvHead);
	/* handle it */
	if (srv != NULL) {
	    srv->pid = -1;
	    if (srv->status == RUNNING || srv->status == STOPPED)
		srv->status = DEAD;
	    (void)sprintf(tmpbuf,"server %s found terminated err=%d sig=%d%s\n",
		  srv->basis->name,
		  status.w_retcode,
		  status.w_termsig,
		  status.w_coredump ? " (core dumped)":"");
	    print_log(tmpbuf);
	    if (status.w_retcode == 2) {
		(void)sprintf(tmpbuf,"Disabling %s\n", srv->basis->name);
		print_log(tmpbuf);
		srv->status = ABORTED;
	    }
	}
	/* else we in deep trouble */
	else
	    print_log("Non server child terminated, PID = %d\n", chld_pid);
    }
}

/**** ck_depend:
 *  Look through the list of servers and make sure that the
 * executable files has not changed since last we looked. If 
 * any of the server's dependancy files have changed, advise 
 * the nanny server responsible.
 */
ck_depend()
{
    SERVER * srv;
    char  **tmp, *filnam;
    long    tim;

    if ((srv = SrvHead) == NULL)
	return;

    do {
	if (srv->status == KILLED || srv->status == ABORTED || !srv->valid)
	    continue;
	if (srv->status == RUNNING && srv->valid) {
	    errno = 0;
	    tim = 0;
	    for (tmp = srv->basis->runfile; filnam = *tmp; tmp++) {
		tim = MAX(tim, last_mod(filnam));
		if (errno) {
		    print_log("Unable to check up on %s' dependancy file %s\n"
			      ,srv->basis->name,filnam);
		    kill_srv(srv);
		    break;
		}
	    }
	    if (tim > srv->tim) {
		print_log("One of %s's files has been altered\n"
			  ,srv->basis->name);
		srv->tim = tim;
		restart_srv(srv);
		break;
	    }
	}
    } while ((srv = srv->Next) != SrvHead);
}

/*
 * request nanny is the user nanny that communicates requests
 * to the mother or master nanny.
 */

reqnanny()
{
    int mask;
    struct timeval timout;
    int i;

    
    while (anscount < reqcount) {
	timout.tv_sec = 60;
	timout.tv_usec = 0;
	if (verbose)
	    print_log("Timeout is %d seconds\n",timout.tv_sec);

	/* figure out what file descriptors we care about */
	mask = (nanny_sock < 0) ? 0 : (1 << nanny_sock);
	if (mask == 0) {
	    print_log("nanny_sock is zero - exiting\n");
	    break;
	}
	else if ((i = select(30, (fd_set *)&mask, (fd_set *)NULL, (fd_set *)NULL, &timout)) < 0) {
	    if (errno != EINTR)
		print_log("Select returned err='%d'\n", errmsg(errno));
	} else if (i != 0 && mask != 0) {
	    for(i = 0; i<30; i++)
		if ((mask & (1 << i)) != 0)
		    rcv_comm(i);
	}
    }
    if (verbose)
	print_log("Got %d answers to %d requests\n",anscount,reqcount);
    if (verbose)
	print_log("Bye.\n");
    exit(0);
}

print_st_head()
{
printf("  PID   STATE RESTARTS VALID SERVER\n");
printf("----- ------- -------- ----- ------------\n");
}

char *mod_str[] = {
	"Unknown",
	"Running",
	"Dead",
	"Stopped",
	"Killed",
	"Aborted"
};

/**** print_state:
 *  Do a nice job of printing the given state.
 */
print_state(s)
SERVER *s;
{
    char reststr[60];
    int restdelay;

    *reststr = 0;
    if (s->status == DEAD) {
	restdelay = (s->resttim + RESTARTMAX) - time((long *)NULL);
	if (restdelay > 0)
	    (void)sprintf(reststr," (restart in %d:%02d)",restdelay/60,restdelay%60);
    }

    if (s)
	printf("%5d %7s %8d %5s %s%s\n"
	       ,s->pid
	       ,mod_str[s->status]
	       ,s->restarts > 0 ? s->restarts : 0
	       ,s->valid ? "yes" : " No"
	       ,s->basis->name
	       ,reststr);
}

char   *switches[] = {
    "-host",
    "-logdir",
    "-init",
    "-start",
    "-list",
    "-status",
    "-continue",
    "-stop",
    "-kill",
    "-restart",
    "-reconfigure",
    "-Suicide",
    "-debug",
    "-display",
    "-small",
    0
};
enum Funcs {
    F_HOST,
    F_LOG,
    F_INIT,
    F_START,
    F_LIST,
    F_STAT,
    F_CONT,
    F_STOP,
    F_KILL,
    F_RESTART,
    F_REC,
    F_SUI,
    F_DEBUG,
    F_DIS,
    F_SMALL
};

/**** main:
 */
main(argc, argv)
int argc;
char *argv[];
{
    int             flag;

    /* make sure we run as opr */
    if (!chkopr())
    {
	fprintf(stderr, "nanny: insufficient privledges\n");
	exit(1);
    }

    /* initialize */

    dying = 0;				/* not dying yet */
    attrib = NULL;			/* no host attributes */
    logdir = salloc(LOGPATH);		/* initial log directory */
    config = NULL;			/* no config file yet */
    SrvHead = NULL;			/* global server */
    verbose = 0;			/* no debugging output */
    nanny_num = REQNANNY;		/* assume user nanny */
    host = NULL;			/* no remote host */

    /* set up our umask and remember the old one */
    omask = umask(DIRMASK);

    /* skip over program name */
    argc--;
    argv++;

    if (argc <= 0)
	quit(-1,"no user mode for nanny\n");

    while (argc--) {
	if (**argv == '-') {
	    flag = stablk(*argv++, switches, 1);
	    if (flag < 0) {
		fprintf(stderr,"nanny: switch '%s' unknown\n",argv[-1]);
		continue;
	    }
	}
	else
	    quit(-1,"nanny: unexpected argument '%s'\n",argv);

	switch (flag) {
	    case F_HOST:		/* set remote host */
		if (host != NULL) {
		    free(host);
		    host = NULL;
		}
		if (argc && **argv != '-') {
		    host = salloc(*argv++);
		    argc--;
		}
		break;
	    case F_LOG:			/* assign a logging dir */
		if (argc && **argv != '-') {
		    free(logdir);
		    logdir = salloc(*argv++);
		    argc--;
		}
		break;
	    case F_INIT:		/* Set up as a mother */
		if (config != NULL)
		    free(config);
		if (argc && **argv != '-') {
		    config = salloc(*argv++);
		    argc--;
		}
		else
		    config = salloc(CNFGFIL);
		nanny_num = MOMNANNY;
		mothernanny();
		break;
	    case F_LIST:
		/* just do the switch */
		open_comm();
		req_list((char *)NULL);
		reqnanny();
		break;
	    case F_STAT:
		open_comm();
		print_st_head();
		if (!argc || **argv == '-')
		    req_status((char *)NULL);
		else while (argc && **argv != '-') {
		    req_status(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_CONT:
		open_comm();
		while (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting continue of %s\n", *argv);
		    req_cont(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_STOP:
		open_comm();
		while (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting stop of %s\n", *argv);
		    req_stop(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_KILL:
		open_comm();
		while (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting kill of %s\n", *argv);
		    req_kill(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_RESTART:
		open_comm();
		while (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting restart of %s\n", *argv);
		    req_restart(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_REC:
		open_comm();
		if (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting reconfiguration using %s\n", *argv);
		    req_recon(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_START:
		open_comm();
		while (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting start of %s\n", *argv);
		    req_cont(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_SUI:
		open_comm();
		if (verbose)
		    print_log("Requesting suicide\n");
		req_die();
		reqnanny();
		break;
	    case F_DEBUG:
		verbose++;
		break;
	    case F_DIS:
		open_comm();
		while (argc && **argv != '-') {
		    if (verbose)
			print_log("Requesting display of %s\n", *argv);
		    req_display(*argv++);
		    argc--;
		}
		reqnanny();
		break;
	    case F_SMALL:
		break;
	    default:
		print_err("Internal error processing switches\n");
		break;
	}
    }

    exit(0);
}

