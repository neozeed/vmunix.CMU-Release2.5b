/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)pstat.c	5.8 (Berkeley) 5/5/86";
#endif not lint

/*
 * Print system stuff
 **********************************************************************
 * HISTORY
 *  9-Jul-89  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Eliminated use of task structure as utask pointer is now kept
 *	in proc stucture. Changed to use new user area identity structure.
 *	Reduced all conditionals to MACH or CMU.
 *	Removed support for -K(kernelmap) and -S(swapmap) which were
 *	CMU added features that no longer work with Mach kernels.
 *
 * $Log:	pstat.c,v $
 * Revision 1.8  89/02/02  16:23:49  gm0w
 * 	Added local file types to pstat -f.
 * 	[89/02/02            gm0w]
 * 
 * Revision 1.7  89/01/22  12:49:24  gm0w
 * 	Added code to find process with a particular open file pointer.
 * 	Removed CSFE code.
 * 	[89/01/22            gm0w]
 * 
 * 20-Apr-88  Mary Thompson (mrt) at Carnegie Mellon
 *	Changed all MACH_VM and MACH_TT conditionals  to
 *	(MACH ) and ( MACH_TT || MACH )
 *
 * 19-Jan-88  Mike Accetta (mja) at Carnegie-Mellon University
 *	Correct domap() conditional to be compiled without MACH_VM.
 *
 * 20-Jul-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Remove references to p_wchan under MACH_TT.
 *
 * 25-Mar-87  William Bolosky (bolosky) at Carnegie-Mellon University
 *	Made to work with MACH_VM.  Many of the bogus old Berkeley VM
 *	stuff is turned off as it makes no sense.  -K and -S also gone.
 *	This program should really be merged in with vmmprint, vmoprint,
 *	zprint, etc.
 *
 * 17-Nov-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Modified to work on the Sun under Mach.
 *
 * 26-Mar-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed to use new vmnlist() routine to speed symbol lookup;
 *	modified to omit uninitialized terminals from -t display;  
 *	upgraded from 4.1; modified for easier separate compilation
 *	of our local features while integrating changes (below):
 *
 * 15-Dec-83  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added -S switch to print swap map and combined -K to share most
 *	of the code.
 *
 * 02-Dec-81  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added "-c" option to display callout list.
 *
 * 16-Nov-81  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added -K to examine kernelmap.
 *
 **********************************************************************
 */

#define mask(x) (x&0377)
#define	clear(x) ((int)x&0x7fffffff)

#if	MACH
#define	SHOW_UTT 1
#endif	MACH
#include <sys/param.h>
#include <sys/dir.h>
#if	MACH
#include <sys/callout.h>
#define	KERNEL_FILE
#else	MACH
#define	KERNEL
#endif	MACH
#include <sys/file.h>
#if	MACH
#undef	KERNEL_FILE
#else	MACH
#undef	KERNEL
#endif	MACH
#include <sys/user.h>
#include <sys/proc.h>
#if	MACH
#else 	MACH
#include <sys/text.h>
#endif	MACH
#include <sys/inode.h>
#include <sys/map.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vm.h>
#include <nlist.h>
#if	MACH && !defined(vax)
/* don't include this on non-vaxen with Mach virtual memory */
#else	MACH && !defined(vax)
#include <machine/pte.h>
#endif	MACH && !defined(vax)
#include <stdio.h>

char	*fcore	= "/dev/kmem";
char	*fmem	= "/dev/mem";
char	*fnlist	= "/vmunix";
int	fc, fm;

