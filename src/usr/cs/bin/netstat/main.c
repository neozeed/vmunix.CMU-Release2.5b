/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)main.c	5.7 (Berkeley) 5/22/86";
#endif not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	main.c,v $
 * Revision 1.4  89/02/15  16:39:28  berman
 * 	Put in #if's to conditionalize inclusion of machine/rosetta.h 
 * 	for ibmrt and of machine/pte.h for other architectures.
 * 	[89/02/15            berman]
 * 
 * Revision 1.3  89/01/17  14:40:57  parker
 * 	Incorporated changes from Robert Baron (rvb):
 * 	Fix klseek for sun, rt, ...  What I have done isn't really
 * 	correct but will work until the kernel is > 16Meg and avoids
 * 	sucking in a myriad of strange include files.
 * 	[89/01/17            parker]
 * 
 * 29-May-87  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed to avoid referencing VAX page table fields in klseek()
 *	when not building for a VAX although this almost certainly
 *	won't work correctly in those cases yet.
 *
 * 20-Jun-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Merged changes (below) for 4.3; updated to use new vmvexec()
 *	routine.
 *
 * 11-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added setnetdbent(1) call to open binary database.
 *
 * 06-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added call to vmnlist() routine before calling nlist().
 *
 * 28-Sep-85  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added -H option to display ARP cache.
 *
 **********************************************************************
 */
#include <sys/param.h>
#include <sys/vmmac.h>
#include <sys/socket.h>
#if ibmrt
#include <machine/rosetta.h>
#else	/* ibmrt */
#include <machine/pte.h>
#endif	/* ibmrt */
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <nlist.h>
#include <stdio.h>

struct nlist nl[] = {
#define	N_MBSTAT	0
	{ "_mbstat" },
#define	N_IPSTAT	1
	{ "_ipstat" },
#define	N_TCB		2
	{ "_tcb" },
#define	N_TCPSTAT	3
	{ "_tcpstat" },
#define	N_UDB		4
	{ "_udb" },
#define	N_UDPSTAT	5
	{ "_udpstat" },
#define	N_RAWCB		6
	{ "_rawcb" },
#define	N_SYSMAP	7
	{ "_Sysmap" },
#define	N_SYSSIZE	8
	{ "_Syssize" },
#define	N_IFNET		9
	{ "_ifnet" },
#define	N_HOSTS		10
	{ "_hosts" },
#define	N_RTHOST	11
	{ "_rthost" },
#define	N_RTNET		12
	{ "_rtnet" },
#define	N_ICMPSTAT	13
	{ "_icmpstat" },
#define	N_RTSTAT	14
	{ "_rtstat" },
#define	N_NFILE		15
	{ "_nfile" },
#define	N_FILE		16
	{ "_file" },
#define	N_UNIXSW	17
	{ "_unixsw" },
#define N_RTHASHSIZE	18
	{ "_rthashsize" },
#define N_IDP		19
	{ "_nspcb"},
#define N_IDPSTAT	20
	{ "_idpstat"},
#define N_SPPSTAT	21
	{ "_spp_istat"},
#define N_NSERR		22
	{ "_ns_errstat"},
#if	CMUCS
#define	N_ARPTAB	23
	{ "_arptab" },
#define	N_ARPTAB_BSIZ	24
	{ "_arptab_bsiz" },
#define	N_ARPTAB_NB	25
	{ "_arptab_nb" },
#endif	CMUCS
	"",
};

/* internet protocols */
extern	int protopr();
extern	int tcp_stats(), udp_stats(), ip_stats(), icmp_stats();
extern	int nsprotopr();
extern	int spp_stats(), idp_stats(), nserr_stats();

