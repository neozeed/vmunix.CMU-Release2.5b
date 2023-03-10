/*
 * SUP Communication Module for 4.3 BSD
 *
 * SUP COMMUNICATION MODULE SPECIFICATIONS:
 *
 * IN THIS MODULE:
 *
 * CONNECTION ROUTINES
 *
 *   FOR SERVER
 *	servicesetup (port)	establish TCP port connection
 *	  char *port;			name of service
 *	service ()		accept TCP port connection
 *	servicekill ()		close TCP port in use by another process
 *	serviceprep ()		close temp ports used to make connection
 *	serviceend ()		close TCP port
 *
 *   FOR CLIENT
 *	request (port,hostname,retry) establish TCP port connection
 *	  char *port,*hostname;		  name of service and host
 *	  int retry;			  true if retries should be used
 *	requestend ()		close TCP port
 *
 * HOST NAME CHECKING
 *	p = remotehost ()	remote host name (if known)
 *	  char *p;
 *	i = samehost ()		whether remote host is also this host
 *	  int i;
 *	i = matchhost (name)	whether remote host is same as name
 *	  int i;
 *	  char *name;
 *
 * RETURN CODES
 *	All procedures return values as indicated above.  Other routines
 *	normally return SCMOK on success, SCMERR on error.
 *
 * COMMUNICATION PROTOCOL
 *
 *	Described in scmio.c.
 *
 **********************************************************************
 * HISTORY
 * $Log:	scm.c,v $
 * Revision 1.11  89/08/03  19:49:03  mja
 * 	Updated to use v*printf() in place of _doprnt().
 * 	[89/04/19            mja]
 * 
 * 11-Feb-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Moved sleep into computeBackoff, renamed to dobackoff.
 *
 * 10-Feb-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added timeout to backoff.
 *
 * 27-Dec-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed nameserver support.
 *
 * 09-Sep-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fixed to depend less upon having name of remote host.
 *
 * 25-May-87  Doug Philips (dwp) at Carnegie-Mellon Universtiy
 *	Extracted backoff/sleeptime computation from "request" and
 *	created "computeBackoff" so that I could use it in sup.c when
 *	trying to get to nameservers as a group.
 *
 * 21-May-87  Chriss Stephens (chriss) at Carnegie Mellon University
 *	Merged divergent CS and EE versions.
 *
 * 02-May-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added some bullet-proofing code around hostname calls.
 *
 * 31-Mar-87  Dan Nydick (dan) at Carnegie-Mellon University
 *	Fixed for 4.3.
 *
 * 30-May-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to use known values for well-known ports if they are
 *	not found in the host table.
 *
 * 19-Feb-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed setsockopt SO_REUSEADDR to be non-fatal.  Added fourth
 *	parameter as described in 4.3 manual entry.
 *
 * 15-Feb-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added call of readflush() to requestend() routine.
 *
 * 29-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Major rewrite for protocol version 4.  All read/write and crypt
 *	routines are now in scmio.c.
 *
 * 14-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added setsockopt SO_REUSEADDR call.
 *
 * 01-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Removed code to "gracefully" handle unexpected messages.  This
 *	seems reasonable since it didn't work anyway, and should be
 *	handled at a higher level anyway by adhering to protocol version
 *	number conventions.
 *
 * 26-Nov-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fixed scm.c to free space for remote host name when connection
 *	is closed.
 *
 * 07-Nov-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Fixed 4.2 retry code to reload sin values before retry.
 *
 * 22-Oct-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to retry initial connection open request.
 *
 * 22-Sep-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Merged 4.1 and 4.2 versions together.
 *
 * 21-Sep-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Add close() calls after pipe() call.
 *
 * 12-Jun-85  Steven Shafer (sas) at Carnegie-Mellon University
 *	Converted for 4.2 sockets; added serviceprep() routine.
 *
 * 04-Jun-85  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for 4.2 BSD.
 *
 **********************************************************************
 */

#include <libc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <varargs.h>
#include "sup.h"

extern int errno;

char scmversion[] = "4.3 BSD";

/*************************
 ***    M A C R O S    ***
 *************************/

/* networking parameters */
#define NCONNECTS 5

/*********************************************
 ***    G L O B A L   V A R I A B L E S    ***
 *********************************************/

extern char program[];			/* name of program we are running */
extern int progpid;			/* process id to display */

int netfile = -1;			/* network file descriptor */

