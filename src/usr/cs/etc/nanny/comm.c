#include "nanny.h"
#include <net/if.h>
/*
 *****************************************************************
 * HISTORY
 * $Log:	comm.c,v $
 * Revision 1.6  89/08/15  17:04:35  mja
 * 	Augmented getmyhostinfo() to fall back on obtaining an address
 * 	from the interface list if the gethostbyname() call fails
 * 	(which happens when running a kernel that does not return an
 * 	error when sending a message to a non-existent local socket).
 * 	[89/04/20            mja]
 * 
 * 29-Nov-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Support added for -host switch.
 *
 * 05-Aug-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to check and restart named.
 *
 * 30-Mar-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Cache the local host name and address information to improve
 *	performance. Pass around restart time information.
 *
 * 08-Nov-86  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Add some debugging code. Add more checks. Trying to understand 
 *	what is going on here.
 *
 */

int nanny_sock;
int anscount;
int reqcount;

/**** comm_init:
 *  Called to initialize the nanny_sock array. If called before, we
 * will assume that each of the elements in the array has been opened
 * and must now be shutdown.
 */
static
comm_init()
{
    static did = 0;

    if (did && nanny_sock >= 0)
	(void) close(nanny_sock);
    nanny_sock = -1;
    did = 1;
    reqcount = anscount = 0;
}

/*
 * load host name and address
 */
char myhostname[MAXHOSTNAMELEN] = "";
struct sockaddr_in myhostaddr;
struct sockaddr_in remoteaddr;
u_short nanny_port = 0;

getmyhostinfo()
{
    struct hostent *hent;
    struct servent *sent;
    char tmpname[MAXHOSTNAMELEN];

    myhostaddr.sin_family = AF_INET;

    if (*myhostname)
	return(0);

    if (gethostname(tmpname, sizeof(tmpname))) {
	print_err("Can't find out who we are!! %s\n", errmsg(errno));
	return (1);
    }

    folddown(tmpname, tmpname);
    if ((hent = gethostbyname(tmpname)) == NULL) {
	struct ifreq ifrv[100], *ifrp;
	struct ifconf ifc;
	int s;

	print_err("Can not find '%s' in the host table -- using interface list\n", tmpname);

	s = socket(myhostaddr.sin_family, SOCK_DGRAM, 0);
	if (s < 0) {
	    print_err("Can not create interface socket: %s\n", errmsg(-1));
	    return(1);
	}

	ifc.ifc_len = sizeof(ifrv);
	ifc.ifc_req = &ifrv[0];
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
	    print_err("Can not get interface configuration: %s\n", errmsg(-1));
	    return(1);
	}

	for (ifrp=ifc.ifc_req; ifc.ifc_len; ifc.ifc_len -= sizeof(*ifrp))
	{
	    if (ioctl(s, SIOCGIFADDR, (char *)ifrp) >= 0)
	    	break;
	    ifrp++;
	}
	if (ifc.ifc_len == 0) {
	    print_err("unable to locate interface address\n");
	    return(1);
	}
	bcopy(&ifrp->ifr_addr, (char *)&myhostaddr, sizeof(myhostaddr));
	print_err("%s %s\n", ifrp->ifr_name, inet_ntoa(myhostaddr.sin_addr));
	(void) close(s);
    }
    else
	bcopy(hent->h_addr, (char *)&myhostaddr.sin_addr, hent->h_length);

    if ((sent = getservbyname(NNYPORT, "tcp")) == NULL) {
	print_err("Unable to find entry for a port named '%s'\n", NNYPORT);
	exit(1);
    }
    nanny_port = ntohs((u_short)sent->s_port);

    (void)strcpy(myhostname,tmpname);

    return(0);
}

Check_Named()
{
    SERVER *srv;

    if ((srv = fetch_srv("named")) == NULL)
	return;
    if (!srv->valid || srv->status != RUNNING)
	return;
    if (!getmyhostinfo() && gethostbyname(myhostname) != NULL)
	return;
    restart_srv(srv);
}

/**** open_listen:
 *  Open the listening channel for the mother. We are placing this
 * on top of the TCP internet facility to help with reliability.
 */
open_listen()
{
    struct sockaddr_in addr;
    int sock;
    int on = 1;

    comm_init();

    if (getmyhostinfo()) {
	print_err("getmyhostinfo failed in open_listen\n");
	exit(1);
    }

    addr = myhostaddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nanny_port + MOMNANNY);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	print_err("socket failed for server: %s\n", errmsg(-1));
	exit(1);
    }
    (void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    (void)fcntl(sock, F_SETFL, FNDELAY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	print_err("bind failed for server: %s\n", errmsg(-1));
	exit(1);
    }

    remoteaddr = myhostaddr;
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_port = htons(nanny_port + REQNANNY);

    nanny_sock = sock;
}