struct nlist nl[] = {
#define	SINODE	0
	{ "_inode" },
#if	MACH
#define	SPROC	1
	{ "_proc" },
#define	SDZ	2
	{ "_dz_tty" },
#define	SNDZ	3
	{ "_dz_cnt" },
#define	SKL	4
	{ "_cons" },
#define	SFIL	5
	{ "_file" },
#define	SDH	6
	{ "_dh11" },
#define	SNDH	7
	{ "_ndh11" },
#define	SNPROC	8
	{ "_nproc" },
#define	SNFILE	9
	{ "_nfile" },
#define	SNINODE	10
	{ "_ninode" },
#define	SPTY	11
	{ "_pt_tty" },
#define	SDMF	12
	{ "_dmf_tty" },
#define	SNDMF	13
	{ "_ndmf" },
#define	SNPTY	14
	{ "_npty" },
#define	SDHU	15
	{ "_dhu_tty" },
#define	SNDHU	16
	{ "_ndhu" },
#define	SYSMAP	17
	{ "_Sysmap" },
#define	SDMZ	18
	{ "_dmz_tty" },
#define	SNDMZ	19
	{ "_ndmz" },
#define	SFETAB	20
	{ "_fetab" },
#define SFET11	21
	{ "_fet11" },
#define SFETTY	22
	{ "_fe_tty" },
#define SNFETTY	23
	{ "_nfetty" },
#define SCMUPTY	24
	{ "_cmupty_tty" },
#define SNCMUPTY 25
	{ "_ncmupty" },
#define SCALLOUT 26
	{ "_callout" },
#define SNCALLOUT 27
	{ "_ncallout" },
#define SCALLTODO 28
	{ "_calltodo" },
#if	( MACH )
#else	( MACH )
#define	STEXT	29
	{ "_text" },
#define	USRPTMA	30
	{ "_Usrptmap" },
#define	USRPT	31
	{ "_usrpt" },
#define	SWAPMAP	32
	{ "_swapmap" },
#define	SNTEXT	33
	{ "_ntext" },
#define	SNSWAPMAP 34
	{ "_nswapmap" },
#define	SDMMIN	35
	{ "_dmmin" },
#define	SDMMAX	36
	{ "_dmmax" },
#define	SNSWDEV	37
	{ "_nswdev" },
#define	SSWDEVT	38
	{ "_swdevt" },
#define SKMAP	39
	{ "_kernelmap" },
#endif	( MACH )
#else	MACH
#define	STEXT	1
	{ "_text" },
#define	SPROC	2
	{ "_proc" },
#define	SDZ	3
	{ "_dz_tty" },
#define	SNDZ	4
	{ "_dz_cnt" },
#define	SKL	5
	{ "_cons" },
#define	SFIL	6
	{ "_file" },
#define	USRPTMA	7
	{ "_Usrptmap" },
#define	USRPT	8
	{ "_usrpt" },
#define	SWAPMAP	9
	{ "_swapmap" },
#define	SDH	10
	{ "_dh11" },
#define	SNDH	11
	{ "_ndh11" },
#define	SNPROC	12
	{ "_nproc" },
#define	SNTEXT	13
	{ "_ntext" },
#define	SNFILE	14
	{ "_nfile" },
#define	SNINODE	15
	{ "_ninode" },
#define	SNSWAPMAP 16
	{ "_nswapmap" },
#define	SPTY	17
	{ "_pt_tty" },
#define	SDMMIN	18
	{ "_dmmin" },
#define	SDMMAX	19
	{ "_dmmax" },
#define	SNSWDEV	20
	{ "_nswdev" },
#define	SSWDEVT	21
	{ "_swdevt" },
#define	SDMF	22
	{ "_dmf_tty" },
#define	SNDMF	23
	{ "_ndmf" },
#define	SNPTY	24
	{ "_npty" },
#define	SDHU	25
	{ "_dhu_tty" },
#define	SNDHU	26
	{ "_ndhu" },
#define	SYSMAP	27
	{ "_Sysmap" },
#define	SDMZ	28
	{ "_dmz_tty" },
#define	SNDMZ	29
	{ "_ndmz" },
#endif	MACH
	{ "" }
};

int	inof;
int	prcf;
int	ttyf;
int	usrf;
long	ubase;
int	filf;
int	totflg;
char	partab[1];
struct	cdevsw	cdevsw[1];
struct	bdevsw	bdevsw[1];
int	allflg;
int	kflg;
u_long	getw();
off_t	mkphys();
#if	MACH
int	filef;		/* -F flag */
long	chkfile;
int	callf;		/* -c flag */
#else	MACH
int	txtf;
int	swpf;
struct	pte *Usrptma;
struct	pte *usrpt;
#endif	MACH


main(argc, argv)
char **argv;
{
	register char *argp;
	int allflags;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		argp = *argv++;
		argp++;
		argc--;
		while (*argp++)
		switch (argp[-1]) {

		case 'T':
			totflg++;
			break;

		case 'a':
			allflg++;
			break;

		case 'i':
			inof++;
			break;

		case 'k':
			kflg++;
			fcore = fmem = "/vmcore";
			break;

		case 'x':
#if	MACH
			printf("In Mach, there's no text table...ignoring '-x'.\n");
#else	MACH
			txtf++;
#endif	MACH
			break;

		case 'p':
			prcf++;
			break;

		case 't':
			ttyf++;
			break;

		case 'u':
			if (argc == 0)
				break;
			argc--;
			usrf++;
			sscanf( *argv++, "%x", &ubase);
			break;

		case 'f':
			filf++;
			break;
		case 's':
#if	MACH
			printf("There's no swapping in Mach.  '-s' ignored.\n");
#else	MACH
			swpf++;
#endif	MACH
			break;
#if	MACH
		case 'F':
			if (argc == 0)
				break;
			argc--;
			filef++;
			sscanf( *argv++, "%x", &chkfile);
			break;

		case 'c':
			callf++;
			break;
#endif	MACH
		default:
			usage();
			exit(1);
		}
	}
	if (argc>1) {
		fcore = fmem = argv[1];
		kflg++;
	}
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if ((fm = open(fmem, 0)) < 0) {
		printf("Can't find %s\n", fmem);
		exit(1);
	}
	if (argc>0)
		fnlist = argv[0];
#if	MACH
	if (vmnlist(fnlist, nl))
#endif	MACH
	nlist(fnlist, nl);
#if	MACH
#else	MACH
	usrpt = (struct pte *)nl[USRPT].n_value;
	Usrptma = (struct pte *)nl[USRPTMA].n_value;
#endif	MACH
	if (nl[0].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}
#if	MACH
	allflags = filf | totflg | inof | prcf | ttyf | usrf | callf | filef;
#else	MACH
	allflags = filf | totflg | inof | prcf | txtf | ttyf | usrf | swpf | filef;
#endif	MACH

	if (allflags == 0) {
#if	MACH
		printf("pstat: one or more of -[aiptfucFT] is required\n");
#else	MACH
		printf("pstat: one or more of -[aixptfsuT] is required\n");
#endif	MACH
		exit(1);
	}
	if (filf||totflg)
		dofile();
	if (inof||totflg)
		doinode();
#if	MACH
	if (prcf||filef||totflg)
#else	MACH
	if (prcf||totflg)
