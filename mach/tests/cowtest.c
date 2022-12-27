/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	cowtest.c,v $
 * Revision 1.2  89/05/05  19:03:22  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 *	Test copy-on-write.
 */

#include <mach_init.h>
#include <mach.h>
#include <stdio.h>
#include <sys/wait.h>

#define NO_ONE_WAIT 0
#define PARENT_WAIT 1
#define CHILD_WAIT 2
#define	streql(a,b)	(strcmp(a,b) == 0)

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int		npages, pid;
	int		*mem, *lock;
	register int	i;
	int		ret;
	union wait	status;

	if (*++argv != (char *) 0) {
		if (streql(*argv, "-h")) {
			printf("cowtest <number of pages, default is 100>\n");
			printf("This program tests copy on write memory\n");
			printf("between child and parent processes.\n");
			printf("Do not run this program unless there is a least\n");
			printf("(2 * #of pages * pagesize) disk space free.\n");
			exit(1);
		}
		else if ((npages = atoi(*argv)) <= 0)
			npages = 100;
	}
	else npages = 100;

	if ((ret = vm_allocate(task_self(), &lock, sizeof(int), 
		TRUE)) != KERN_SUCCESS) {
		mach_error("cowtest: vm_allocate returned ", ret);
		exit(1);
	}
	printf("cowtest: Successful vm_allocate.\n");
	
	if ((ret = vm_inherit(task_self(), lock, vm_page_size, 
		VM_INHERIT_SHARE)) != KERN_SUCCESS) {
		mach_error("cowtest: vm_inherit returned ", ret);
		exit(1);
	}
	printf("cowtest: Successful vm_inherit\n");

	if ((ret = vm_allocate(task_self(), &mem, npages*vm_page_size, 
		TRUE)) != KERN_SUCCESS) {
		mach_error("cowtest: vm_allocate returned ", ret);
		exit(1);
	}

	printf("cowtest: initalize %d of the %d pages...", npages/2, npages);
	fflush(stdout);
	for (i = 0; i < npages/2; i++)
		mem[i * (vm_page_size/sizeof(int))] = i+1;
	printf("done.\n");

	*lock = NO_ONE_WAIT;  
	pid = fork();
	if (pid) {
		printf("cowtest: parent: store new data...");
		fflush(stdout);
		for (i = 0; i < npages; i++)
			mem[i * (vm_page_size/sizeof(int))] = i;
		printf("done.\n");

		printf("cowtest: parent: waiting for child pass\n");
		*lock = PARENT_WAIT;  
		while (*lock == PARENT_WAIT) ;

		printf("cowtest: parent: active again\n");
		printf("cowtest: parent: checking data...");
		fflush(stdout);
		for (i = 0; i < npages; i++)
			if (mem[i*(vm_page_size/sizeof(int))] != i) {
				printf("cowtest: parent: incorrect entry (%d) at page %d.\n", mem[i*(vm_page_size/sizeof(int))], i);
				exit(1);
			}
		printf("done.\n");
		printf("Parent process exiting.\n");
		*lock = PARENT_WAIT;
		exit(wait(&status) ? status.w_retcode : 1);
	}

	while (*lock != PARENT_WAIT) ;
	printf("cowtest: child: starting pass\n");
	printf("cowtest: child: checking data...");
	fflush(stdout);
	for (i = 0; i < npages; i++) {
		if (mem[i*(vm_page_size/sizeof(int))] != 
			((i < (npages/2)) ? (i+1) : 0)) {
			printf("cowtest: child: incorrect entry (%d) at page %d.\n",
				mem[i*(vm_page_size/sizeof(int))], i);
			exit(1);
		}
	}
	printf("done.\n");
	printf("cowtest: child: ending pass\n");
	*lock = CHILD_WAIT;
	while (*lock == CHILD_WAIT);
	printf("Child process exiting.\n");
	printf("cowtest: Finished successfully.\n");
	exit(0);
}