/**** open_comm:
 *  open a connection to the server process.
 */
open_comm()
{
    struct sockaddr_in addr;
    int sock;
    int on = 1;

    comm_init();

    if (getmyhostinfo()) {
	print_err("getmyhostinfo failed for open_comm\n");
	exit(1);
    }

    addr = myhostaddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nanny_port + REQNANNY);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	print_err("socket failed for client: %s\n", errmsg(-1));
	exit(1);
    }
    (void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    (void)fcntl(sock, F_SETFL, FNDELAY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	print_err("bind failed for client: %s\n", errmsg(-1));
	exit(1);
    }

    remoteaddr = myhostaddr;
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_port = htons(nanny_port + MOMNANNY);

    if (host != NULL) {
	if (isdigit(*host)) {
	    remoteaddr.sin_addr.s_addr = inet_addr(host);
	} else {
	    struct hostent *hent = gethostbyname(host);

	    if (hent == NULL) {
		print_err("gethostbyname(%s) failed\n", host);
		exit(1);
	    }
	    bcopy(hent->h_addr, (char *)&remoteaddr.sin_addr, hent->h_length);
	}
    }

    nanny_sock = sock;
}

/* begin unpack routines */

/**** unpack_str:
 *   Parse out a counted string from the given location and adjust the
 * given pointer. The returned pointer points to internal static space.
 * It may be overwritten in future calls.
 */
char *unpack_str(ptr)
byte **ptr;
{
    static char     buff[MAXDATA];
    char           *tmp;
    int             len;

    len = *(*ptr)++;
    tmp = buff;
    while (len--)
	*tmp++ = *(*ptr)++;
    *tmp = 0;
    return (buff);
}

/**** unpack_int:
 * Unpack an integer.
 */
unpack_int(ptr)
byte **ptr;
{
    int     tmp = 0;
    int     size = sizeof(int);

    while (size--)
	tmp |= (*(*ptr)++) <<(size * 8);
    return(tmp);
}

display_basis(ptr)
SHORTSRV *ptr;
{
    char **list;

    printf("    Name: %s\n", ptr->name);
    printf("\tRequired file(s):\n");
    for(list = ptr->runfile;*list;list++)
	printf("\t\t\t  %s\n", *list);

    printf("\tStartup script:   ");
    for(list = ptr->script;*list;list++)
	printf("\"%s\" ", *list);
    printf("\n");

    printf("\tUser ID:          ");
    for(list = ptr->uid;*list;list++)
	printf("%s \n", *list);

    printf("\tGroup ID:         ");
    for(list = ptr->gid;*list;list++)
	printf("%s ", *list);
    printf("\n");

    printf("\tAttributes:       ");
    for(list = ptr->attributes;*list;list++)
	printf("%s ", *list);
    printf("\n");

    printf("\tNice: %d          \n", ptr->nice);
    printf("\n");
}

SHORTSRV *unpack_basis(ptr)
byte **ptr;
{
    char *tmp;
    SHORTSRV *basis;
    int cnt;
    char *ptrs[500];

    if (**ptr == 0)
	return (NULL);

    basis = NewBasis();

    if (tmp = unpack_str(ptr))
	basis->name = salloc(tmp);

    for(cnt = 0; *(tmp = unpack_str(ptr)) != '\0'; cnt++)
	ptrs[cnt] = salloc(tmp);
    ptrs[cnt] = NULL;
    basis->runfile = (char **) malloc((unsigned)sizeof(char *) * ++cnt);
    bcopy((char *)ptrs,(char *)basis->runfile,sizeof(char *) * cnt);

    for(cnt = 0; *(tmp = unpack_str(ptr)) != '\0'; cnt++)
	ptrs[cnt] = salloc(tmp);
    ptrs[cnt] = NULL;
    basis->script = (char **) malloc((unsigned)sizeof(char *) * ++cnt);
    bcopy((char *)ptrs,(char *)basis->script,sizeof(char *) * cnt);

    for(cnt = 0; *(tmp = unpack_str(ptr)) != '\0'; cnt++)
	ptrs[cnt] = salloc(tmp);
    ptrs[cnt] = NULL;
    basis->uid = (char **) malloc((unsigned)sizeof(char *) * ++cnt);
    bcopy((char *)ptrs,(char *)basis->uid,sizeof(char *) * cnt);

    for(cnt = 0; *(tmp = unpack_str(ptr)) != '\0'; cnt++)
	ptrs[cnt] = salloc(tmp);
    ptrs[cnt] = NULL;
    basis->gid = (char **) malloc((unsigned)sizeof(char *) * ++cnt);
    bcopy((char *)ptrs,(char *)basis->gid,sizeof(char *) * cnt);

    for(cnt = 0; *(tmp = unpack_str(ptr)) != '\0'; cnt++)
	ptrs[cnt] = salloc(tmp);
    ptrs[cnt] = NULL;
    basis->attributes = (char **) malloc((unsigned)sizeof(char *) * ++cnt);
    bcopy((char *)ptrs,(char *)basis->attributes,sizeof(char *) * cnt);

    basis->nice = unpack_int(ptr);
    return(basis);
}