#endif	MACH
		doproc();
#if	MACH
#else	MACH
	if (txtf||totflg)
		dotext();
#endif	MACH
	if (ttyf)
		dotty();
	if (usrf)
		dousr();
#if	MACH
#else	MACH
	if (swpf||totflg)
		doswap();
#endif	MACH
#if	MACH
	if (callf||totflg)
		docallout();
#endif	MACH
}

usage()
{

#if	MACH
	printf("usage: pstat -[aiptfscTF] [-u [ubase]] [system] [core]\n");
#else	MACH
	printf("usage: pstat -[aixptfsT] [-u [ubase]] [system] [core]\n");
#endif	MACH
}

#if	MACH
#define	CALLADDR(loc)	((struct callout *)((caddr_t)(loc)-(caddr_t)acall+(caddr_t)xcall))

docallout()
{
	struct callout *xcall, *acall, *calltodo;
	int ncallout;
	register struct callout *cp;
	register loc, nc;

	ncallout = getw(nl[SNCALLOUT].n_value);
	calltodo = (struct callout *)getw(&((struct callout *)(nl[SCALLTODO].n_value))->c_next);
	xcall = (struct callout *)calloc(ncallout, sizeof (struct callout));
	lseek(fc, (int)(acall = (struct callout *)getw(nl[SCALLOUT].n_value)), 0);
	read(fc, xcall, ncallout * sizeof (struct callout));
	nc = 0;
	for (cp=calltodo; cp; cp=CALLADDR(cp)->c_next)
		nc++;
	if (totflg) {
		printf("%3d/%3d callouts\n", nc, ncallout);
		return;
	}
	printf("%d/%d callouts\n", nc, ncallout);
	printf(" TIME   FUNC      ARG\n");
	for (cp=calltodo; cp; cp=CALLADDR(cp)->c_next)
	    printf("%5d %8x %8x\n", CALLADDR(cp)->c_time,
		   CALLADDR(cp)->c_func, CALLADDR(cp)->c_arg);

}
#endif	MACH

doinode()
{
	register struct inode *ip;
	struct inode *xinode, *ainode;
	register int nin;
	int ninode;

	nin = 0;
	ninode = getw(nl[SNINODE].n_value);
	xinode = (struct inode *)calloc(ninode, sizeof (struct inode));
	ainode = (struct inode *)getw(nl[SINODE].n_value);
	if (ninode < 0 || ninode > 10000) {
		fprintf(stderr, "number of inodes is preposterous (%d)\n",
			ninode);
		return;
	}
	if (xinode == NULL) {
		fprintf(stderr, "can't allocate memory for inode table\n");
		return;
	}
	lseek(fc, mkphys((off_t)ainode), 0);
	read(fc, xinode, ninode * sizeof(struct inode));
	for (ip = xinode; ip < &xinode[ninode]; ip++)
		if (ip->i_count)
			nin++;
	if (totflg) {
		printf("%3d/%3d inodes\n", nin, ninode);
		return;
	}
	printf("%d/%d active inodes\n", nin, ninode);
printf("   LOC      FLAGS    CNT DEVICE  RDC WRC  INO  MODE  NLK UID   SIZE/DEV\n");
	for (ip = xinode; ip < &xinode[ninode]; ip++) {
		if (ip->i_count == 0)
			continue;
		printf("%8.1x ", ainode + (ip - xinode));
		putf(ip->i_flag&ILOCKED, 'L');
		putf(ip->i_flag&IUPD, 'U');
		putf(ip->i_flag&IACC, 'A');
		putf(ip->i_flag&IMOUNT, 'M');
		putf(ip->i_flag&IWANT, 'W');
		putf(ip->i_flag&ITEXT, 'T');
		putf(ip->i_flag&ICHG, 'C');
		putf(ip->i_flag&ISHLOCK, 'S');
		putf(ip->i_flag&IEXLOCK, 'E');
		putf(ip->i_flag&ILWAIT, 'Z');
		printf("%4d", ip->i_count&0377);
		printf("%4d,%3d", major(ip->i_dev), minor(ip->i_dev));
		printf("%4d", ip->i_shlockc&0377);
		printf("%4d", ip->i_exlockc&0377);
		printf("%6d", ip->i_number);
		printf("%6x", ip->i_mode & 0xffff);
		printf("%4d", ip->i_nlink);
		printf("%4d", ip->i_uid);
		if ((ip->i_mode&IFMT)==IFBLK || (ip->i_mode&IFMT)==IFCHR)
			printf("%6d,%3d", major(ip->i_rdev), minor(ip->i_rdev));
		else
			printf("%10ld", ip->i_size);
		printf("\n");
	}
	free(xinode);
}

u_long
getw(loc)
	off_t loc;
{
	u_long word;

	if (kflg)
		loc &= 0x7fffffff;
	lseek(fc, loc, 0);
	read(fc, &word, sizeof (word));
	return (word);
}

putf(v, n)
{
	if (v)
		printf("%c", n);
	else
		printf(" ");
}

