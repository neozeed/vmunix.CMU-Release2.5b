/*****************
 * HISTORY
 * 29-Nov-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Support added for -host switch.
 *
 * 30-Mar-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	A request to kill a server clears the restart time.
 *
 */
#include "nanny.h"

static struct comm snd_buf;

/**** do_send:
 *  The actual sends are done here. Since everything is in the global 
 * send buffer, just push it to its destination.
 */
do_send(to)
struct sockaddr *to;
{

    if (verbose)
	print_log("Sending:  type=%6o\n", snd_buf.d_type);

    snd_buf.d_len += sizeof(snd_buf.head);
    if (sendto(nanny_sock, (char *) &snd_buf, snd_buf.d_len, 0,
		to, sizeof(struct sockaddr)) <= 0) {
	switch (errno) {
	    case ECONNREFUSED:
		print_err("Nanny not responding!\n");
		break;
	    case ENOBUFS:
		print_err("System ran out of buffers!\n");
		break;
	    default:
		print_err("Error in sending comm type=%o err=%s\n", 
			  snd_buf.d_type, errmsg(errno));
		break;
	}
    }
    else if ((snd_buf.d_type&DONE) == 0 && (snd_buf.d_type&COMMERROR) == 0) {
	reqcount++;
    }
}

req_kill(name)
char *name;
{
    snd_buf.d_type = REQUEST | KILL;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

req_restart(name)
char *name;
{
    snd_buf.d_type = REQUEST | RESTART;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

req_stop(name)
char *name;
{
    snd_buf.d_type = REQUEST | STOP;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

req_cont(name)
char *name;
{
    snd_buf.d_type = REQUEST | CONT;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

req_status(name)
char *name;
{
    snd_buf.d_type = REQUEST | STATUS;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

req_display(name)
char *name;
{
    snd_buf.d_type = REQUEST | DISPLAY;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

req_recon(file)
char *file;
{
    snd_buf.d_type = REQUEST | RECONFIG;
    snd_buf.d_len = pack_str(snd_buf.data, file) - snd_buf.data;
    do_send(&remoteaddr);
}

req_die()
{
    snd_buf.d_type = REQUEST | DIE;
    snd_buf.d_len = 0;
    do_send(&remoteaddr);
}

req_list(name)
char *name;
{
    snd_buf.d_type = REQUEST | LIST;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(&remoteaddr);
}

/**** snd_err:
 *   We do not send error messages yet. When we do, they will be 
 * an error string.
 */
snd_err(str, to)
char *str;
struct sockaddr *to;
{
    snd_buf.d_type = COMMERROR;
    snd_buf.d_len = pack_str(snd_buf.data, str) - snd_buf.data;
    do_send(to);
}

snd_status(srv, to)
SERVER *srv;
struct sockaddr *to;
{
    snd_buf.d_type = DATA | STATUS;
    snd_buf.d_len = pack_state(snd_buf.data, srv) - snd_buf.data;
    do_send(to);
}

snd_list(name, to)
char *name;
struct sockaddr *to;
{
    snd_buf.d_type = DATA | LIST;
    snd_buf.d_len = pack_str(snd_buf.data, name) - snd_buf.data;
    do_send(to);
}

snd_dsp(basis, to)
SHORTSRV *basis;
struct sockaddr *to;
{
    snd_buf.d_type = DATA | DISPLAY;
    snd_buf.d_len = pack_basis(snd_buf.data, basis) - snd_buf.data;
    do_send(to);
}

/**** snd_done:
 *   Send a "done" comm. These are sent on a first come first done
 * assumption.
 */
snd_done(to)
struct sockaddr *to;
{
    snd_buf.d_type = DONE;
    snd_buf.d_len = 0;
    do_send(to);
}

/*
 * common kill server code
 */
com_kill_srv(srv,newstat)
SERVER *srv;
{
    if (!srv->valid) {
	print_log("Attempt to kill invalid server %s\n",srv->basis->name);
	return;
    }
    srv->resttim = 0;
    if (srv->status != RUNNING && srv->status != STOPPED ) {
	srv->status = newstat;
	return;
    }
    print_log("Killing server %s (pid=%d)\n", srv->basis->name, srv->pid);
    if (srv->pid < 3) {
	srv->status = newstat;
	print_err("Process ID was %d. Signal not sent\n", srv->pid);
	return;
    }
    if (kill(srv->pid, SIGKILL) < 0)
	print_err("kill (SIGKILL) failed for pid %d - %s\n",srv->pid,errmsg(errno));

    srv->status = newstat;
}

/**** kill_srv:
 *   Kill off the named server by sending a kill -9 to its PID. Inform the
 * mother nanny of what we did.
 */
kill_srv(srv)
SERVER *srv;
{
    com_kill_srv(srv,KILLED);	/* marked as manually killed */
}

/**** restart_srv:
 *   Attempt to restart the given server. We will not restart servers that 
 * should not run on this host.
 */
restart_srv(srv)
SERVER *srv;
{
    com_kill_srv(srv,DEAD);	/* marked as restartable */
}

/**** cont_srv:
 *  This will only work on those servers that are already stopped.
 */
cont_srv(srv)
SERVER *srv;
{
    if (!srv->valid) {
	print_log("Attempt to kill invalid server %s\n",srv->basis->name);
	return;
    }
    if (srv->status != STOPPED) {
	print_log("Attempt to kill continue unstopped server %s\n",srv->basis->name);
	return;
    }
    if (verbose)
	print_log("Continuing server %s (pid=%d)\n",srv->basis->name,srv->pid);
    if (kill(srv->pid, SIGCONT) < 0) {
	print_err("kill (SIGCONT) failed for pid %d - %s\n",srv->pid,errmsg(errno));
	return;
    }
    /* record status as being unstopped */
    srv->status = RUNNING;
}

/**** stop_srv:
 *  We only stop running servers.
 */
stop_srv(srv)
SERVER *srv;
{
    if (!srv->valid) {
	print_log("Attempt to kill invalid server %s\n",srv->basis->name);
	return;
    }
    if (srv->status != RUNNING) {
	print_log("Attempt to kill stop not running server %s\n",srv->basis->name);
	return;
    }
    if (verbose)
	print_log("Stopped server %s (pid=%d)\n",srv->basis->name,srv->pid);
    if (kill(srv->pid, SIGSTOP) < 0) {
	print_err("kill (SIGSTOP) failed for pid %d - %s\n",srv->pid,errmsg(errno));
	return;
    }
    /* record status as being stopped */
    srv->status = STOPPED;
}