struct protox {
	u_char	pr_index;		/* index into nlist of cb head */
	u_char	pr_sindex;		/* index into nlist of stat block */
	u_char	pr_wanted;		/* 1 if wanted, 0 otherwise */
	int	(*pr_cblocks)();	/* control blocks printing routine */
	int	(*pr_stats)();		/* statistics printing routine */
	char	*pr_name;		/* well-known name */
} protox[] = {
	{ N_TCB,	N_TCPSTAT,	1,	protopr,
	  tcp_stats,	"tcp" },
	{ N_UDB,	N_UDPSTAT,	1,	protopr,
	  udp_stats,	"udp" },
	{ -1,		N_IPSTAT,	1,	0,
	  ip_stats,	"ip" },
	{ -1,		N_ICMPSTAT,	1,	0,
	  icmp_stats,	"icmp" },
	{ -1,		-1,		0,	0,
	  0,		0 }
};

struct protox nsprotox[] = {
	{ N_IDP,	N_IDPSTAT,	1,	nsprotopr,
	  idp_stats,	"idp" },
	{ N_IDP,	N_SPPSTAT,	1,	nsprotopr,
	  spp_stats,	"spp" },
	{ -1,		N_NSERR,	1,	0,
	  nserr_stats,	"ns_err" },
	{ -1,		-1,		0,	0,
	  0,		0 }
};

struct	pte *Sysmap;

char	*system = "/vmunix";
char	*kmemf = "/dev/kmem";
int	kmem;
int	kflag;
int	Aflag;
int	aflag;
#if	CMUCS
int	Hflag;
#endif	CMUCS
int	hflag;
int	iflag;
int	mflag;
int	nflag;
int	rflag;
int	sflag;
int	tflag;
int	fflag;
int	interval;
char	*interface;
int	unit;
#if	CMUCS
char	usage[] = "[ -AHaihmnrstu ] [-f address_family] [-I interface] [ interval ] [ system ] [ core ]";
#else	CMUCS
char	usage[] = "[ -Aaihmnrst ] [-f address_family] [-I interface] [ interval ] [ system ] [ core ]";
#endif	CMUCS

#if	CMUCS
int	af = AF_INET;
#else	CMUCS
int	af = AF_UNSPEC;
#endif	CMUCS

main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	char *cp, *name;
	register struct protoent *p;
	register struct protox *tp;

	name = argv[0];
	argc--, argv++;
  	while (argc > 0 && **argv == '-') {
		for (cp = &argv[0][1]; *cp; cp++)
		switch(*cp) {

		case 'A':
			Aflag++;
			break;

		case 'a':
			aflag++;
			break;

#if	CMUCS
		case 'H':
			Hflag++;
			break;

#endif	CMUCS
		case 'h':
			hflag++;
			break;

		case 'i':
			iflag++;
			break;

		case 'm':
			mflag++;
			break;

		case 'n':
			nflag++;
			break;

		case 'r':
			rflag++;
			break;

		case 's':
			sflag++;
			break;

		case 't':
			tflag++;
			break;

		case 'u':
			af = AF_UNIX;
			break;

		case 'f':
			argv++;
			argc--;
			if (strcmp(*argv, "ns") == 0)
				af = AF_NS;
			else if (strcmp(*argv, "inet") == 0)
				af = AF_INET;
			else if (strcmp(*argv, "unix") == 0)
				af = AF_UNIX;
			else {
				fprintf(stderr, "%s: unknown address family\n",
					*argv);
				exit(10);
			}
			break;
			
		case 'I':
			iflag++;
			if (*(interface = cp + 1) == 0) {
				if ((interface = argv[1]) == 0)
					break;
				argv++;
				argc--;
			}
			for (cp = interface; isalpha(*cp); cp++)
				;
			unit = atoi(cp);
#if	CMUCS
			{
				int l = (cp-interface);
				char *ip = (char *)malloc(l+1);

				strncpy(ip, interface, l);
				cp = &ip[l];
				interface = ip;
			}
#endif	CMUCS
			*cp-- = 0;
			break;

		default:
use:
			printf("usage: %s %s\n", name, usage);
			exit(1);
		}
		argv++, argc--;
	}
	if (argc > 0 && isdigit(argv[0][0])) {
		interval = atoi(argv[0]);
		if (interval <= 0)
			goto use;
		argv++, argc--;
		iflag++;
	}
	if (argc > 0) {
		system = *argv;
		argv++, argc--;
	}
