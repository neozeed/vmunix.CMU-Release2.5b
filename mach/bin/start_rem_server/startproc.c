/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	startproc.c,v $
 * Revision 1.2  89/06/08  18:16:03  mrt
 * 	Updated for Mach 2.5
 * 
 *  13-May-88 Mary R Thompson (mrt) at Carnegie Mellon
 *	Changed rreferences to mach_errormsg to mach_error_string
 */
/*
 * Abstract:
 * 	Random things to start a logged-in process on a vax
 */

#include <stdio.h>
#include <mach.h>
#include <servers/ipcx.h>
#include <mach_error.h>

extern int errno;
extern char CurrentPassword[];	/* Don`t worry: only encripted */
extern char CurrentUserid[];
extern char *PREFIX;

extern int  GetPassword();


/*
 * This is the callable one...
 */
int StartProcess(m,c)
char *m, *c;
{
	kern_return_t   gr;
	port_t          k, d, ipcexecdport;
	char            temp[256];

	if (CurrentPassword[0] == '\0')
		GetPassword(0);

	strcpy(temp, PREFIX);
	strcat(temp, m);
	if ((gr = netname_look_up(name_server_port, m, temp, &ipcexecdport))
	    != KERN_SUCCESS)
	{
		printf("Error %s trying to find %s\n", mach_error_string(gr), temp);
		return (-1);
	}

	gr = startserver(ipcexecdport, CurrentUserid, CurrentPassword, c, &k, &d);

	if (gr == KERN_SUCCESS) {
		if (k != PORT_NULL)
		{
			printf("Server successfully started on %s (%d,%d)\n",
		      		 m, k, d);
			return (0);
		}
		else
		{	printf("Incorrect account or password\n");
			return (-1);
		}
	}

	printf("Error %s in starting task on  %s\n", mach_error_string(gr), m);
	return (-1);
}

