/**************
 * HISTORY
 * 29-Nov-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added host variable declaration.
 *
 * 05-Aug-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added named check every two minutes.
 *
 * 30-Mar-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Increase number of servers from 8 to 12 that each server nanny can
 *	handle. If each server needs a file for the log file and a file for
 *	the pty to the actual server then this takes a maximum of 24 files
 *	leaving 8 for stdin, stdout, stderr, configuration file. Also add
 *	a slot in the server structure for recording the last restart time...
 *	this information will allow us to slow down restarts of a totally
 *	busted server.
 *
 * 05-Nov-86  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Simplified some data types to increase code performance. Unsigned
 *	types are harder for C to handle. Removed declaractions that are
 *	defined elsewhere. Removed macros which make things less obvious
 *	to other programmers. Add definitions of sides of pipes to make
 *	things clearer.
 *
 */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <sys/vtimes.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <setjmp.h>
#include <ctype.h>
#include <libc.h>
#include <c.h>

typedef unsigned char byte;

/* Message types */
#define DONE		0010000
#define REQUEST		0020000
#define DATA		0040000
#define COMMERROR	0100000
#define TYPEMASK	0770000

#define	KILL		0000001
#define	RESTART		0000002
#define	STOP		0000004
#define	CONT		0000010
#define	DIE		0000020
#define	RECONFIG	0000040
#define	STATUS		0000100
#define	LIST		0000200
#define DISPLAY		0000400

#define MAXDATA		512

#define NNYPORT		"nanny"

/* various nanny defs */
#define REQNANNY 0
#define MOMNANNY 1

/* Timeouts and such */
#define MINUTES		60
#define HOUR		3600

/* random configuration parameters */
#define SRVDIR		"/tmp"
#define CNFGCHK		HOUR	/* how often we check the config file */
#define DPNDCHK		HOUR	/* how often we check for dependecies */
#define NAMEDCHK	(5*MINUTES) /* how often we check named */
#define RESTARTMAX	(15*MINUTES) /* how often we allow a restart */

/* Masks */
#define DIRMASK		007	 /* directory mode mask */
#define LOGMASK		037      /* mode for log file */

/* Dependancy files */
#define LOGPATH         "/usr/adm/log"
#define CNFGFIL 	"/usr/cs/etc/nanny.config"

/* Server modes */
#define RUNNING	1
#define DEAD	2
#define STOPPED	3
#define KILLED	4
#define ABORTED 5

/* basic information about a server - created from configuration file */

typedef struct {
    char *name;		/* Name that the server is known by */
    char **runfile;	/* The files this server is dependent on */
    char **script;	/* Broken down command line to start server */
    char **uid;		/* User ID that the server is to use */
    char **gid;		/* Group ID(s) that the server is to run under */
    char **attributes;	/* Or attributes needed on the current machine */
    int  nice;		/* Priority that the server is to run at */
} SHORTSRV;

/* running and calculated information about a server */

typedef struct server {
    struct server *Next, *Prev;	/* must be first two pointers */
    SHORTSRV *basis;
    long   tim;		/* Time that the dependancy files were last checked */
    int	   status; 	/* What is the state of this server? */
    int	   pid;		/* Process ID of the spawned server child */ 
    int    restarts;	/* times the server has been restarted */
    long   resttim;	/* last restart time -- too slow restarts */
    int    uid;		/* The user ID that the server is to run under */
    int   *gid;		/* The group ids the server is to run under */
    int    valid;	/* Should this server run on this system? */
} SERVER;

/* error number variable */
extern int  errno;

/* random global variables */
extern int	nanny_num;	/* The id of this nanny */
extern int	nanny_sock;	/* socket for nanny communication */
extern int	verbose;	/* verbose flag */
extern int	omask;		/* Mask that nanny inheritied */
extern int	dying;		/* true if shutting down */
extern int	reqcount;	/* count of requests */
extern int	anscount;	/* count of answers */

extern char	**attrib;	/* Attributes of this host */
extern char	*logdir;	/* Directory where log file is to be kept */
extern char	*config;  	/* Configuration file */
extern char	*host;  	/* Remote host */
extern SERVER	*SrvHead;	/* current list of servers */
extern char	myhostname[];	/* my host name */
extern struct sockaddr_in myhostaddr;	/* my host address */
extern struct sockaddr_in remoteaddr;	/* remote host address */
extern u_short  nanny_port;	/* base port number */

/* random global access routines */
extern char *Malloc();
extern char *Realloc();
extern long last_mod();
extern char *unpack_str();
extern int unpack_int();
extern char **get_attr();
extern SHORTSRV *unpack_basis();
extern SHORTSRV *NewBasis();
extern SERVER *unpack_state();
extern SERVER *fetch_srv();
extern SERVER *load();
extern SERVER *NewSrv();
extern byte *pack_str();
extern byte *pack_int();
extern byte *pack_basis();
extern byte *pack_state();

/* External definitions */
extern long time();
extern uid_t getuid(), geteuid();
extern char *inet_ntoa();
extern struct tm *localtime();

/* structure of a communication message sent between nannys */
struct comm {
    /* the header */
    struct comm_head {
	int len;
	int type;
    } head;
#define	d_len	head.len
#define	d_type	head.type
    /* the data */
    byte   data[MAXDATA-sizeof(struct comm_head)];
};

union datatype {
    char           *strptr;
    long           *lngptr;
    int            *intptr;
    SERVER         *srvptr;
    SHORTSRV       *baseptr;
};