#if	CMUCS
	if (vmnlist(system, nl)) {
#endif	CMUCS
	nlist(system, nl);
	if (nl[0].n_type == 0) {
		fprintf(stderr, "%s: no namelist\n", system);
		exit(1);
	}
#if	CMUCS
	}
#endif	CMUCS
	if (argc > 0) {
		kmemf = *argv;
		kflag++;
	}
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		fprintf(stderr, "cannot open ");
		perror(kmemf);
		exit(1);
	}
	if (kflag) {
		off_t off;

		off = nl[N_SYSMAP].n_value & 0x7fffffff;
		lseek(kmem, off, 0);
		nl[N_SYSSIZE].n_value *= 4;
		Sysmap = (struct pte *)malloc(nl[N_SYSSIZE].n_value);
		if (Sysmap == 0) {
			perror("Sysmap");
			exit(1);
		}
		read(kmem, Sysmap, nl[N_SYSSIZE].n_value);
	}
	if (mflag) {
		mbpr(nl[N_MBSTAT].n_value);
		exit(0);
	}
	/*
	 * Keep file descriptors open to avoid overhead
	 * of open/close on each call to get* routines.
	 */
	sethostent(1);
	setnetent(1);
	if (iflag) {
		intpr(interval, nl[N_IFNET].n_value);
		exit(0);
	}
	if (hflag) {
		hostpr(nl[N_HOSTS].n_value);
		exit(0);
	}
#if	CMUCS
	if (Hflag) {
		arppr(nl[N_ARPTAB].n_value, nl[N_ARPTAB_BSIZ].n_value,
			nl[N_ARPTAB_NB].n_value);
		exit(0);
	}
#endif	CMUCS
	if (rflag) {
		if (sflag)
			rt_stats(nl[N_RTSTAT].n_value);
		else
			routepr(nl[N_RTHOST].n_value, nl[N_RTNET].n_value,
				nl[N_RTHASHSIZE].n_value);
		exit(0);
	}
    if (af == AF_INET || af == AF_UNSPEC) {
	setprotoent(1);
	setservent(1);
	while (p = getprotoent()) {

		for (tp = protox; tp->pr_name; tp++)
			if (strcmp(tp->pr_name, p->p_name) == 0)
				break;
		if (tp->pr_name == 0 || tp->pr_wanted == 0)
			continue;
		if (sflag) {
			if (tp->pr_stats)
				(*tp->pr_stats)(nl[tp->pr_sindex].n_value,
					p->p_name);
		} else
			if (tp->pr_cblocks)
				(*tp->pr_cblocks)(nl[tp->pr_index].n_value,
					p->p_name);
	}
	endprotoent();
    }
    if (af == AF_NS || af == AF_UNSPEC) {
	for (tp = nsprotox; tp->pr_name; tp++) {
		if (sflag) {
			if (tp->pr_stats)
				(*tp->pr_stats)(nl[tp->pr_sindex].n_value,
					tp->pr_name);
		} else
			if (tp->pr_cblocks)
				(*tp->pr_cblocks)(nl[tp->pr_index].n_value,
					tp->pr_name);
	}
    }
    if ((af == AF_UNIX || af == AF_UNSPEC) && !sflag)
	    unixpr(nl[N_NFILE].n_value, nl[N_FILE].n_value,
		nl[N_UNIXSW].n_value);
    exit(0);
}

/*
 * Seek into the kernel for a value.
 */
klseek(fd, base, off)
	int fd, base, off;
{

	if (kflag) {
		/* get kernel pte */
#ifdef vax
		base &= 0x7fffffff;
#else
		base &= 0x0fffffff;
#endif
#if	CMUCS && !defined(vax)
		base &= 0xfffffff;	/* XXX */
#else	CMUCS && !defined(vax)
		base = ctob(Sysmap[btop(base)].pg_pfnum) + (base & PGOFSET);
#endif	CMUCS && !defined(vax)
	}
	lseek(fd, base, off);
}

char *
plural(n)
	int n;
{

	return (n != 1 ? "s" : "");
}
