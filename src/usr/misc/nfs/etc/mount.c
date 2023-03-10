/* @(#)mount.c	1.2 86/11/06 NFSSRC */
#ifndef lint
static char *sccsid = "@(#)mount.c	4.10 (Berkeley) 5/28/83";
#endif

/*
 * Copyright (c) 1985 Sun Microsystems, Inc.
 */

#define	NFS
/*
 * mount
 **********************************************************************
 * HISTORY
 * $Log:	mount.c,v $
 * Revision 2.1  89/07/19  09:10:30  dimitris
 * 	Changed to accept Ultrix type format for /etc/fstab.
 * 	[89/07/19  09:00:49  dimitris]
 * 
 * Revision 2.0  89/06/15  15:31:46  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *	Changed mount() to syscall(SYS_vfsmount,...) for CMUCS. Added
 *	include to sys/syscall.h
 *
 * 20-Feb-89  Paul Parker (parker) at Carnegie-Mellon University
 *	Merged CMUCS changes listed in edit history below into NFS
 *	version and added include for nfs_mount.h
 *
 * 06-Nov-80  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed to allow operator to mount file systems.
 *
 **********************************************************************
 */
#include <sys/param.h>
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <nfs/nfs.h>
#include <rpcsvc/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <mntent.h>
#include <sys/mount.h>
#if	CMUCS
#include <nfs/nfs_mount.h>
#include <sys/syscall.h>
#endif	/* CMUCS */

int	ro = 0;
int	quota = 0;
int	fake = 0;
int	freq = 1;
int	passno = 2;
int	all = 0;
int	verbose = 0;
int	printed = 0;


#define	NRETRY	10000	/* number of times to retry a mount request */
#define	BGSLEEP	5	/* initial sleep time for background mount in seconds */
#define MAXSLEEP 120	/* max sleep time for background mount in seconds */
/*
 * Fake errno for RPC failures that don't map to real errno's
 */
#define ERPC	(10000)

extern int errno;
#if CMUCS
char 	*convert();
#endif CMUCS
char	*index(), *rindex();
char	host[MNTMAXSTR];
char	name[MNTMAXSTR];
char	dir[MNTMAXSTR];
char	type[MNTMAXSTR];
char	opts[MNTMAXSTR];

/*
 * Structure used to build a mount tree.  The tree is traversed to do
 * the mounts and catch dependancies
 */
struct mnttree {
	struct mntent *mt_mnt;
	struct mnttree *mt_sib;
	struct mnttree *mt_kid;
};
struct mnttree *maketree();