/**** unpack_state:
 *  From the given buffer, unpack a status structure. Return a pointer to
 * the unpack structure. (static data) Adjust the given pointer to point 
 * just after the state info in the buffer.
 */
SERVER *unpack_state(b)
byte **b;
{
    static SERVER s;
    static SHORTSRV c;
    static int glist[100];
    int cnt;

    if (**b == 0)
	return(NULL);
    bzero((char *)&s,sizeof(s));
    bzero((char *)&c,sizeof(c));
    s.basis = &c;
    s.basis->name = unpack_str(b);
    s.status = unpack_int(b);
    s.pid = unpack_int(b);
    s.restarts = unpack_int(b);
    s.valid = unpack_int(b);
    s.uid = unpack_int(b);
    s.resttim = unpack_int(b);
    for(cnt=0;(glist[cnt] = unpack_int(b)) != 0;cnt++);
    s.gid = &glist[0];
    return(&s);
}

/* end unpack routines */


/* Begin pack routines */

/**** pack_str:
 *   Count the given null terminated string and place the counted string
 * into the give buffer. We include the null for the heck of it. Return a 
 * pointer to just after the inserted string.
 */
byte *pack_str(buff, from)
byte *buff;
char *from;
{
    byte  *len;

    len = buff++;
    *len = 0;
    if (from != NULL)
	while (*from) {
	    *buff++ = *from++;
	   (*len)++;
	}
    return(buff);
}

byte *pack_int(buff, num)
byte *buff;
int num;
{
    int     size = sizeof(int);

    while (size-- > 0)
	*buff++ = (num >>(size * 8)) & 0377;
    return(buff);
}

/**** pack_basis:
 *   Pack the given buffer with the given basis. All lists will be 
 * null string terminated.
 */
byte *pack_basis(buff, s)
byte *buff;
SHORTSRV *s;
{
    char  **tmp;

    buff = pack_str(buff, s->name);

    for (tmp = s->runfile; *tmp; tmp++)
	buff = pack_str(buff, *tmp);
    buff = pack_str(buff, (char *)NULL);

    for (tmp = s->script; *tmp; tmp++)
	buff = pack_str(buff, *tmp);
    buff = pack_str(buff, (char *)NULL);

    for (tmp = s->uid; *tmp; tmp++)
	buff = pack_str(buff, *tmp);
    buff = pack_str(buff, (char *)NULL);

    for (tmp = s->gid; *tmp; tmp++)
	buff = pack_str(buff, *tmp);
    buff = pack_str(buff, (char *)NULL);

    for (tmp = s->attributes; *tmp; tmp++)
	buff = pack_str(buff, *tmp);
    buff = pack_str(buff, (char *)NULL);

    return(pack_int(buff, s->nice));
}

/**** pack_state:
 *   Pack the important parts of the server structure into the given buffer.
 * Return a pointer to where we left off in the buffer.
 */
byte *pack_state(buff, s)
byte *buff;
SERVER *s;
{
    int *gp;

    buff = pack_str(buff,s->basis->name);
    buff = pack_int(buff,s->status);
    buff = pack_int(buff,s->pid);
    buff = pack_int(buff,s->restarts);
    buff = pack_int(buff,s->valid);
    buff = pack_int(buff,s->uid);
    buff = pack_int(buff,(int)s->resttim);
    for(gp = s->gid;*gp != 0;gp++)
	buff = pack_int(buff,*gp);
    *buff++ = 0;
    return(buff);
}

/* End pack routines */
