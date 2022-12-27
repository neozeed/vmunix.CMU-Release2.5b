/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	vm_read.c,v $
 * Revision 1.2  89/05/05  19:04:51  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  4-Jan-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Changed how we get page size.
 *	Declared data, data_cnt; added appropriate typing.
 */

#include <mach_init.h>
#include <mach.h>
#include <stdio.h>

main(argc, argv)
	int	argc;
	char	*argv[];
{
	char		*data1, *temp;
	char		*data2;
	int		data_cnt, save_address, i, min;  
	kern_return_t	rtn;

	if (argc != 1) {
		printf("vm_read requires no switches.\n");
		printf("vm_read will allocate some space, and fill it with\n");
		printf("data.  vm_read will then be called starting at\n");
		printf("the address of the previously allocated\n");
		printf("data.  After vm_read returns, the data it read is\n");
		printf("compared with the data in the allocated space;\n");
		printf("this is to test vm_read.  vm_deallocate is then\n");
		printf("used.\n");
		exit(1);
	}

	if ((rtn = vm_allocate(task_self(), &data1, vm_page_size,
		TRUE)) != KERN_SUCCESS) {
		mach_error("vm_read: vm_allocate returned ", rtn);
		exit(1);
	}
	printf("vm_read: Successful vm_allocate.\n");

	temp = data1;
	for (i = 0; (i < vm_page_size); i++) 
		temp[i] = i;
	
	printf("doing read....\n");
	if ((rtn = vm_read(task_self(), data1, vm_page_size, &data2, 
		&data_cnt)) != KERN_SUCCESS) {
		mach_error("vm_read: vm_read returned ", rtn);
		exit(1);
	}
	printf("vm_read: Successful vm_read.\n");
	

	if (vm_page_size != data_cnt) {
		printf("vm_read: Number of bytes read not equal to number");
		printf("available and requested.  \n");
	}
	min = (vm_page_size < data_cnt) ? vm_page_size : data_cnt;


	for (i = 0; (i < min); i++) {
		if (data1[i] != data2[i]) {
			printf("vm_read: Data not read correctly.\n");
			printf("vm_read: Exiting.\n");
			exit(1);
		}
	}
	printf("vm_read: Successful copying.\n");

	save_address = (int) data1;

	if ((rtn = vm_deallocate(task_self(), data1, 
		vm_page_size)) != KERN_SUCCESS) {
		mach_error("vm_read vm_deallocate returned ", rtn);
		exit(1);
	}

	if ((rtn = vm_allocate(task_self(), &data1, vm_page_size,
		TRUE)) != KERN_SUCCESS) {
		mach_error("vm_read: vm_allocate returned ", rtn);
		exit(1);
	}

	if (save_address != (int) data1) {
		printf("vm_read: New address not equal to previously ");
		printf("deallocated address\n");
		printf("vm_deallocate errored.\n");
		printf("vm_read: Exiting.\n");
		exit(1);
	}
	printf("vm_read: Successful vm_deallocate.\n");

	if ((rtn = vm_deallocate(task_self(), data2, 
		data_cnt)) != KERN_SUCCESS) {
		mach_error("vm_read: vm_dealloacte returned ", rtn);
		exit(1);
	}
	printf("vm_read: Finished successfully!\n");
	exit(0);
}

