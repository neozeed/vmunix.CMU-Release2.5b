/*
 *  Top users display for Berkeley Unix
 *  Version 1.8
 *
 *  This program may be freely redistributed to other Unix sites, but this
 *  entire comment MUST remain intact.
 *
 *  Copyright (c) 1984, William LeFebvre, Rice University
 *
 *  This program is designed to run on either Berkeley 4.1 or 4.2 Unix.
 *  Compile with the preprocessor constant "FOUR_ONE" set to get an
 *  executable that will run on Berkeley 4.1 Unix.
 *
 *  The Sun kernel uses scaled integers instead of floating point so compile
 *  with the preprocessor variable "SUN" to get an executable that will run
 *  on Sun Unix version 1.1 or later.
 *
 *  Fixes and enhancements since version 1.5:
 *
 *  Jonathon Feiber at sun:
 *	added "#ifdef SUN" code to make top work on a Sun,
 *	fixed race bug in getkval for getting user structure,
 *	efficiency improvements:  added register variables and
 *	removed the function hashit
 *
 *	added real and virtual memory status line
 *
 *	added second "key" to the qsort comparisn function "proc_compar"
 *	which sorts by on cpu ticks if percentage is equal
 *
 **********************************************************************
 * HISTORY
 * 30-Aug-88  Mary R. Thompson (mrt) at Carnegie Mellon
 *	Changed avenrun.tl_avenrun[i] to aenrun.tl_avenrun.l[i] for
 *	new include files.
 *  6-May-88  David Golub (dbg) at Carnegie-Mellon University
 *	Completely rewritten for MACH.  This version will NOT run on any
 *	other version of BSD.
 *
 */

#ifndef	KERNEL_FEATURES
#define	KERNEL_FEATURES		/* get include-version number */
#endif	KERNEL_FEATURES

#include <sys/features.h>	/* necessary? */
#include <sys/version.h>	/* this is what we really want */

#if	(INCLUDE_VERSION == 12)
#define	PROC_COMPAT	1	/* if TBL_PROCINFO missing, read proc structure
				   from /dev/kmem */
#endif


#define	Default_TOPN	10	/* default number of lines */
#define	Default_DELAY	15	/* default delay interval */

#include <curses.h>
#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/table.h>

#include <mach.h>

char	*malloc();

/* old time calls */
extern long	time();
extern char	*ctime();

/* Number of lines of header information on the standard screen */
#define	Header_lines	6

#define sec_to_minutes(t)       ((t) / 60)
#define sec_to_seconds(t)       ((t) % 60)
#define usec_to_100ths(t)       ((t) / 10000)

#ifndef	TH_USAGE_SCALE
#define	TH_USAGE_SCALE	1000
#endif	TH_USAGE_SCALE
#define usage_to_percent(u)	((u*100)/TH_USAGE_SCALE)
#define usage_to_tenths(u)	(((u*1000)/TH_USAGE_SCALE) % 10)


struct	proc_info {
	uid_t			uid;
	short			pid;
	short			ppid;
	short			pgrp;
	int			status;
	int			flag;

	int			state;
	int			pri;
	int			base_pri;
	boolean_t		all_swapped;
	time_value_t		total_time;

	vm_size_t		virtual_size;
	vm_size_t		resident_size;
	int			cpu_usage;

	char			command[20];

	int			num_threads;
	thread_basic_info_t	threads;	/* array */
};
typedef	struct proc_info	*proc_info_t;

#ifdef	PROC_COMPAT
int	kmemf = -1;		/* /dev/kmem, only if used */
#endif	PROC_COMPAT



/*
 *	Translate thread state to a number in an ordered scale.
 *	When collapsing all the threads' states to one for the
 *	entire task, the lower-numbered state dominates.
 */
#define	STATE_MAX	7

int
mach_state_order(s, sleep_time)
        int s;
        long sleep_time;
 {
    switch (s) {
    case TH_STATE_RUNNING:      return(1);
    case TH_STATE_UNINTERRUPTIBLE:
                                return(2);
    case TH_STATE_WAITING:      return((sleep_time > 20) ? 4 : 3);
    case TH_STATE_STOPPED:      return(5);
    case TH_STATE_HALTED:       return(6);
    default:                    return(7);
    }
 }
			    /*01234567 */
