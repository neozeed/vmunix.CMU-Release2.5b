/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	hostinfo.c,v $
 * Revision 1.2  89/05/05  18:22:18  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 * 28-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	File:	hostinfo.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1987, Avadis Tevanian, Jr.
 *
 *	Display information about the host this program is
 *	execting on.
 */

#include <mach.h>

struct machine_info	mi;
struct machine_slot	ms;
int			nslots;

main(argc, argv)
	int	argc;
	char	*argv[];
{
	kern_return_t	ret;
	register int	i;

	ret = host_info(task_self(), &mi);
	if (ret != KERN_SUCCESS) {
		mach_error(argv[0], ret);
		exit(1);
	}
	printf("Mach kernel version %d.%d.\n", mi.major_version,
			mi.minor_version);
	nslots = mi.max_cpus;
	if (nslots > 1)
		printf("Kernel configured for up to %d processors.\n", nslots);
	else
		printf("Kernel configured for a single processor only.\n");
	printf("%d processor%s physically available.\n", mi.avail_cpus,
		(mi.avail_cpus > 1) ? "s are" : " is");
	printf("Primary memory available: %.2f megabytes.\n",
			(float)mi.memory_size/(1024.0*1024.0));
	for (i = 0; i < nslots; i++)
		do_slot(i);
}

do_slot(slot)
	int	slot;
{
	kern_return_t	ret;

	ret = slot_info(task_self(), slot, &ms);
	if (ms.is_cpu) {
		printf("slot %d:", slot);
		do_cpu(&ms);
		printf("\n");
	}
}

do_cpu(ms)
	register struct machine_slot	*ms;
{
	char	*cpu_name, *cpu_subname;

	slot_name(ms->cpu_type, ms->cpu_subtype, &cpu_name, &cpu_subname);
	printf(" %s (%s) %s", cpu_name, cpu_subname,
			ms->running ? "UP" : "DOWN");
}