#if	MACH
#else	MACH
dotext()
{
	register struct text *xp;
	int ntext;
	struct text *xtext, *atext;
	int ntx, ntxca;

	ntx = ntxca = 0;
	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	atext = (struct text *)getw(nl[STEXT].n_value);
	if (ntext < 0 || ntext > 10000) {
		fprintf(stderr, "number of texts is preposterous (%d)\n",
			ntext);
		return;
	}
	if (xtext == NULL) {
		fprintf(stderr, "can't allocate memory for text table\n");
		return;
	}
	lseek(fc, mkphys((off_t)atext), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_iptr != NULL)
			ntxca++;
		if (xp->x_count != 0)
			ntx++;
	}
	if (totflg) {
		printf("%3d/%3d texts active, %3d used\n", ntx, ntext, ntxca);
		return;
	}
	printf("%d/%d active texts, %d used\n", ntx, ntext, ntxca);
	printf("\
   LOC   FLAGS DADDR     CADDR  RSS SIZE     IPTR   CNT CCNT      FORW     BACK\n");
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		printf("%8.1x", atext + (xp - xtext));
		printf(" ");
		putf(xp->x_flag&XPAGI, 'P');
		putf(xp->x_flag&XTRC, 'T');
		putf(xp->x_flag&XWRIT, 'W');
		putf(xp->x_flag&XLOAD, 'L');
		putf(xp->x_flag&XLOCK, 'K');
		putf(xp->x_flag&XWANT, 'w');
		printf("%5x", xp->x_daddr[0]);
		printf("%10x", xp->x_caddr);
		printf("%5d", xp->x_rssize);
		printf("%5d", xp->x_size);
		printf("%10.1x", xp->x_iptr);
		printf("%5d", xp->x_count&0377);
		printf("%5d", xp->x_ccount);
		printf("%10x", xp->x_forw);
		printf("%9x", xp->x_back);
		printf("\n");
	}
	free(xtext);
}
#endif	MACH

doproc()
{
	struct proc *xproc, *aproc;
	int nproc;
	register struct proc *pp;
	register loc, np;
#if	MACH
#else	MACH
	struct pte apte;
#endif	MACH

	nproc = getw(nl[SNPROC].n_value);
	xproc = (struct proc *)calloc(nproc, sizeof (struct proc));
	aproc = (struct proc *)getw(nl[SPROC].n_value);
	if (nproc < 0 || nproc > 10000) {
		fprintf(stderr, "number of procs is preposterous (%d)\n",
			nproc);
		return;
	}
	if (xproc == NULL) {
		fprintf(stderr, "can't allocate memory for proc table\n");
		return;
	}
	lseek(fc, mkphys((off_t)aproc), 0);
	read(fc, xproc, nproc * sizeof (struct proc));
	np = 0;
	for (pp=xproc; pp < &xproc[nproc]; pp++)
		if (pp->p_stat)
			np++;
	if (totflg) {
		printf("%3d/%3d processes\n", np, nproc);
		return;
	}
	printf("%d/%d processes\n", np, nproc);
#if	MACH
	printf("   LOC    S    F PRI      SIG  UID SLP TIM  CPU  NI   PGRP    PID   PPID      LINK\n");
#else	MACH 
	printf("   LOC    S    F POIP PRI      SIG  UID SLP TIM  CPU  NI   PGRP    PID   PPID    ADDR   RSS SRSS SIZE    WCHAN    LINK   TEXTP\n");
#endif	MACH
	for (pp=xproc; pp<&xproc[nproc]; pp++) {
		if (pp->p_stat==0 && allflg==0)
			continue;
#if	MACH
		if (filef) {
/*		    struct task task; */
		    struct utask utask;
		    int i;

/*		    lseek(fc, (long)(pp->task), 0);
		    read(fc, &task, sizeof(struct task));
		    lseek(fc, (long)(task.u_address), 0);
*/
		    lseek(fc, (long)(pp->utask), 0);
		    read(fc, &utask, sizeof(struct utask));
		    for (i = 0; i < NOFILE; i++)
			if ((long)(utask.uu_ofile[i]) == chkfile)
			    break;
		    if (i == NOFILE)
			continue;
		}
#endif	MACH
		printf("%8x", aproc + (pp - xproc));
		printf(" %2d", pp->p_stat);
		printf(" %4x", pp->p_flag & 0xffff);
#if	MACH
#else	MACH
		printf(" %4d", pp->p_poip);
#endif	MACH
		printf(" %3d", pp->p_pri);
		printf(" %8x", pp->p_sig);
		printf(" %4d", pp->p_uid);
		printf(" %3d", pp->p_slptime);
		printf(" %3d", pp->p_time);
		printf(" %4d", pp->p_cpu&0377);
		printf(" %3d", pp->p_nice);
		printf(" %6d", pp->p_pgrp);
		printf(" %6d", pp->p_pid);
		printf(" %6d", pp->p_ppid);
#if	MACH
		printf(" %8x", clear(pp->p_link));
#else	MACH
		if (kflg)
			pp->p_addr = (struct pte *)clear((int)pp->p_addr);
		if (pp->p_flag & SLOAD) {
			lseek(fc, (long)pp->p_addr, 0);
			read(fc, &apte, sizeof(apte));
			printf(" %8x", apte.pg_pfnum);
		} else
			printf(" %8x", pp->p_swaddr);
		printf(" %4x", pp->p_rssize);
		printf(" %4x", pp->p_swrss);
		printf(" %5x", pp->p_dsize+pp->p_ssize);
		printf(" %7x", clear(pp->p_wchan));
		printf(" %7x", clear(pp->p_link));
		printf(" %7x", clear(pp->p_textp));
#endif	MACH
		printf("\n");
	}
	free(xproc);
}