char	mach_state_table[] = " RUSITH?";

char *	state_name[] = {
		"zombie",
		"running",
		"stuck",
		"sleeping",
		"idle",
		"stopped",
		"halted"
};

char *state_to_string(pi)
	proc_info_t	pi;
{
    static char	s[5];		/* STATIC! */

    s[0] = mach_state_table[pi->state];
    s[1] = (pi->all_swapped) ? 'W' : ' ';
    s[2] = (pi->base_pri > 50) ? 'N' :
		(pi->base_pri < 40) ? '<' : ' ';
    s[3] = ' ';
    s[4] = '\0';
    return(s);
}

print_time(seconds)
    long	seconds;
{
    if (seconds < 999*60) {
	printw("%3d:%02d", seconds/60, seconds % 60);
    }
    else if (seconds < 9999*60*60) {
	printw("%4dhr", seconds/(60*60));
    }
    else {
	printw("%3dday", seconds/(60*60*24));
    }
}


char *
digits(n)
    float	n;
{
    static char	tmp[10];	/* STATIC! */

    if ((n > 0) && (n < 10))
	sprintf(tmp, "%4.2f", n);
    else if ((n > 0) && (n < 100))
	sprintf(tmp, "%4.1f", n);
    else
	sprintf(tmp, "%4.0f", n);
    return(tmp);
}

char *
mem_to_string(n)
    unsigned	int	n;
{
    static char	s[10];		/* STATIC! */

    /* convert to bytes */
    n /= 1024;

    if (n > 1024*1024)
	sprintf(s, "%sG", digits(((float)n)/(1024.0*1024.0)));
    else if (n > 1024)
	sprintf(s, "%sM", digits((float)n/(1024.0)));
    else
	sprintf(s, "%dK", n);

    return( s );
}

/* All of this should come out of the process manager... */

void get_proc_info(ppb, pi)
    struct tbl_procinfo *ppb;
    struct proc_info	*pi;
{
    task_t	task;

    pi->uid	= ppb->pi_uid;
    pi->pid	= ppb->pi_pid;
    pi->ppid	= ppb->pi_ppid;
    pi->pgrp	= ppb->pi_pgrp;
    pi->status	= ppb->pi_status;
    pi->flag	= ppb->pi_flag;

    /*
     *	Find the other stuff
     */
    if (task_by_unix_pid(task_self(), pi->pid, &task) != KERN_SUCCESS) {
	pi->status = PI_ZOMBIE;
    }
    else {
	task_basic_info_data_t	ti;
	unsigned int		count;
	thread_array_t		thread_table;
	unsigned int		table_size;

	thread_basic_info_t	thi;
	thread_basic_info_data_t
				thi_data;
	int			i, t_state;

	count = TASK_BASIC_INFO_COUNT;
	if (task_info(task, TASK_BASIC_INFO, (task_info_t)&ti,
		      &count)
		!= KERN_SUCCESS) {
	    pi->status = PI_ZOMBIE;
	}
	else {
	    pi->virtual_size = ti.virtual_size;
	    pi->resident_size = ti.resident_size;

	    (void)task_threads(task, &thread_table, &table_size);

	    pi->total_time = ti.user_time;
	    time_value_add(&pi->total_time, &ti.system_time);

	    pi->state = STATE_MAX;
	    pi->pri = 255;
	    pi->base_pri = 255;
	    pi->all_swapped = TRUE;
	    pi->cpu_usage = 0;
    
	    thi = &thi_data;

	    for (i = 0; i < table_size; i++) {
		count = THREAD_BASIC_INFO_COUNT;
		if (thread_info(thread_table[i], THREAD_BASIC_INFO,
				(thread_info_t)thi, &count) == KERN_SUCCESS) {
		    time_value_add(&pi->total_time, &thi->user_time);
		    time_value_add(&pi->total_time, &thi->system_time);
		    t_state = mach_state_order(thi->run_state,
					       thi->sleep_time);
		    if (t_state < pi->state)
			pi->state = t_state;
		    if (thi->cur_priority < pi->pri)
			pi->pri = thi->cur_priority;
		    if (thi->base_priority < pi->base_pri)
			pi->base_pri = thi->base_priority;
		    if ((thi->flags & TH_FLAGS_SWAPPED) == 0)
			pi->all_swapped = FALSE;
		    pi->cpu_usage += thi->cpu_usage;

		}
	    }
	    (void) vm_deallocate(task_self(), (vm_offset_t)thread_table,
				table_size * sizeof(*thread_table));
	}
    }

