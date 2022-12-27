/*
 **********************************************************************
 * HISTORY
 * 29-Nov-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Support added for -host switch.
 *
 * 27-Aug-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed to ignore bad requests, not suicide...
 *
 * 30-Mar-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Treat EOF as EOF not as an error when doing non-blocking recv() call 
 *	and for the sake of tracking errors, print out a message but not as
 *	scary.
 *
 * 21-Oct-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added missing argument to gettimeofday() call.
 *
 **********************************************************************
 */
#include "nanny.h"

static struct comm rcv_buf;

/**** rcv_comm:
 *   Read the specified socket and fill in a comm structure. This
 * consists of reading the comm header from the socket, then the
 * specified data from there.
 */
rcv_comm(sock)
int sock;
{
    int size, len;
    struct sockaddr_in from;
    int remote = 0;

    if (verbose)
	print_log("rcv_comm for fd%d\n", sock);

    /* read request */
    size = sizeof(from);
    len = recvfrom(sock, (char *)&rcv_buf, sizeof(rcv_buf), 0, &from, &size);
    if (len <= 0) {
	print_log("rcv_comm recv failed fd%d: %s\n",sock,errmsg(errno));
	return;
    } else if (len != rcv_buf.d_len) {
	print_log("rcv_comm recv'd %d when expecting %d bytes\n",
		  len,rcv_buf.d_len);
	return;
    }

    if (bcmp((char *)&from, (char *)&remoteaddr, sizeof(from)) != 0)
	remote++;

    if (verbose)
	print_log("Recieved type=%6o\n", rcv_buf.d_type);

    /* process the beast */
    switch ((int)(rcv_buf.d_type & TYPEMASK)) {
	case COMMERROR:
	    rcv_error(&rcv_buf);
	    break;
	case DONE:
	    anscount++;
	    break;
	case REQUEST:
	    rcv_req(&rcv_buf, &from, remote);
	    break;
	case DATA:
	    rcv_data(&rcv_buf);
	    break;
	default:
	    print_err("Unknown comm type given type=%o\n", rcv_buf.d_type);
	    break;
    }

    if ((rcv_buf.d_type & TYPEMASK) != DONE)
	    snd_done(&from);
}

/**** rcv_error:
 *   If and when a nanny encounter odd data from another nanny, it sends an
 * error comm. The comm is equivelent to a done but the data is interpreted
 * as a string and printed.
 */
rcv_error(comm)
struct comm *comm;
{
    byte  *ptr;

    ptr = comm->data;
    print_err("%s\n", unpack_str(&ptr));
}

/**** rcv_data:
 *   Data usually comes to the mother from the nanny servers or to the user
 * nanny from the mother nanny. Deal with each of these cases. The regular
 * nannys should never recieve data comms.
 *   It should be noted that the mother rarely hands off data from the 
 * nanny servers to the user process. It usually stores the data internally
 * for later reference.
 */
rcv_data(comm)
struct comm *comm;
{
    SERVER         *srv;
    byte  *ptr;
    SHORTSRV	   *basptr;

    ptr = comm->data;
    switch (nanny_num) {
	case MOMNANNY:
	    print_err("MOTHER is not supposed to recieve %d type comms\n",
		    comm->d_type);
	    break;
	case REQNANNY:
	    switch ((int)(comm->d_type & ~TYPEMASK)) {
		case STATUS:
		    srv = unpack_state(&ptr);
		    print_state(srv);
		    break;
		case LIST:
		    printf("\t%s\n", unpack_str(&ptr));
		    break;
		case DISPLAY:
		    basptr = unpack_basis(&ptr);
		    display_basis(basptr);
		    FreeBasis(basptr);
		    break;
		default:
		    break;
	    }
	    break;
    }
}

/**** rcv_req:
 *   Requests all have two things in common, they all must go through the 
 * mother nanny at some point, and the nanny servers never make one.
 *   The mother nanny just passes server specific requests on to the implied
 * nanny server. Otherwise, it deals with the request itself.
 */
