#include "nanny.h"

/**** Alarm:
 *  A catch all for all alarm signals that do nothing. 
 */
Alarm()
{
}

/**** set_pending:
 *  Set the pending death flag and return.
 */
set_pending() 
{
    pending_death = 1;
}				/* set_pending */

/**** Sleep:
 *   To sleep in nanny, we schedule the wakeup in the future and sigpause
 * until the alloted time.
 */
Sleep(t)
time_t t;
{
    struct timeval  tv,
                    now;

    if (t <= 0)
	return;
    (void)gettimeofday(&tv, (struct timezone *)0);
    tv.tv_sec += t;
    set_call(&tv, Alarm);
    do {
	sigpause(0);
	(void)gettimeofday(&now, (struct timezone *)0);
    } while (timercmp(&now, &tv, <));
}

/**** setsigs:
 *   This is an attempt to centralize all of the signal management.
 */
setsigs(func)
int func;
{
    struct sigvec   tmp;
    unsigned        holdmask;
    int             do_call(), exit();

    switch (func) {
	case INIT:
	    init_sig();
	    tmp.sv_handler = SIG_IGN;
	    tmp.sv_mask = ~0;		    /* Block everything in handler! */
	    tmp.sv_onstack = 0;
	    /*
	     * sigvec(SIGHUP, &tmp, 0); sigvec(SIGQUIT, &tmp, 0); sigvec(SIGINT, &tmp, 0);
	     * sigvec(SIGTSTP, &tmp, 0); sigvec(SIGIO, &tmp, 0); 
	     */
	    tmp.sv_handler = set_pending;
	    (void)sigvec(SIGCHLD, &tmp, (struct sigvec *)0);
	    tmp.sv_handler = do_call;
	    (void)sigvec(SIGALRM, &tmp, (struct sigvec *)0);
	    tmp.sv_handler = exit;
	    (void)sigvec(SIGURG, &tmp, (struct sigvec *)0);
	    break;
	case HOLD:
	    holdmask = (1 << SIGQUIT);
	    (void)sigblock((int)holdmask);
	    break;
	case RELEASE:
	    (void)sigsetmask(0);	/* do not block anything */
	    break;

	case RESET:
	    tmp.sv_handler = SIG_DFL;
	    tmp.sv_mask = 0;
	    tmp.sv_onstack = 0;
	    (void)sigvec(SIGHUP, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGQUIT, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGINT, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGTSTP, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGURG, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGIO, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGCHLD, &tmp, (struct sigvec *)0);
	    (void)sigvec(SIGALRM, &tmp, (struct sigvec *)0);
	    break;
    }
}

#define	MAXPEND	50			/* Max number of pending alarms */
static 	sig_init = 0;			/* signals initialized flag */
static	struct ptable {
	struct timeval tv;
	int (*func)();
} ptable[MAXPEND];			/* table of pending alarms */
static	struct sigstack s_stack;

/**** init_sig:
 *   Set up the ptable, allocate the signal stack, and inform the system of 
 * the new stack.
 */
init_sig()
{
    int     i;

    for (i = 0; i < MAXPEND; i++)
	timerclear(&ptable[i].tv);
    s_stack.ss_sp = (caddr_t) Malloc(1024);
    (void)sigstack(&s_stack, (struct sigstack *)0);
    sig_init = 1;
}

/**** set_call:
 *   Enter a new call into the pending interupt table. This is a critical
 * section so all signals are blocked till we are done.
 */
set_call(when, f)
struct timeval *when;
int (*f)();
{
    struct timeval  now;
    int     i;

    if (!sig_init)
	print_err("dreadful error\n");
    (void)gettimeofday(&now, (struct timezone *)0);
    setsigs(HOLD);
    for (i = 0; i < MAXPEND; i++) {
	if (!timerisset(&ptable[i].tv)) {
	    ptable[i].tv = *when;
	    ptable[i].func = f;
	    set_alrm();
	    setsigs(RELEASE);
	    return;
	}
    }
    setsigs(RELEASE);
    print_err("Pending signal buffer full!!\n");
    print_err("BYE\n");
    exit(1);
/* NOTREACHED */
}

clear_call(when, f)
struct timeval *when;
int (*f)();
{
    int     j;

    for (j = 0; j < MAXPEND; j++)
	if (timercmp(&ptable[j].tv, when, ==) && ptable[j].func == f) {
	    timerclear(&ptable[j].tv);
	    set_alrm();
	    return;
	}
}

set_alrm()
{
    struct timeval  now;
    struct itimerval    it;
    int     i;

    (void)gettimeofday(&now, (struct timezone *)0);
    timerclear(&it.it_interval);
    timerclear(&it.it_value);
    for (i = 0; i < MAXPEND; i++)
	if (timerisset(&ptable[i].tv)) {
	    it.it_value = ptable[i].tv;
	    break;
	}
    for (; i < MAXPEND; i++)
	if (timerisset(&ptable[i].tv) && timercmp(&ptable[i].tv, &it.it_value, <))
	    it.it_value = ptable[i].tv;

    if (timerisset(&it.it_value))
	if (timercmp(&now, &it.it_value, <)) {/* still in future */
	    it.it_value.tv_sec -= now.tv_sec;
	    it.it_value.tv_usec += 1;
	}
	else {				    /* time is past due */
	    it.it_value.tv_sec = 0;
	    it.it_value.tv_usec = 10;
	}
    (void)setitimer(ITIMER_REAL, &it, (struct itimerval *)0);
}

/**** do_call:
 *   This should only be called when an ALARM signal occures. We should 
 * take the signal on the private signal stack allocate earlier. We will
 * assume that all other signals are blocked while we are working. 
 */
do_call()
{
    struct timeval  now;
    struct ptable  *pt;
    int     i;

    if (!sig_init) {
	print_err("do_call called before initialized\n");
	return;
    }
    for (;;) {
	(void)gettimeofday(&now, (struct timezone *)0);
	for (pt = NULL, i = 0; i < MAXPEND; i++)
	    if (timerisset(&ptable[i].tv) && timercmp(&ptable[i].tv, &now, <)) {
		pt = &ptable[i];
		break;
	    }
	if (!pt)
	    break;
	for (; i < MAXPEND; i++)
	    if (timerisset(&ptable[i].tv) && timercmp(&ptable[i].tv, &pt->tv, <)) {
		pt = &ptable[i];
	    }
	timerclear(&pt->tv);
	(*pt->func)();
    }
    set_alrm();
}