    (void) strncpy(pi->command, ppb->pi_comm,
			sizeof(ppb->pi_comm)-1);
    pi->command[sizeof(ppb->pi_comm)-1] = '\0';
}

/*
 *  signal handlers
 */

leave()			/* exit under normal conditions -- INT handler */
{
    move(LINES - 1, 0);
    refresh();
    endwin();
    exit(0);
}

quit(status)		/* exit under duress */
int status;
{
    endwin();
    exit(status);
}

onalrm()
{
    return(0);
}

/*
 *  comparison function for "qsort"
 *  Do first order sort based on cpu percentage computed by kernel and
 *  second order sort based on total time for the process.
 */
 
proc_compar(p1, p2)

register struct proc_info **p1;
register struct proc_info **p2;

{
    if ((*p1)->cpu_usage < (*p2)->cpu_usage)
        return(1);
    else if ((*p1)->cpu_usage > (*p2)->cpu_usage)
	return(-1);
    else {
	if ((*p1)->total_time.seconds < (*p2)->total_time.seconds)
	    return(1);
	else
	    return(-1);
    }
}


main(argc, argv)
	int	argc;
	char	*argv[];
{

    char	*myname = "top";

    int		delay = Default_DELAY;
    int		topn  = Default_TOPN;

    int		nproc;
    struct tbl_procinfo	*pbase, *ppb;
    struct proc_info	*proc,  *pp;
    struct proc_info	**pref, **prefp;

    int		mpid;
    int		total_procs;
    int		active_procs;
    int		i;

    struct tbl_loadavg	avenrun;
    long	curr_time;

    int		state_breakdown[STATE_MAX+1];

    vm_size_t	total_virtual_size;
    vm_size_t	active_virtual_size;

    vm_statistics_data_t	vm_stat;

    /* get our name */
    if (argc > 0) {
	if ((myname = rindex(argv[0], '/')) == 0) {
	    myname = argv[0];
	}
	else {
	    myname++;
	}
    }

    /* check for time delay option */
    if (argc > 1 && argv[1][0] == '-') {

	if (argv[1][1] != 's') {
	    fprintf(stderr, "Usage: %s [-sn] [number]\n", myname);
	    exit(1);
	}

	delay = atoi(&argv[1][2]);
	argc--;
	argv++;
    }

    /* get count of top processes to display (if any) */
    if (argc > 1) {
	topn = atoi(argv[1]);
    }

    /* allocate space for proc structure array and array of pointers */
    nproc = table(TBL_PROCINFO, 0, (char *)0, 32767, 0);
#ifdef	PROC_COMPAT
    if (nproc < 0)
	nproc = compat_proc_setup();
#endif	PROC_COMPAT
    pbase = (struct tbl_procinfo *)
		malloc(nproc * sizeof(struct tbl_procinfo));
    proc  = (struct proc_info *)
		malloc(nproc * sizeof(struct proc_info));
    pref  = (struct proc_info **)
		malloc(nproc * sizeof(struct proc_info *));

    /* initializes curses and screen (last) */
    initscr();
    erase();
    clear();
    refresh();

    /* set up signal handlers */
    signal(SIGINT, leave);
    signal(SIGQUIT, leave);

    /* can only display (LINES - Header_lines) processes */
    if (topn > LINES - Header_lines) {

	printw("Warning: this terminal can only display %d processes...\n",
		LINES - Header_lines);
	refresh();
	sleep(2);
	topn = LINES - Header_lines;
	clear();
    }

    /* main loop */

    for (;;) {

	/* read all of the process information */
#ifdef	PROC_COMPAT
	if (kmemf != -1)
	    compat_get_procs(pbase, nproc);
	else
#endif	PROC_COMPAT
	(void) table(TBL_PROCINFO, 0, (char *)pbase, nproc, sizeof(pbase[0]));

	/* get the cp_time array */

	/* get the load averages */
	(void) table(TBL_LOADAVG, 0, (char *)&avenrun, 1, sizeof(avenrun));

	/* get total - systemwide main memory usage structure */
	(void)vm_statistics(task_self(), &vm_stat);

	/* count up process states and get pointers to interesting procs */

	mpid = 0;
	total_procs = 0;
	active_procs = 0;
	bzero((char *)state_breakdown, sizeof(state_breakdown));
	total_virtual_size = 0;
	active_virtual_size = 0;

	prefp = pref;
	for (ppb = pbase, pp = proc, i = 0;
	     i < nproc;
	     ppb++, pp++, i++) {

	    /* place pointers to each valid proc structure in pref[] */

	    if (ppb->pi_status != PI_EMPTY && ppb->pi_pid != 0) {
		total_procs++;
		get_proc_info(ppb, pp);
		if (ppb->pi_status != PI_ZOMBIE) {
		    *prefp++ = pp;
		    active_procs++;
		    if (pp->pid > mpid)
			mpid = pp->pid;
		    state_breakdown[pp->state]++;
		    total_virtual_size += pp->virtual_size;
		    if (!pp->all_swapped) {
			active_virtual_size += pp->virtual_size;
		    }
		}
		else {
		    state_breakdown[0]++;
		}
	    }
	}

	/* display the load averages */
	printw("last pid: %d;  load averages", mpid);
	for (i = 0; i < 3; i++) {
	    printw("%c %4.2f",
		i == 0 ? ':' : ',',
		(double)avenrun.tl_avenrun.l[i] / (double)avenrun.tl_lscale);
	}

	/*
	 *  Display the current time.
	 *  "ctime" always returns a string that looks like this:
	 *  
	 *	Sun Sep 16 01:03:52 1973
	 *      012345678901234567890123
	 *	          1         2
	 *
	 *  We want indices 11 thru 18 (length 8).
	 */

	curr_time = time((long *)0);
	move(0, 79-8);
	printw("%-8.8s\n", &(ctime(&curr_time)[11]));


	/* display process state breakdown */
	printw("%d processes", total_procs);
	for (i = 0; i <= STATE_MAX; i++) {
	    if (state_breakdown[i] != 0) {
		printw("%c %d %s%s",
			i == 0 ? ':' : ',',
			state_breakdown[i],
			state_name[i],
			(i == 0 && state_breakdown[0] > 1) ? "s" : ""
		      );
	    }
	}
	printw("\n");

	/* calculate percentage time in each cpu state */

	/* display main memory statistics */
	{
	    vm_size_t	total_resident_size,
			active_resident_size,
			free_size;

	    total_resident_size  = (vm_stat.active_count +
				    vm_stat.inactive_count +
				    vm_stat.wire_count) * vm_stat.pagesize;
	    active_resident_size = vm_stat.active_count * vm_stat.pagesize;
	    free_size		 = vm_stat.free_count   * vm_stat.pagesize;

	    printw("Memory: ");
	    printw("%5.5s", mem_to_string(total_resident_size));
	    printw(" (%5.5s) real, ", mem_to_string(active_resident_size));
	    printw("%5.5s", mem_to_string(total_virtual_size));
	    printw(" (%5.5s) virtual, ", mem_to_string(active_virtual_size));
	    printw("%5.5s free\n", mem_to_string(free_size));
	}

	/* display the processes */
	if (topn > 0)
	{
	    printw("\n  PID USERNAME PRI NICE   SIZE   RES STATE   TIME   WCPU    CPU COMMAND\n");
    
	    /* sort by cpu percentage (pctcpu) */
	    qsort((char *)pref,
		  active_procs,
		  sizeof(struct proc_info *),
		  proc_compar);
    
	    /* now, show the top whatever */
	    if (active_procs > topn)
	    {
		/* adjust for a lack of processes */
		active_procs = topn;
	    }
	    for (prefp = pref, i = 0; i < active_procs; prefp++, i++)
	    {
		pp = *prefp;

		printw("%5d %-8.8s %3d %4d  ",
		    pp->pid,				/* pid */
		    getname(pp->uid),			/* username */
		    pp->pri - 50,			/* priority */
		    (pp->base_pri / 2) - 25);		/* 'nice' */
		printw("%5.5s ",
		    mem_to_string(pp->virtual_size));	/* size */
		printw("%5.5s ",
		    mem_to_string(pp->resident_size));	/* res size */
		printw("%-5s ",
		    state_to_string(pp));		/* state */

		print_time(pp->total_time.seconds);	/* cputime */

		printw(" %3d.%01d%%        %-14.14s\n",
		    usage_to_percent(pp->cpu_usage),
		    usage_to_tenths(pp->cpu_usage),	/* %cpu */
							/* raw %cpu */
		    pp->command);			/* command */
	    }
	}
	refresh();

	/* wait ... */
	signal(SIGALRM, onalrm);
	alarm(delay);
	pause();

	/* clear for new display */
	erase();
    }
}