rcv_req(comm, from, remote)
struct comm *comm;
struct sockaddr *from;
int remote;
{
    SERVER *srv;
    byte *ptr;
    char *tmp;

    ptr = comm->data;
    switch (nanny_num) {
	case MOMNANNY:
	    switch ((int)(comm->d_type & ~TYPEMASK)) {
		    /* We take care of these right away */
		case KILL:
		    print_log("Recieved kill request\n");
		    if (remote)
			snd_err("Illegal remote request\n", from);
		    else if (*(tmp = unpack_str(&ptr)) == '\0')
			snd_err("No server specified\n", from);
		    else if ((srv = fetch_srv(tmp)) == NULL)
			snd_err("No such server\n", from);
		    else
			kill_srv(srv);
		    break;
		case RESTART:
		    print_log("Recieved restart request\n");
		    if (remote)
			snd_err("Illegal remote request\n", from);
		    else if (*(tmp = unpack_str(&ptr)) == '\0')
			snd_err("No server specified\n", from);
		    else if ((srv = fetch_srv(tmp)) == NULL)
			snd_err("No such server\n", from);
		    else
			restart_srv(srv);
		    break;
		case CONT:
		    print_log("Recieved continue request\n");
		    if (remote)
			snd_err("Illegal remote request\n", from);
		    else if (*(tmp = unpack_str(&ptr)) == '\0')
			snd_err("No server specified\n", from);
		    else if ((srv = fetch_srv(tmp)) == NULL)
			snd_err("No such server\n", from);
		    else
			cont_srv(srv);
		    break;
		case STOP:
		    print_log("Recieved stop request\n");
		    if (remote)
			snd_err("Illegal remote request\n", from);
		    else if (*(tmp = unpack_str(&ptr)) == '\0')
			snd_err("No server specified\n", from);
		    else if ((srv = fetch_srv(tmp)) == NULL)
			snd_err("No such server\n", from);
		    else
			stop_srv(srv);
		    break;
		case STATUS:
		    print_log("Recieved status request\n");
		    tmp = unpack_str(&ptr);
		    if (*tmp) {
			if ((srv = fetch_srv(tmp)) == NULL) {
			    print_err("No such server %s\n", tmp);
			    snd_err("No such server\n", from);
			} else
			    snd_status(srv, from);
		    }
		    else {
			if ((srv=SrvHead) != NULL) do {
			    snd_status(srv, from);
			} while ((srv=srv->Next) != SrvHead);
		    }
		    break;
		case LIST:
		    print_log("Recieved list request\n");
		    if ((srv=SrvHead) != NULL) do {
			if (srv->valid)
			    snd_list(srv->basis->name, from);
		    } while ((srv=srv->Next) != SrvHead);
		    break;
		case DISPLAY:
		    print_log("Recieved display request\n");
		    if (*(tmp = unpack_str(&ptr)) == NULL)
			print_log("No server specified to be DISPLAYed\n");
		    if (*tmp) {
			if ((srv = fetch_srv(tmp)) == NULL)
			    snd_err("No such server\n", from);
			else
			    snd_dsp(srv->basis, from);
		    }
		    else
			snd_err("No server specified\n", from);
		    break;
		    /* Dealt with internally, with potential side effects */
		case DIE:
		    print_log("Recieved Suicide request\n");
		    if (remote) {
			snd_err("Illegal remote request\n", from);
			break;
		    }
		    dying++;
		    if ((srv=SrvHead) != NULL) do {
			if (srv->valid)
			    kill_srv(srv);
		    } while ((srv=srv->Next) != SrvHead);
		    break;
		case RECONFIG:
		    print_log("Recieved reconfiguration request\n");
		    if (remote)
			snd_err("Illegal remote request\n", from);
		    else
			Check_cnfg(unpack_str(&ptr));
		    break;
		default:
		    print_err("Unknown request type. type=%o\n", comm->d_type);
		    break;
	    }
	    break;
	case REQNANNY:
	    print_err("Bad request type=%o\n", comm->d_type);
	    break;
    }
}