static char mesg[] =
" # RAW CAN OUT     MODE     ADDR DEL COL     STATE  PGRP DISC\n";
static int ttyspace = 128;
static struct tty *tty;

dotty()
{
	extern char *malloc();

	if ((tty = (struct tty *)malloc(ttyspace * sizeof(*tty))) == 0) {
		printf("pstat: out of memory\n");
		return;
	}
	printf("1 cons\n");
	if (kflg)
		nl[SKL].n_value = clear(nl[SKL].n_value);
	lseek(fc, (long)nl[SKL].n_value, 0);
	read(fc, tty, sizeof(*tty));
	printf(mesg);
	ttyprt(&tty[0], 0);
	if (nl[SNDZ].n_type != 0)
		dottytype("dz", SDZ, SNDZ);
	if (nl[SNDH].n_type != 0)
		dottytype("dh", SDH, SNDH);
	if (nl[SNDMF].n_type != 0)
		dottytype("dmf", SDMF, SNDMF);
	if (nl[SNDHU].n_type != 0)
		dottytype("dhu", SDHU, SNDHU);
	if (nl[SNDMZ].n_type != 0)
		dottytype("dmz", SDMZ, SNDMZ);
	if (nl[SNPTY].n_type != 0)
		dottytype("pty", SPTY, SNPTY);
#if	MACH
	if (nl[SNCMUPTY].n_type != 0)
		dottytype("cmupty", SCMUPTY, SNCMUPTY);
	if (nl[SNFETTY].n_type != 0)
		dottytype("fe", SFETTY, SNFETTY);
#endif	MACH
}

dottytype(name, type, number)
char *name;
{
	int ntty;
	register struct tty *tp;
	extern char *realloc();

	if (tty == (struct tty *)0)
		return;
	if (kflg) {
		nl[number].n_value = clear(nl[number].n_value);
		nl[type].n_value = clear(nl[type].n_value);
	}
	lseek(fc, (long)nl[number].n_value, 0);
	read(fc, &ntty, sizeof(ntty));
	printf("%d %s lines\n", ntty, name);
	if (ntty > ttyspace) {
		ttyspace = ntty;
		if ((tty = (struct tty *)realloc(tty, ttyspace * sizeof(*tty))) == 0) {
			printf("pstat: out of memory\n");
			return;
		}
	}
	lseek(fc, (long)nl[type].n_value, 0);
	read(fc, tty, ntty * sizeof(struct tty));
	printf(mesg);
	for (tp = tty; tp < &tty[ntty]; tp++)
		ttyprt(tp, tp - tty);
}

ttyprt(atp, line)
struct tty *atp;
{
	register struct tty *tp;

#if	MACH
	if (atp->t_flags == 0 && atp->t_state == 0)
	    return;
#endif	MACH
	printf("%2d", line);
	tp = atp;
	switch (tp->t_line) {

/*
	case NETLDISC:
		if (tp->t_rec)
			printf("%4d%4d", 0, tp->t_inbuf);
		else
			printf("%4d%4d", tp->t_inbuf, 0);
		break;
*/

	default:
		printf("%4d%4d", tp->t_rawq.c_cc, tp->t_canq.c_cc);
	}
	printf("%4d %8x %8x%4d%4d", tp->t_outq.c_cc, tp->t_flags,
		tp->t_addr, tp->t_delct, tp->t_col);
	putf(tp->t_state&TS_TIMEOUT, 'T');
	putf(tp->t_state&TS_WOPEN, 'W');
	putf(tp->t_state&TS_ISOPEN, 'O');
	putf(tp->t_state&TS_FLUSH, 'F');
	putf(tp->t_state&TS_CARR_ON, 'C');
	putf(tp->t_state&TS_BUSY, 'B');
	putf(tp->t_state&TS_ASLEEP, 'A');
	putf(tp->t_state&TS_XCLUDE, 'X');
	putf(tp->t_state&TS_TTSTOP, 'S');
	putf(tp->t_state&TS_HUPCLS, 'H');
	printf("%6d", tp->t_pgrp);
	switch (tp->t_line) {

	case OTTYDISC:
		printf("\n");
		break;

	case NTTYDISC:
		printf(" ntty\n");
		break;

	case NETLDISC:
		printf(" berknet\n");
		break;

	case TABLDISC:
		printf(" tab\n");
		break;

	default:
		printf(" %d\n", tp->t_line);
	}
}

