#include <netdb.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define MAXRETRY	30

#define AVAIL		0
#define ALLOC		1

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
#define	RECONFIG	0000100
#define	STATUS		0000200
#define	LIST		0000400
#define DISPLAY		0001000
#define WAIT		0002000

#define MAXDATA		255		 /* Real limit unknown for now */

#ifdef 	DEBUG
#define NNYPORT		"nannydbg"
#else	DEBUG
#define NNYPORT		"nanny"
#endif	DEBUG

/* structure for holding server status */
struct state {
    char           *name;
    int             state,
                    pid,
                    restarts,
                    valid;
};

/* structure of a comm */
struct comm {
    struct comm_head {
	int             len,
	                type;
    }               head;
#define	d_len	head.len
#define	d_type	head.type
    unsigned char   data[MAXDATA];
};

union datatype {
    char           *strptr;
    long           *lngptr;
    int            *intptr;
    struct state   *stateptr;
    SERVER         *srvptr;
    SHORTSRV       *baseptr;
};

/* Functions */
char *unpack_str();
int unpack_int();
short unpack_short();
struct state *unpack_state();
unsigned char *pack_str(),
	 *pack_int(),
	 *pack_short(),
	 *pack_basis(),
	 *pack_state();