#ifdef	PROC_COMPAT
/*
 * Read the proc table on old kernels.
 */
#include <nlist.h>
#include <sys/file.h>

#include <sys/proc.h>
#include <sys/user.h>

char	*name_list_file = "/vmunix";

off_t get_kernel_long(kmem, loc)
	int	kmem;
	off_t	loc;
{
    off_t	word;

    lseek(kmem, loc, L_SET);
    if (read(kmem, (char *)&word, sizeof(word)) != sizeof(word)) {
	fprintf(stderr, "error reading /dev/kmem at %x\n", loc);
	exit(1);
    }
    return (word);
}

off_t	kernel_proc_table;

int compat_proc_setup()
{
    struct nlist name_list[3];

    name_list[0].n_name = "_proc";
    name_list[1].n_name = "_nproc";
    name_list[2].n_name = "";

    nlist(name_list_file, name_list);
    if (name_list[0].n_type == 0) {
	fprintf(stderr, "'%s' No namelist.\n", name_list_file);
	exit(1);
    }

    if ((kmemf = open("/dev/kmem", 0)) < 0) {
	fprintf(stderr, "Error opening /dev/kmem.\n");
	exit(1);
    }

    kernel_proc_table = get_kernel_long(kmemf, (off_t)name_list[0].n_value);
    return ((int) get_kernel_long(kmemf, (off_t)name_list[1].n_value));
}