dousr()
{
	struct user U;
	struct identity UI;
	register i, j, *ip;
	register struct nameidata *nd = &U.u_nd;

	/* This wins only if CLBYTES >= sizeof (struct user) */
	lseek(fm, ubase * NBPG, 0);
	read(fm, &U, sizeof(U));
	lseek(fm,(long)U.u_identity,0);
	read(fm, &UI, sizeof(UI));
	printf("pcb");
	ip = (int *)&U.u_pcb;
	while (ip < &U.u_arg[0]) {
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\t");
		printf("%x ", *ip++);
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\n");
	}
	if ((ip - (int *)&U.u_pcb) % 4 != 0)
		printf("\n");
	printf("arg");
	for (i=0; i<sizeof(U.u_arg)/sizeof(U.u_arg[0]); i++) {
		if (i%5==0)
			printf("\t");
		printf(" %.1x", U.u_arg[i]);
		if (i%5==4)
			printf("\n");
	}
	if (i%5)
		printf("\n");
	printf("segflg\t%d\nerror %d\n", nd->ni_segflg, U.u_error);
	printf("uids\t%d,%d,%d,%d\n", UI.id_uid,
			UI.id_gid,UI.id_ruid,UI.id_rgid);
	printf("procp\t%.1x\n", U.u_procp);
	printf("ap\t%.1x\n", U.u_ap);
	printf("r_val?\t%.1x %.1x\n", U.u_r.r_val1, U.u_r.r_val2);
	printf("base, count, offset %.1x %.1x %ld\n", nd->ni_base,
		nd->ni_count, nd->ni_offset);
	printf("cdir rdir %.1x %.1x\n", U.u_cdir, U.u_rdir);
	printf("dirp %.1x\n", nd->ni_dirp);
	printf("dent %d %.14s\n", nd->ni_dent.d_ino, nd->ni_dent.d_name);
	printf("pdir %.1o\n", nd->ni_pdir);
	printf("file");
	for (i=0; i<NOFILE; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%9.1x", U.u_ofile[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (i % 8)
		printf("\n");
	printf("pofile");
	for (i=0; i<NOFILE; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%9.1x", U.u_pofile[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (i % 8)
		printf("\n");
#if	MACH
#else	MACH
	printf("ssave");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ssave.val[i]);
		if (i%5==4)
			printf("\n");
	}
#endif	MACH
	if (i%5)
		printf("\n");
	printf("sigs");
	for (i=0; i<NSIG; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%.1x ", U.u_signal[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (i % 8)
		printf("\n");
	printf("code\t%.1x\n", U.u_code);
	printf("ar0\t%.1x\n", U.u_ar0);
	printf("prof\t%X %X %X %X\n", U.u_prof.pr_base, U.u_prof.pr_size,
	    U.u_prof.pr_off, U.u_prof.pr_scale);
	printf("\neosys\t%d\n", U.u_eosys);
	printf("ttyp\t%.1x\n", U.u_ttyp);
	printf("ttyd\t%d,%d\n", major(U.u_ttyd), minor(U.u_ttyd));
	printf("comm %.14s\n", U.u_comm);
	printf("start\t%D\n", U.u_start);
	printf("acflag\t%D\n", U.u_acflag);
	printf("cmask\t%D\n", U.u_cmask);
	printf("sizes\t%.1x %.1x %.1x\n", U.u_tsize, U.u_dsize, U.u_ssize);
	printf("ru\t");
	ip = (int *)&U.u_ru;
	for (i = 0; i < sizeof(U.u_ru)/sizeof(int); i++)
		printf("%D ", ip[i]);
	printf("\n");
	ip = (int *)&U.u_cru;
	printf("cru\t");
	for (i = 0; i < sizeof(U.u_cru)/sizeof(int); i++)
		printf("%D ", ip[i]);
	printf("\n");
/*
	i =  U.u_stack - &U;
	while (U[++i] == 0);
	i &= ~07;
	while (i < 512) {
		printf("%x ", 0140000+2*i);
		for (j=0; j<8; j++)
			printf("%9x", U[i++]);
		printf("\n");
	}
*/
}

oatoi(s)
char *s;
{
	register v;

	v = 0;
	while (*s)
		v = (v<<3) + *s++ - '0';
	return(v);
}

dofile()
{
	int nfile;
	struct file *xfile, *afile;
	register struct file *fp;
	register nf;
	int loc;
#if	CMU
	static char *dtypes[] = { "???", "inode", "socket",
				  "rfsino", "rfsctl", "viceino" };
#else	/* CMU */
	static char *dtypes[] = { "???", "inode", "socket" };
#endif	/* CMU */

	nf = 0;
	nfile = getw(nl[SNFILE].n_value);
	xfile = (struct file *)calloc(nfile, sizeof (struct file));
	afile = (struct file *)getw(nl[SFIL].n_value);
	if (nfile < 0 || nfile > 10000) {
		fprintf(stderr, "number of files is preposterous (%d)\n",
			nfile);
		return;
	}
	if (xfile == NULL) {
		fprintf(stderr, "can't allocate memory for file table\n");
		return;
	}
	lseek(fc, mkphys((off_t)afile), 0);
	read(fc, xfile, nfile * sizeof (struct file));
	for (fp=xfile; fp < &xfile[nfile]; fp++)
		if (fp->f_count)
			nf++;
	if (totflg) {
		printf("%3d/%3d files\n", nf, nfile);
		return;
	}
	printf("%d/%d open files\n", nf, nfile);
	printf("   LOC   TYPE    FLG     CNT  MSG    DATA    OFFSET\n");
	for (fp=xfile,loc=(int)afile; fp < &xfile[nfile]; fp++,loc+=sizeof(xfile[0])) {
		if (fp->f_count==0)
			continue;
		printf("%8x ", loc);
#if	CMU
		if (fp->f_type <= DTYPE_VICEINO)
#else	/* CMU */
		if (fp->f_type <= DTYPE_SOCKET)
#endif	/* CMU */
			printf("%-8.8s", dtypes[fp->f_type]);
		else
			printf("%8d", fp->f_type);
		putf(fp->f_flag&FREAD, 'R');
		putf(fp->f_flag&FWRITE, 'W');
		putf(fp->f_flag&FAPPEND, 'A');
		putf(fp->f_flag&FSHLOCK, 'S');
		putf(fp->f_flag&FEXLOCK, 'X');
		putf(fp->f_flag&FASYNC, 'I');
		printf("  %3d", mask(fp->f_count));
		printf("  %3d", mask(fp->f_msgcount));
		printf("  %8.1x", fp->f_data);
		if (fp->f_offset < 0)
			printf("  %x\n", fp->f_offset);
		else
			printf("  %ld\n", fp->f_offset);
	}
	free(xfile);
}

#if	MACH
#else	MACH
int dmmin, dmmax, nswdev;

doswap()
{
	struct proc *proc;
	int nproc;
	struct text *xtext;
	int ntext;
	struct map *swapmap;
	int nswapmap;
	struct swdevt *swdevt, *sw;
	register struct proc *pp;
	int nswap, used, tused, free, waste;
	int db, sb;
	register struct mapent *me;
	register struct text *xp;
	int i, j;

	nproc = getw(nl[SNPROC].n_value);
	ntext = getw(nl[SNTEXT].n_value);
	if (nproc < 0 || nproc > 10000 || ntext < 0 || ntext > 10000) {
		fprintf(stderr, "number of procs/texts is preposterous (%d, %d)\n",
			nproc, ntext);
		return;
	}
	proc = (struct proc *)calloc(nproc, sizeof (struct proc));
	if (proc == NULL) {
		fprintf(stderr, "can't allocate memory for proc table\n");
		exit(1);
	}
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	if (xtext == NULL) {
		fprintf(stderr, "can't allocate memory for text table\n");
		exit(1);
	}
	nswapmap = getw(nl[SNSWAPMAP].n_value);
	swapmap = (struct map *)calloc(nswapmap, sizeof (struct map));
	if (swapmap == NULL) {
		fprintf(stderr, "can't allocate memory for swapmap\n");
		exit(1);
	}
	nswdev = getw(nl[SNSWDEV].n_value);
	swdevt = (struct swdevt *)calloc(nswdev, sizeof (struct swdevt));
	if (swdevt == NULL) {
		fprintf(stderr, "can't allocate memory for swdevt table\n");
		exit(1);
	}
	lseek(fc, mkphys((off_t)nl[SSWDEVT].n_value), L_SET);
	read(fc, swdevt, nswdev * sizeof (struct swdevt));
	lseek(fc, mkphys((off_t)getw(nl[SPROC].n_value)), 0);
	read(fc, proc, nproc * sizeof (struct proc));
	lseek(fc, mkphys((off_t)getw(nl[STEXT].n_value)), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	lseek(fc, mkphys((off_t)getw(nl[SWAPMAP].n_value)), 0);
	read(fc, swapmap, nswapmap * sizeof (struct map));
	swapmap->m_name = "swap";
	swapmap->m_limit = (struct mapent *)&swapmap[nswapmap];
	dmmin = getw(nl[SDMMIN].n_value);
	dmmax = getw(nl[SDMMAX].n_value);
	nswap = 0;
	for (sw = swdevt; sw < &swdevt[nswdev]; sw++)
		if (sw->sw_freed)
			nswap += sw->sw_nblks;
	free = 0;
	for (me = (struct mapent *)(swapmap+1);
	    me < (struct mapent *)&swapmap[nswapmap]; me++)
		free += me->m_size;
	tused = 0;
	for (xp = xtext; xp < &xtext[ntext]; xp++)
		if (xp->x_iptr!=NULL) {
			tused += ctod(clrnd(xp->x_size));
			if (xp->x_flag & XPAGI)
				tused += ctod(clrnd(ctopt(xp->x_size)));
		}
	used = tused;
	waste = 0;
	for (pp = proc; pp < &proc[nproc]; pp++) {
		if (pp->p_stat == 0 || pp->p_stat == SZOMB)
			continue;
		if (pp->p_flag & SSYS)
			continue;
		db = ctod(pp->p_dsize), sb = up(db);
		used += sb;
		waste += sb - db;
		db = ctod(pp->p_ssize), sb = up(db);
		used += sb;
		waste += sb - db;
		if ((pp->p_flag&SLOAD) == 0)
			used += ctod(vusize(pp));
	}
	if (totflg) {
#define	btok(x)	((x) / (1024 / DEV_BSIZE))
		printf("%3d/%3d 00k swap\n",
		    btok(used/100), btok((used+free)/100));
		return;
	}
	printf("%dk used (%dk text), %dk free, %dk wasted, %dk missing\n",
	    btok(used), btok(tused), btok(free), btok(waste),
/* a dmmax/2 block goes to argmap */
	    btok(nswap - dmmax/2 - (used + free)));
	printf("avail: ");
	for (i = dmmax; i >= dmmin; i /= 2) {
		j = 0;
		while (rmalloc(swapmap, i) != 0)
			j++;
		if (j) printf("%d*%dk ", j, btok(i));
	}
	free = 0;
	for (me = (struct mapent *)(swapmap+1);
	    me < (struct mapent *)&swapmap[nswapmap]; me++)
		free += me->m_size;
	printf("%d*1k\n", btok(free));
}

up(size)
	register int size;
{
	register int i, block;

	i = 0;
	block = dmmin;
	while (i < size) {
		i += block;
		if (block < dmmax)
			block *= 2;
	}
	return (i);
}

/*
 * Compute number of pages to be allocated to the u. area
 * and data and stack area page tables, which are stored on the
 * disk immediately after the u. area.
 */
vusize(p)
	register struct proc *p;
{
	register int tsz = p->p_tsize / NPTEPG;

	/*
	 * We do not need page table space on the disk for page
	 * table pages wholly containing text. 
	 */
	return (clrnd(UPAGES +
	    clrnd(ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES)) - tsz));
}

/*
 * Allocate 'size' units from the given
 * map. Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 *
 * Algorithm is first-fit.
 *
 * This routine knows about the interleaving of the swapmap
 * and handles that.
 */
long
rmalloc(mp, size)
	register struct map *mp;
	long size;
{
	register struct mapent *ep = (struct mapent *)(mp+1);
	register int addr;
	register struct mapent *bp;
	swblk_t first, rest;

	if (size <= 0 || size > dmmax)
		return (0);
	/*
	 * Search for a piece of the resource map which has enough
	 * free space to accomodate the request.
	 */
	for (bp = ep; bp->m_size; bp++) {
		if (bp->m_size >= size) {
			/*
			 * If allocating from swapmap,
			 * then have to respect interleaving
			 * boundaries.
			 */
			if (nswdev > 1 &&
			    (first = dmmax - bp->m_addr%dmmax) < bp->m_size) {
				if (bp->m_size - first < size)
					continue;
				addr = bp->m_addr + first;
				rest = bp->m_size - first - size;
				bp->m_size = first;
				if (rest)
					rmfree(mp, rest, addr+size);
				return (addr);
			}
			/*
			 * Allocate from the map.
			 * If there is no space left of the piece
			 * we allocated from, move the rest of
			 * the pieces to the left.
			 */
			addr = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
			}
			if (addr % CLSIZE)
				return (0);
			return (addr);
		}
	}
	return (0);
}

