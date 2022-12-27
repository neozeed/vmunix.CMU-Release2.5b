/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	start_rem_server.c,v $
 * Revision 1.2  89/06/08  18:15:57  mrt
 * 	Updated for Mach 2.5
 * 
 *
 *  7-May-87  Mary Thompson at Carnegie Mellon
 *	started.
 */
/*
 * Abstract:
 * 	Program to start a process on a remote machine
 */
#include <stdio.h>
#include <mach.h>
#include <servers/ipcx.h>

char *PREFIX;

main(argc, argv)
char **argv;
{
	char    machname[256];
	char	cmdline[256];
	int	i;

	init_ipcx(PORT_NULL);

	machname[0] = '\0';
	PREFIX = DEFAULT_PREFIX;

	for (i = 1; i<argc; i++)
	if (argv[i][0] == '-')
		switch (argv[i++][1])
		{ case 'p': PREFIX = argv[i];
			    break;
		  case 'h': strcpy(machname,argv[i]);
			    break;
		  default : printf("usage is: %s, [-p <prefix>] [-h <hostname>]\n",
					argv[0]);
		}


	if (machname[0] == '\0')
	{	printf("Start process on machine :");
		fflush(stdout);
		fgets(machname, 255, stdin);
		machname[strlen(machname) - 1] = '\0';
		printf("machine name is %s\n",machname);
	}
	printf("Command line to start remote process:");
	fflush(stdout);
	fgets(cmdline, 255, stdin);
	cmdline[strlen(cmdline) - 1] = '\0';
	StartProcess(machname, cmdline);

}
