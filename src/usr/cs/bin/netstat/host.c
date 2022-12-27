/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)host.c	5.2 (Berkeley) 9/27/85";
#endif not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	host.c,v $
 * Revision 1.2  89/01/17  14:40:21  parker
 * 	Fixed arppr() to work with valloc'd arptab [courtesy of
 * 	Robert Baron (rvb)] and changed arppr() to print ethernet
 * 	address in hex format.
 * 	[89/01/17            parker]
 * 
 * 20-Jun-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Merged changes (below) for 4.3.
 *
 * 28-Sep-85  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added arppr() routine to display ARP cache.
 *
 **********************************************************************
 */
#include <sys/types.h>
#include <sys/mbuf.h>

#include <netinet/in.h>
#include <netimp/if_imp.h>
#include <netimp/if_imphost.h>

extern	int kmem;
extern 	int nflag;
extern	char *inetname();

/*
 * Print the host tables associated with the ARPANET IMP.
 * Symbolic addresses are shown unless the nflag is given.
 */
hostpr(hostsaddr)
	off_t hostsaddr;
{
	struct mbuf *hosts, mb;
	register struct mbuf *m;
	register struct hmbuf *mh;
	register struct host *hp;
	char flagbuf[10], *flags;
	int first = 1;

	if (hostsaddr == 0) {
		printf("hosts: symbol not in namelist\n");
		return;
	}
	klseek(kmem, hostsaddr, 0);
	read(kmem, &hosts, sizeof (hosts));
	m = hosts;
	printf("IMP Host Table\n");
	printf("%-5.5s %-15.15s %-4.4s %-9.9s %-4.4s %s\n",
		"Flags", "Host", "Qcnt", "Q Address", "RFNM", "Timer");
	while (m) {
		klseek(kmem, m, 0);
		read(kmem, &mb, sizeof (mb));
		m = &mb;
		mh = mtod(m, struct hmbuf *);
		if (mh->hm_count == 0) {
			m = m->m_next;
			continue;
		}
		for (hp = mh->hm_hosts; hp < mh->hm_hosts + HPMBUF; hp++) {
			if ((hp->h_flags&HF_INUSE) == 0 && hp->h_timer == 0)
				continue;
			flags = flagbuf;
			*flags++ = hp->h_flags&HF_INUSE ? 'A' : 'F';
			if (hp->h_flags&HF_DEAD)
				*flags++ = 'D';
			if (hp->h_flags&HF_UNREACH)
				*flags++ = 'U';
			*flags = '\0';
			printf("%-5.5s %-15.15s %-4d %-9x %-4d %d\n",
				flagbuf,
				inetname(hp->h_addr),
				hp->h_qcnt,
				hp->h_q,
				hp->h_rfnm,
				hp->h_timer);
		}
		m = m->m_next;
	}
}

#if	CMUCS
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/if_ether.h>

/*
 * Print the ARP cache protocol/hardware address cache.
 * Symbolic addresses are shown unless the nflag is given.
 */
arppr(arptabaddr, bsizaddr, nbaddr)
	off_t arptabaddr;
	off_t bsizaddr;
	off_t nbaddr;
{
	long bsize = 5;
	long nb = 19;
	caddr_t arptabptr;
	register struct arptab *arptab;
	register int i, j = 0, m;
	char flagbuf[10], *flags;
	extern int Aflag;

	if (arptabaddr == 0) {
		printf("arptab: symbol(s) not in namelist\n");
		return;
	}
	if (arptabaddr != 0)
	{
		klseek(kmem, arptabaddr, 0);
		read(kmem, &arptabptr, sizeof (arptabptr));
	}
	if (bsizaddr != 0)
	{
		klseek(kmem, bsizaddr, 0);
		read(kmem, &bsize, sizeof (bsize));
	}
	if (nbaddr != 0)
	{
		klseek(kmem, nbaddr, 0);
		read(kmem, &nb, sizeof (nb));
	}
	m = bsize*nb;
        arptab = (struct arptab *)malloc(sizeof(struct arptab)*m);
	if (arptab == 0)
	{
		printf("arptab: could not allocate %d table entries\n", m);
		return;
	}
	klseek(kmem, arptabptr, 0);
	read(kmem, arptab, sizeof (struct arptab)*m);
	printf("Address Resolution Mapping Table: %d Buckets %d Deep %d Bytes\n",
		nb, bsize, sizeof (struct arptab));
	printf("Cnt  ");
	if (Aflag)
		printf("%-8.8s ", "ADDR");
	printf("%5.5s %-5.5s %-24.24s %s\n",
		"Flags", "Timer", "Host", "Phys Addr");
	for (i=0; i<m; i++) {
		register struct arptab *at = &arptab[i];

		if (at->at_flags == 0)
			continue;
		flags = flagbuf;
		if (at->at_flags&ATF_INUSE)
			*flags++ = 'U';
		if (at->at_flags&ATF_COM)
			*flags++ = 'C';
		if (at->at_flags&ATF_PERM)
			*flags++ = 'P';
		if (at->at_flags&ATF_PUBL)
			*flags++ = 'B';
		if (at->at_flags&ATF_USETRAILERS)
			*flags++ = 'T';
		*flags = '\0';
		printf("%3d: ", ++j);
		if (Aflag)
			printf("%8x ",arptabaddr+((caddr_t)at-(caddr_t)arptab));
		printf("%-5.5s  %3d  %-24.24s ", flagbuf, at->at_timer, inetname(at->at_iaddr));
		if (at->at_enaddr[5] == 0 && at->at_enaddr[4] == 0 && at->at_enaddr[3] == 0 && at->at_enaddr[2] == 0 && at->at_enaddr[1] == 0)
			printf("#%o", at->at_enaddr[0]);
		else
			printf("%02x:%02x:%02x:%02x:%02x:%02x",
			       at->at_enaddr[0], at->at_enaddr[1], at->at_enaddr[2],
			       at->at_enaddr[3], at->at_enaddr[4], at->at_enaddr[5]);
		printf("\n");
	}
}
#endif	CMUCS