main(argc, argv)
	int argc;
	char **argv;
{
	struct mntent mnt;
	struct mntent *mntp;
	FILE *mnttab;
	char *options;
	char *colon;
	struct mnttree *mtree;

#ifdef	CMUCS
	chkopr();
#endif
	if (argc == 1) {
		mnttab = setmntent(MOUNTED, "r");
		while ((mntp = getmntent(mnttab)) != NULL) {
#if CMUCS

			if (hasmntopt(mntp,MNTOPT_IGNORE)) {
#else CMUCS
			if (strcmp(mntp->mnt_type, MNTTYPE_IGNORE) == 0) {
#endif CMUCS
				continue;
			}
			printent(mntp);
		}
		endmntent(mnttab);
		exit(0);
	}

	close(2);
	if (dup2(1, 2) < 0) {
		perror("dup");
		exit(1);
	}

	opts[0] = '\0';
	type[0] = '\0';

	/*
	 * Set options
	 */
	while (argc > 1 && argv[1][0] == '-') {
		options = &argv[1][1];
		while (*options) {
			switch (*options) {
			case 'a':
				all++;
				break;
			case 'f':
				fake++;
				break;
			case 'o':
				if (argc < 3) {
					usage();
				}
				strcpy(opts, argv[2]);
				argv++;
				argc--;
				break;
			case 'p':
				if (argc != 2) {
					usage();
				}
				printmtab(stdout);
				exit(0);
			case 'q':
				quota++;
				break;
			case 'r':
				ro++;
				break;
			case 't':
				if (argc < 3) {
					usage();
				}
				strcpy(type, argv[2]);
				argv++;
				argc--;
				break;
			case 'v':
				verbose++;
				break;
			default:
				fprintf(stderr, "mount: unknown option: %c\n",
				    *options);
				usage();
			}
			options++;
		}
		argc--, argv++;
	}

	if (geteuid() != 0) {
		fprintf(stderr, "Must be root to use mount\n");
		exit(1);
	}

	if (all) {
		if (argc != 1) {
			usage();
		}
		mnttab = setmntent(MNTTAB, "r");
		if (mnttab == NULL) {
			fprintf(stderr, "mount: ");
			perror(MNTTAB);
			exit(1);
		}
		mtree = NULL;
		while ((mntp = getmntent(mnttab)) != NULL) {
#if CMUCS
			if (hasmntopt(mntp, MNTOPT_IGNORE) ||
			    hasmntopt(mntp, MNTOPT_SWAP) ||
#else CMUCS

			if ((strcmp(mntp->mnt_type, MNTTYPE_IGNORE) == 0) ||
			    (strcmp(mntp->mnt_type, MNTTYPE_SWAP) == 0) ||
#endif CMUCS

			    hasmntopt(mntp, MNTOPT_NOAUTO) ||
			    (strcmp(mntp->mnt_dir, "/") == 0) ) {
				continue;
			}
			if (type[0] != '\0' &&
			    strcmp(mntp->mnt_type, type) != 0) {
				continue;
			}
#if CMUCS
			strcpy(mntp->mnt_fsname,
				convert(mntp->mnt_fsname,"@",":"));
#endif CMUCS
			mtree = maketree(mtree, mntp);
		}
		endmntent(mnttab);
		mounttree(mtree);
		exit(0);
	}

	/*
	 * Command looks like: mount <dev>|<dir>
	 * we walk through /etc/fstab til we match either fsname or dir.
	 */
	if (argc == 2) {
		mnttab = setmntent(MNTTAB, "r");
		if (mnttab == NULL) {
			fprintf(stderr, "mount: ");
			perror(MNTTAB);
			exit(1);
		}
		while ((mntp = getmntent(mnttab)) != NULL) {
#if CMUCS

			if (hasmntopt(mntp, MNTOPT_IGNORE) ||
			    hasmntopt(mntp, MNTOPT_SWAP)) {
#else CMUCS
			if ((strcmp(mntp->mnt_type, MNTTYPE_IGNORE) == 0) ||
			    (strcmp(mntp->mnt_type, MNTTYPE_SWAP) == 0)) {
#endif CMUCS
				continue;
			}
			if ((strcmp(mntp->mnt_fsname, argv[1]) == 0) ||
#if CMUCS
			    /* See if they typed in the host:/dir format */
			    (strcmp(convert(mntp->mnt_fsname,"@",":"),
						       argv[1]) == 0) ||
#endif CMUCS
			    (strcmp(mntp->mnt_dir, argv[1]) == 0) ) {
#if CMUCS
				strcpy(mntp->mnt_fsname,
					convert(mntp->mnt_fsname,"@",":"));
#endif CMUCS
			     
				if (opts[0] != '\0') {
					char *topts;
					/*
					 * override fstab with command line
					 * options
					 */
					topts = mntp->mnt_opts;
					mntp->mnt_opts = opts;
					mounttree(maketree(NULL, mntp));
					mntp->mnt_opts = topts;
				} else {
					mounttree(maketree(NULL, mntp));
				}
				exit(0);
			}
		}
		fprintf(stderr, "mount: %s not found in %s\n", argv[1], MNTTAB);
		exit(0);
	}

	if (argc != 3) {
		usage();
	}
	strcpy(dir, argv[2]);
	strcpy(name, argv[1]);

	/*
	 * Check for file system names of the form
	 *     host:path
	 * make these type nfs
	 */
#if CMUCS
	if (!(colon = index(name, ':'))) {
	     if (colon=index(name,"@"))
		strcpy(name,convert(name,"@",":"));
	}
#else CMUCS
	colon = index(name, ':');
#endif CMUCS
	if (colon) {
		if (type[0] != '\0' && strcmp(type, "nfs") != 0) {
			fprintf(stderr,"%s: %s; must use type nfs\n",
			    "mount: remote file system", name);
			usage();
		}
		strcpy(type, MNTTYPE_NFS);
	}
	if (type[0] == '\0') {
		strcpy(type, MNTTYPE_42);		/* default type = 4.2 */
	}
	if (dir[0] != '/') {
		fprintf(stderr, "mount: directory path must begin with '/'\n");
		exit(1);
	}

	if (opts[0] == '\0') {
		strcpy(opts, ro ? MNTOPT_RO : MNTOPT_RW);
		if (strcmp(type, MNTTYPE_42) == 0) {
			strcat(opts, ",");
			strcat(opts, quota ? MNTOPT_QUOTA : MNTOPT_NOQUOTA);
		}
	}

	if (strcmp(type, MNTTYPE_NFS) == 0) {
		passno = 0;
		freq = 0;
	}

	mnt.mnt_fsname = name;
	mnt.mnt_dir = dir;
	mnt.mnt_type = type;
	mnt.mnt_opts = opts;
	mnt.mnt_freq = freq;
	mnt.mnt_passno = passno;
	mounttree(maketree(NULL, &mnt));
}

/*
 * attempt to mount file system, return errno or 0
 */
mountfs(print, mnt)
	int print;
	struct mntent *mnt;
{
	int error;
	extern int errno;
	int type = -1;
	int flags = 0;
	static char opts[1024];
	char *optp, *optend;
	union data {
		struct ufs_args	ufs_args;
		struct nfs_args nfs_args;
#ifdef PCFS
		struct pc_args pc_args;
#endif
	} data;

	if (mounted(mnt)) {
		if (print) {
			fprintf(stderr, "mount: %s already mounted\n",
			    mnt->mnt_fsname);
		}
		return (0);
	}
	if (fake) {
		addtomtab(mnt);
		return (0);
	}
	if (strcmp(mnt->mnt_type, MNTTYPE_42) == 0) {
		type = MOUNT_UFS;
		error = mount_42(mnt, &data.ufs_args);
	} else if (strcmp(mnt->mnt_type, MNTTYPE_NFS) == 0) {
		type = MOUNT_NFS;
		error = mount_nfs(mnt, &data.nfs_args);
#ifdef PCFS
	} else if (strcmp(mnt->mnt_type, MNTTYPE_PC) == 0) {
		type = MOUNT_PC;
		error = mount_pc(mnt, &data.pc_args);
#endif
	} else {
		fprintf(stderr,
		    "mount: unknown file system type %s\n",
		    mnt->mnt_type);
		error = EINVAL;
	}

	if (error) {
		return (error);
	}

	flags |= (hasmntopt(mnt, MNTOPT_RO) == NULL) ? 0 : M_RDONLY;
	flags |= (hasmntopt(mnt, MNTOPT_NOSUID) == NULL) ? 0 : M_NOSUID;
#if CMUCS
	if (syscall(SYS_vfsmount,type, mnt->mnt_dir, flags, &data) < 0) {
#else 
	if (mount(type, mnt->mnt_dir, flags, &data) < 0) {
#endif
		if (print) {
			fprintf(stderr, "mount: %s on ", mnt->mnt_fsname);
			perror(mnt->mnt_dir);
		}
		return (errno);
	}

	if ((optp = hasmntopt(mnt, MNTOPT_QUOTA)) != NULL) {
		/*
		 * cut out quota option and put in noquota option, for mtab
		 */
		optend = index(optp, ',');
		if (optp != mnt->mnt_opts)
			optp--;			/* back up to ',' */
		if (optend == NULL)
			*optp = '\0';
		else
			while (*optp++ = *optend++)
				;
		sprintf(opts, "%s,%s", mnt->mnt_opts, MNTOPT_NOQUOTA);
		mnt->mnt_opts = opts;
	}
	addtomtab(mnt);
	return (0);
}

mount_42(mnt, args)
	struct mntent *mnt;
	struct ufs_args *args;
{
	static char name[MNTMAXSTR];

	strcpy(name, mnt->mnt_fsname);
	args->fspec = name;
	return (0);
}

mount_nfs(mnt, args)
	struct mntent *mnt;
	struct nfs_args *args;
{
	static struct sockaddr_in sin;
	struct hostent *hp;
	static struct fhstatus fhs;
	char *cp;
	char *hostp = host;
	char *path;
	int s;
	struct timeval timeout;
	CLIENT *client;
	enum clnt_stat rpc_stat;
	int rsize, wsize;
	u_short port;

	cp = mnt->mnt_fsname;
	while ((*hostp = *cp) != ':') {
		if (*cp == '\0') {
			fprintf(stderr,
			    "mount: nfs file system; use host:path\n");
			return (1);
		}
		hostp++;
		cp++;
	}
	*hostp = '\0';
	path = ++cp;
	/*
	 * Get server's address
	 */
	if ((hp = gethostbyname(host)) == NULL) {
		/*
		 * XXX
		 * Failure may be due to yellow pages, try again
		 */
		if ((hp = gethostbyname(host)) == NULL) {
			fprintf(stderr,
			    "mount: %s not in hosts database\n", host);
			return (1);
		}
	}

	args->flags = 0;

	if (hasmntopt(mnt, MNTOPT_SOFT) != NULL) {
		args->flags |= NFSMNT_SOFT;
	}
	if (hasmntopt(mnt, MNTOPT_INTR) != NULL) {
		args->flags |= NFSMNT_INT;
	}

	/*
	 * get fhandle of remote path from server's mountd
	 */
	bzero(&sin, sizeof(sin));
	bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
	sin.sin_family = AF_INET;
	timeout.tv_usec = 0;
	timeout.tv_sec = 20;
	s = RPC_ANYSOCK;
	if ((client = clntudp_create(&sin, MOUNTPROG, MOUNTVERS,
	    timeout, &s)) == NULL) {
		if (!printed) {
			fprintf(stderr, "mount: %s server not responding",
			    mnt->mnt_fsname);
			clnt_pcreateerror("");
			printed = 1;
		}
		return (ETIMEDOUT);
	}
	if (! bindresvport(s)) {
		fprintf(stderr,"Warning: mount: cannot do local bind\n");
	}
	client->cl_auth = authunix_create_default();
	timeout.tv_usec = 0;
	timeout.tv_sec = 20;
	rpc_stat = clnt_call(client, MOUNTPROC_MNT, xdr_path, &path,
	    xdr_fhstatus, &fhs, timeout);
	errno = 0;
	if (rpc_stat != RPC_SUCCESS) {
		if (!printed) {
			fprintf(stderr, "mount: %s server not responding",
			    mnt->mnt_fsname);
			clnt_perror(client, "");
			printed = 1;
		}
		switch (rpc_stat) {
		case RPC_TIMEDOUT:
		case RPC_PMAPFAILURE:
		case RPC_PROGNOTREGISTERED:
			errno = ETIMEDOUT;
			break;
		case RPC_AUTHERROR:
			errno = EACCES;
			break;
		default:
			errno = ERPC;
			break;
		}
	}
	close(s);
	clnt_destroy(client);
	if (errno) {
		return(errno);
	}

	if (errno = fhs.fhs_status) {
		if (errno == EACCES) {
			fprintf(stderr, "mount: access denied for %s:%s\n",
			    host, path);
		} else {
			fprintf(stderr, "mount: ");
			perror(mnt->mnt_fsname);
		}
		return (errno);
	}
	if (printed) {
		fprintf(stderr, "mount: %s server ok\n", mnt->mnt_fsname);
		printed = 0;
	}

	/*
	 * set mount args
	 */
	args->fh = &fhs.fhs_fh;
	args->hostname = host;
	args->flags |= NFSMNT_HOSTNAME;
	if (args->rsize = nopt(mnt, "rsize")) {
		args->flags |= NFSMNT_RSIZE;
	}
	if (args->wsize = nopt(mnt, "wsize")) {
		args->flags |= NFSMNT_WSIZE;
	}
	if (args->timeo = nopt(mnt, "timeo")) {
		args->flags |= NFSMNT_TIMEO;
	}
	if (args->retrans = nopt(mnt, "retrans")) {
		args->flags |= NFSMNT_RETRANS;
	}
	if (port = nopt(mnt, "port")) {
		sin.sin_port = htons(port);
	} else {
		sin.sin_port = htons(NFS_PORT);	/* XXX should use portmapper */
	}
	args->addr = &sin;

	/*
	 * should clean up mnt ops to not contain defaults
	 */
	return (0);
}

#ifdef PCFS
mount_pc(mnt, args)
	struct mntent *mnt;
	struct pc_args *args;
{
	args->fspec = mnt->mnt_fsname;
	return (0);
}
#endif

printent(mnt)
	struct mntent *mnt;
{
	fprintf(stdout, "%s on %s type %s (%s)\n",
	    mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type, mnt->mnt_opts);
}

printmtab(outp)
	FILE *outp;
{
	FILE *mnttab;
	struct mntent *mntp;
	int maxfsname = 0;
	int maxdir = 0;
	int maxtype = 0;
	int maxopts = 0;

	/*
	 * first go through and find the max width of each field
	 */
	mnttab = setmntent(MOUNTED, "r");
	while ((mntp = getmntent(mnttab)) != NULL) {
		if (strlen(mntp->mnt_fsname) > maxfsname) {
			maxfsname = strlen(mntp->mnt_fsname);
		}
		if (strlen(mntp->mnt_dir) > maxdir) {
			maxdir = strlen(mntp->mnt_dir);
		}
		if (strlen(mntp->mnt_type) > maxtype) {
			maxtype = strlen(mntp->mnt_type);
		}
		if (strlen(mntp->mnt_opts) > maxopts) {
			maxopts = strlen(mntp->mnt_opts);
		}
	}
	endmntent(mnttab);

	/*
	 * now print them oput in pretty format
	 */
	mnttab = setmntent(MOUNTED, "r");
	while ((mntp = getmntent(mnttab)) != NULL) {
		fprintf(outp, "%-*s", maxfsname+1, mntp->mnt_fsname);
		fprintf(outp, "%-*s", maxdir+1, mntp->mnt_dir);
		fprintf(outp, "%-*s", maxtype+1, mntp->mnt_type);
		fprintf(outp, "%-*s", maxopts+1, mntp->mnt_opts);
		fprintf(outp, " %d %d\n", mntp->mnt_freq, mntp->mnt_passno);
	}
	endmntent(mnttab);
	return (0);
}

/*
 * Check to see if mntck is already mounted.
 * We have to be careful because getmntent modifies its static struct.
 */
mounted(mntck)
	struct mntent *mntck;
{
	int found = 0;
	struct mntent *mnt, mntsave;
	FILE *mnttab;

	mnttab = setmntent(MOUNTED, "r");
	if (mnttab == NULL) {
		fprintf(stderr, "mount: ");
		perror(MOUNTED);
		exit(1);
	}
	mntcp(mntck, &mntsave);
	while ((mnt = getmntent(mnttab)) != NULL) {
#if CMUCS

		if (hasmntopt(mnt, MNTOPT_IGNORE)) {
#else CMUCS
		if (strcmp(mnt->mnt_type, MNTTYPE_IGNORE) == 0) {
#endif CMUCS
			continue;
		}
		if ((strcmp(mntsave.mnt_fsname, mnt->mnt_fsname) == 0) &&
		    (strcmp(mntsave.mnt_dir, mnt->mnt_dir) == 0) ) {
			found = 1;
			break;
		}
	}
	endmntent(mnttab);
	*mntck = mntsave;
	return (found);
}

mntcp(mnt1, mnt2)
	struct mntent *mnt1, *mnt2;
{
	static char fsname[128], dir[128], type[128], opts[128];

	mnt2->mnt_fsname = fsname;
	strcpy(fsname, mnt1->mnt_fsname);
	mnt2->mnt_dir = dir;
	strcpy(dir, mnt1->mnt_dir);
	mnt2->mnt_type = type;
	strcpy(type, mnt1->mnt_type);
	mnt2->mnt_opts = opts;
	strcpy(opts, mnt1->mnt_opts);
	mnt2->mnt_freq = mnt1->mnt_freq;
	mnt2->mnt_passno = mnt1->mnt_passno;
}

/*
 * Return the value of a numeric option of the form foo=x, if
 * option is not found or is malformed, return 0.
 */
nopt(mnt, opt)
	struct mntent *mnt;
	char *opt;
{
	int val = 0;
	char *equal;
	char *str;


	if (str = hasmntopt(mnt, opt)) {
		if (equal = index(str, '=')) {
			val = atoi(&equal[1]);
		} else {
			fprintf(stderr, "mount: bad numeric option '%s'\n",
			    str);
		}
	}
	return (val);
}

/*
 * update /etc/mtab
 */
addtomtab(mnt)
	struct mntent *mnt;
{
	FILE *mnted;

	mnted = setmntent(MOUNTED, "r+");
	if (mnted == NULL) {
		fprintf(stderr, "mount: ");
		perror(MOUNTED);
		exit(1);
	}
#if CMUCS 
	/* write /etc/mtab the same way as /etc/fstab */
	
	if (strcmp(mnt->mnt_type,MNTTYPE_NFS) == 0)
	  if (index(mnt->mnt_fsname,':'))
		  strcpy(mnt->mnt_fsname,convert(mnt->mnt_fsname,":","@"));
#endif CMUCS
	if (addmntent(mnted, mnt)) {
		fprintf(stderr, "mount: ");
		perror(MOUNTED);
		exit(1);
	}
	endmntent(mnted);

	if (verbose) {
		fprintf(stdout, "%s mounted on %s\n",
		    mnt->mnt_fsname, mnt->mnt_dir);
	}
}

char *
xmalloc(size)
	int size;
{
	char *ret;
	
	if ((ret = (char *)malloc(size)) == NULL) {
		fprintf(stderr, "umount: ran out of memory!\n");
		exit(1);
	}
	return (ret);
}

struct mntent *
mntdup(mnt)
	struct mntent *mnt;
{
	struct mntent *new;

	new = (struct mntent *)xmalloc(sizeof(*new));

	new->mnt_fsname = (char *)xmalloc(strlen(mnt->mnt_fsname) + 1);
	strcpy(new->mnt_fsname, mnt->mnt_fsname);

	new->mnt_dir = (char *)xmalloc(strlen(mnt->mnt_dir) + 1);
	strcpy(new->mnt_dir, mnt->mnt_dir);

	new->mnt_type = (char *)xmalloc(strlen(mnt->mnt_type) + 1);
	strcpy(new->mnt_type, mnt->mnt_type);

	new->mnt_opts = (char *)xmalloc(strlen(mnt->mnt_opts) + 1);
	strcpy(new->mnt_opts, mnt->mnt_opts);

	new->mnt_freq = mnt->mnt_freq;
	new->mnt_passno = mnt->mnt_passno;

	return (new);
}

/*
 * Build the mount dependency tree
 */
struct mnttree *
maketree(mt, mnt)
	struct mnttree *mt;
	struct mntent *mnt;
{
	if (mt == NULL) {
		mt = (struct mnttree *)xmalloc(sizeof (struct mnttree));
		mt->mt_mnt = mntdup(mnt);
		mt->mt_sib = NULL;
		mt->mt_kid = NULL;
	} else {
		if (substr(mt->mt_mnt->mnt_dir, mnt->mnt_dir)) {
			mt->mt_kid = maketree(mt->mt_kid, mnt);
		} else {
			mt->mt_sib = maketree(mt->mt_sib, mnt);
		}
	}
	return (mt);
}

printtree(mt)
	struct mnttree *mt;
{
	if (mt) {
		printtree(mt->mt_sib);
		printf("   %s\n", mt->mt_mnt->mnt_dir);
		printtree(mt->mt_kid);
	}
}

mounttree(mt)
	struct mnttree *mt;
{
	int error;
	int slptime;
	int forked;
	int retry;
	int firsttry;

	if (mt) {
		mounttree(mt->mt_sib);
		forked = 0;
		printed = 0;
		firsttry = 1;
		slptime = BGSLEEP;
		retry = nopt(mt->mt_mnt, "retry");
		if (retry == 0) {
			retry = NRETRY;
		}

		do {
			error = mountfs(!forked, mt->mt_mnt);
			if (error != ETIMEDOUT && error != ENETDOWN &&
			    error != ENETUNREACH && error != ENOBUFS &&
			    error != ECONNREFUSED && error != ECONNABORTED) {
				break;
			}

			if (!forked && hasmntopt(mt->mt_mnt, "bg")) {
				fprintf(stderr, "mount: backgrounding\n");
				fprintf(stderr, "   %s\n", mt->mt_mnt->mnt_dir);
				printtree(mt->mt_kid);
				if (fork()) {
					return;
				} else {
					forked = 1;
				}
			}
			if (!forked && firsttry) {
				fprintf(stderr, "mount: retrying\n");
				fprintf(stderr, "   %s\n", mt->mt_mnt->mnt_dir);
				printtree(mt->mt_kid);
				firsttry = 0;
			}
			sleep(slptime);
			slptime = MIN(slptime << 1, MAXSLEEP);
		} while (retry--);

		if (!error) {
			mounttree(mt->mt_kid);
		} else {
			fprintf(stderr, "mount: giving up on:\n");
			fprintf(stderr, "   %s\n", mt->mt_mnt->mnt_dir);
			printtree(mt->mt_kid);
		}
		if (forked) {
			exit(0);
		}
	}
}

printsp(n)
	int n;
{
	while (n--) {
		printf(" ");
	}
}

/*
 * Returns true if s1 is a pathname substring of s2.
 */
substr(s1, s2)
	char *s1;
	char *s2;
{
	while (*s1 == *s2) {
		s1++;
		s2++;
	}
	if (*s1 == '\0' && *s2 == '/') {
		return (1);
	}
	return (0);
}

bindresvport(sd)
	int sd;
{
 
	u_short port;
	struct sockaddr_in sin;
	int err = -1;

#	define MAX_PRIV (IPPORT_RESERVED-1)
#	define MIN_PRIV (IPPORT_RESERVED/2)

	get_myaddress(&sin);
	sin.sin_family = AF_INET;
	for (port = MAX_PRIV; err && port >= MIN_PRIV; port--) {
		sin.sin_port = htons(port);
		err = bind(sd,&sin,sizeof(sin));
	}
	return (err == 0);
}
 

usage()
{
	fprintf(stdout,
	    "Usage: mount [-ravpfto [type|option]] ... [fsname] [dir]\n");
	exit(1);
}