static int sock = -1;			/* socket used to make connection */
static struct in_addr remoteaddr;	/* remote host address */
static char *remotename = NULL;		/* remote host name */
static int swapmode;			/* byte-swapping needed on server? */

/***************************************************
 ***    C O N N E C T I O N   R O U T I N E S    ***
 ***    F O R   S E R V E R                      ***
 ***************************************************/

servicesetup (server)		/* listen for clients */
char *server;
{
	struct sockaddr_in sin;
	struct servent *sp;
	short port;
	int one = 1;
	char *myhost ();

	if (myhost () == NULL)
		return (scmerr (-1,"Local hostname not known"));
	if ((sp = getservbyname(server,"tcp")) == 0) {
		if (strcmp(server, FILEPORT) == 0)
			port = htons((u_short)FILEPORTNUM);
		else
			return (scmerr (-1,"Can't find %s server description",server));
	} else
		port = sp->s_port;
	endservent ();
	sock = socket (AF_INET,SOCK_STREAM,0);
	if (sock < 0)
		return (scmerr (errno,"Can't create socket for connections"));
	if (setsockopt (sock,SOL_SOCKET,SO_REUSEADDR,(char *)&one,sizeof(int)) < 0)
		(void) scmerr (errno,"Can't set SO_REUSEADDR socket option");
	bzero ((char *)&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = port;
	if (bind (sock,&sin,sizeof(sin)) < 0)
		return (scmerr (errno,"Can't bind socket for connections"));
	if (listen (sock,NCONNECTS) < 0)
		return (scmerr (errno,"Can't listen on socket"));
	return (SCMOK);
}

service ()
{
	struct sockaddr_in from;
	int x,len;

	remotename = NULL;
	len = sizeof (from);
	do {
		netfile = accept (sock,&from,&len);
	} while (netfile < 0 && errno == EINTR);
	if (netfile < 0)
		return (scmerr (errno,"Can't accept connections"));
	remoteaddr = from.sin_addr;
	if (read(netfile,(char *)&x,sizeof(int)) != sizeof(int))
		return (scmerr (errno,"Can't transmit data on connection"));
	if (x == 0x01020304)
		swapmode = 0;
	else if (x == 0x04030201)
		swapmode = 1;
	else
		return (scmerr (-1,"Unexpected byteswap mode %x",x));
	return (SCMOK);
}

serviceprep ()		/* kill temp socket in daemon */
{
	if (sock >= 0) {
		(void) close (sock);
		sock = -1;
	}
	return (SCMOK);
}

servicekill ()		/* kill net file in daemon's parent */
{
	if (netfile >= 0) {
		(void) close (netfile);
		netfile = -1;
	}
	if (remotename) {
		free (remotename);
		remotename = NULL;
	}
	return (SCMOK);
}

serviceend ()		/* kill net file after use in daemon */
{
	if (netfile >= 0) {
		(void) close (netfile);
		netfile = -1;
	}
	if (remotename) {
		free (remotename);
		remotename = NULL;
	}
	return (SCMOK);
}

/***************************************************
 ***    C O N N E C T I O N   R O U T I N E S    ***
 ***    F O R   C L I E N T                      ***
 ***************************************************/

dobackoff (t,b)
int *t,*b;
{
	struct timeval tt;
	unsigned s;

	if (*t == 0)
		return (0);
	s = *b * 30;
	if (gettimeofday (&tt,(struct timezone *)NULL) >= 0)
		s += (tt.tv_usec >> 8) % s;
	if (*b < 32) *b <<= 1;
	if (*t != -1) {
		if (s > *t)
			s = *t;
		*t -= s;
	}
	(void) scmerr (-1,"Will retry in %d seconds",s);
	sleep (s);
	return (1);
}

request (server,hostname,retry)		/* connect to server */
char *server;
char *hostname;
int *retry;
{
	int x, backoff;
	struct hostent *h;
	struct servent *sp;
	struct sockaddr_in sin;
	short port;

	if ((sp = getservbyname(server,"tcp")) == 0) {
		if (strcmp(server, FILEPORT) == 0)
			port = htons((u_short)FILEPORTNUM);
		else
			return (scmerr (-1,"Can't find %s server description",
					server));
	} else
		port = sp->s_port;
	if ((h = gethostbyname (hostname)) == NULL)
		return (scmerr (-1,"Can't find host entry for %s",hostname));
	backoff = 1;
	for (;;) {
		netfile = socket (AF_INET,SOCK_STREAM,0);
		if (netfile < 0)
			return (scmerr (errno,"Can't create socket"));
		bzero ((char *)&sin,sizeof(sin));
		bcopy (h->h_addr,(char *)&sin.sin_addr,h->h_length);
		sin.sin_family = AF_INET;
		sin.sin_port = port;
		if (connect(netfile,&sin,sizeof(sin)) >= 0)
			break;
		(void) scmerr (errno,"Can't connect to server for %s",server);
		(void) close(netfile);
		if (!dobackoff (retry,&backoff))
			return (SCMERR);
	}
	remoteaddr = sin.sin_addr;
	remotename = salloc(h->h_name);
	x = 0x01020304;
	(void) write (netfile,(char *)&x,sizeof(int));
	swapmode = 0;		/* swap only on server, not client */
	return (SCMOK);
}

requestend ()			/* end connection to server */
{
	(void) readflush ();
	if (netfile >= 0) {
		(void) close (netfile);
		netfile = -1;
	}
	if (remotename) {
		free (remotename);
		remotename = NULL;
	}
	return (SCMOK);
}

/*************************************************
 ***    H O S T   N A M E   C H E C K I N G    ***
 *************************************************/

static
char *myhost ()		/* find my host name */
{
	struct hostent *h;
	static char name[256];

	if (name[0] != '\0')
		return (name);
	if (gethostname (name,256) < 0)
		return (NULL);
	if ((h = gethostbyname (name)) == NULL)
		return (NULL);
	(void) strcpy (name,h->h_name);
	return (name);
}

char *remotehost ()	/* remote host name (if known) */
{
	if (remotename == NULL)
		remotename = salloc(inet_ntoa(remoteaddr));
	if (remotename == NULL)
		return("UNKNOWN");
	return (remotename);
}

int thishost (host)
register char *host;
{
	register struct hostent *h;
	char *name, *myhost ();

	if ((name = myhost ()) == NULL)
		logquit (1,"Can't find my host entry");
	h = gethostbyname (host);
	if (h == NULL) return (0);
	return (strcasecmp (name,h->h_name) == 0);
}

int samehost ()		/* is remote host same as local host? */
{
	char *name;
	struct hostent *h;

	if ((name = myhost ()) == NULL)
		return (0);
	if (remotename == NULL) {
		h = gethostbyaddr ((char *)&remoteaddr,sizeof(remoteaddr),
				AF_INET);
		if (h == 0)
#if	CMUCS
			logquit (1,"Can't find remote host entry");
#else	CMUCS
			remotename = salloc (inet_ntoa(remoteaddr));
#endif	CMUCS
		else
			remotename = salloc (h->h_name);
	}
	return (strcasecmp (remotename,name) == 0);
}

int matchhost (name)	/* is this name of remote host? */
char *name;
{
	struct hostent *h;

	if (remotename == NULL) {
		h = gethostbyaddr ((char *)&remoteaddr,sizeof(remoteaddr),
				AF_INET);
		if (h == 0)
#if	CMUCS
			logquit (1,"Can't find remote host entry");
#else	CMUCS
			remotename = salloc (inet_ntoa(remoteaddr));
#endif	CMUCS
		else
			remotename = salloc (h->h_name);
	}
	if ((h = gethostbyname (name)) == 0)
		return (0);
	return (strcasecmp (remotename,h->h_name) == 0);
}

/* VARARGS2 */
int scmerr (errno,fmt,va_alist)
int errno;
char *fmt;
va_dcl
{
	va_list ap;

	(void) fflush (stdout);
	if (progpid > 0)
		fprintf (stderr,"%s %d: ",program,progpid);
	else
		fprintf (stderr,"%s: ",program);
	va_start(ap);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (errno >= 0)
		fprintf (stderr,": %s\n",errmsg(errno));
	else
		fprintf (stderr,"\n");
	(void) fflush (stderr);
	return (SCMERR);
}

/*******************************************************
 ***    I N T E G E R   B Y T E - S W A P P I N G    ***
 *******************************************************/

union intchar {
	int ui;
	char uc[sizeof(int)];
};

int byteswap (in)
int in;
{
	union intchar x,y;
	register int ix,iy;

	if (swapmode == 0)  return (in);
	x.ui = in;
	iy = sizeof(int);
	for (ix=0; ix<sizeof(int); ix++) {
		--iy;
		y.uc[iy] = x.uc[ix];
	}
	return (y.ui);
}