/*
 * Free the previously allocated space at addr
 * of size units into the specified map.
 * Sort addr into map and combine on
 * one or both ends if possible.
 */
rmfree(mp, size, addr)
	struct map *mp;
	long size, addr;
{
	struct mapent *firstbp;
	register struct mapent *bp;
	register int t;

	/*
	 * Both address and size must be
	 * positive, or the protocol has broken down.
	 */
	if (addr <= 0 || size <= 0)
		goto badrmfree;
	/*
	 * Locate the piece of the map which starts after the
	 * returned space (or the end of the map).
	 */
	firstbp = bp = (struct mapent *)(mp + 1);
	for (; bp->m_addr <= addr && bp->m_size != 0; bp++)
		continue;
	/*
	 * If the piece on the left abuts us,
	 * then we should combine with it.
	 */
	if (bp > firstbp && (bp-1)->m_addr+(bp-1)->m_size >= addr) {
		/*
		 * Check no overlap (internal error).
		 */
		if ((bp-1)->m_addr+(bp-1)->m_size > addr)
			goto badrmfree;
		/*
		 * Add into piece on the left by increasing its size.
		 */
		(bp-1)->m_size += size;
		/*
		 * If the combined piece abuts the piece on
		 * the right now, compress it in also,
		 * by shifting the remaining pieces of the map over.
		 */
		if (bp->m_addr && addr+size >= bp->m_addr) {
			if (addr+size > bp->m_addr)
				goto badrmfree;
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
		}
		goto done;
	}
	/*
	 * Don't abut on the left, check for abutting on
	 * the right.
	 */
	if (addr+size >= bp->m_addr && bp->m_size) {
		if (addr+size > bp->m_addr)
			goto badrmfree;
		bp->m_addr -= size;
		bp->m_size += size;
		goto done;
	}
	/*
	 * Don't abut at all.  Make a new entry
	 * and check for map overflow.
	 */
	do {
		t = bp->m_addr;
		bp->m_addr = addr;
		addr = t;
		t = bp->m_size;
		bp->m_size = size;
		bp++;
	} while (size = t);
	/*
	 * Segment at bp is to be the delimiter;
	 * If there is not room for it 
	 * then the table is too full
	 * and we must discard something.
	 */
	if (bp+1 > mp->m_limit) {
		/*
		 * Back bp up to last available segment.
		 * which contains a segment already and must
		 * be made into the delimiter.
		 * Discard second to last entry,
		 * since it is presumably smaller than the last
		 * and move the last entry back one.
		 */
		bp--;
		printf("%s: rmap ovflo, lost [%d,%d)\n", mp->m_name,
		    (bp-1)->m_addr, (bp-1)->m_addr+(bp-1)->m_size);
		bp[-1] = bp[0];
		bp[0].m_size = bp[0].m_addr = 0;
	}
done:
	return;
badrmfree:
	printf("bad rmfree\n");
}
#endif	MACH
/*
 * "addr"  is a kern virt addr and does not correspond
 * To a phys addr after zipping out the high bit..
 * since it was valloc'd in the kernel.
 *
 * We return the phys addr by simulating kernel vm (/dev/kmem)
 * when we are reading a crash dump.
 */
off_t
mkphys(addr)
	off_t addr;
{
	register off_t o;

	if (!kflg)
		return(addr);
#if	MACH && !defined(vax)
	printf("If ya wants ta use it, ya gots ta write it.\n");
	exit(1);
#else	MACH && !defined(vax)
	o = addr & PGOFSET;
	addr >>= PGSHIFT;
	addr &= PG_PFNUM;
	addr *=  NBPW;
	addr = getw(nl[SYSMAP].n_value + addr);
	addr = ((addr & PG_PFNUM) << PGSHIFT) | o;
	return(addr);
#endif	MACH && !defined(vax)
}