compat_get_procs(pbase, nproc)
	struct tbl_procinfo	*pbase;
	int			nproc;
{
#define	NPROC	16
    struct proc proc[NPROC];

    register int	i,j;
    register struct proc *mproc;

    struct user	u;

    off_t	procp = kernel_proc_table;

    for (i = 0; i < nproc; i += NPROC) {
	(void) lseek(kmemf, procp, L_SET);
	j = nproc - i;
	if (j > NPROC)
	    j = NPROC;

	j *= sizeof(struct proc);
	if (read(kmemf, (char *)proc, j) != j) {
	    fprintf(stderr, "Can't read the proc table from /dev/kmem.\n");
	    quit(1);
	}
	procp += j;

	for (j = j / sizeof(struct proc) - 1; j >= 0; j--) {
	    mproc = &proc[j];

	    if (mproc->p_stat == 0)
		pbase->pi_status = PI_EMPTY;
	    else {
		pbase->pi_uid = mproc->p_uid;
		pbase->pi_pid = mproc->p_pid;
		pbase->pi_ppid = mproc->p_ppid;
		pbase->pi_pgrp = mproc->p_pgrp;
		pbase->pi_flag = mproc->p_flag;

		if (mproc->task == 0)
		    pbase->pi_status = PI_ZOMBIE;
		else {
		    if (table(TBL_UAREA, mproc->p_pid,
			      (char *)&u, 1, sizeof(struct user))
			!= 1)
			pbase->pi_status = PI_ZOMBIE;
		    else {
			if (mproc->p_flag & SWEXIT)
			    pbase->pi_status = PI_EXITING;
			else
			    pbase->pi_status = PI_ACTIVE;
		    }
		}
		if (u.u_ttyp == 0)
		    pbase->pi_ttyd = NODEV;
		else
		    pbase->pi_ttyd = u.u_ttyd;

		strncpy(pbase->pi_comm, u.u_comm, sizeof(u.u_comm));
		pbase->pi_comm[sizeof(pbase->pi_comm)-1] = '\0';
	    }

	    pbase++;
	}
    }
}
#endif	PROC_COMPAT
