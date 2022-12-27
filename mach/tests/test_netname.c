/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	test_netname.c,v $
 * Revision 1.3  89/05/05  19:04:34  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 * 28-Jun_88 Mary Thomoson (mrt) at Carnegie Mellon
 *	Changed name of nm_token port to NM_ACTIVE
 *
 * 13-May-88 Mary Thomoson (mrt) at Carnegie Mellon
 *	Changed mach_errormsg to mach_error_string.
 * 18-Apr-88 Mary Thompson
 *	Changed name of nmmonitor port from [machine]nmmonitor
 *	to just nmmonitor, which is what the new netmsgerver uses
 *	Should be changed to nm_token as soon as new netmsgerver
 *	uses it.
 */
/*  Program to check if named is working correctly 
 *  and that netnameserver lookups are working
 */

#include <stdio.h>
#include <netdb.h>
#include <mach.h>
#include <servers/netname.h>
#include <mach_error.h>


main(argc,argv)
   int argc;
   char *argv[];
{
    char hostname[80];
    int namelen = 80;
    struct hostent *host_ent;
    char NMMonServName[120];
    char **namep;
    port_t nm_port;
    kern_return_t gr;
    int exit_value = 0;

   if ( gethostname( hostname, namelen) == -1)
   {	printf("cannot get my host name\n");
	exit (1);
   }
   sprintf(NMMonServName, "%s", "NM_ACTIVE");
   printf("port name is %s\n",NMMonServName);
   host_ent = gethostbyname(hostname);
   if (host_ent == 0)
   {	printf ("gethostbyname(%s) failed\n",hostname);
	exit_value = 2;
   }
   else
   {	printf("host_ent->h_name is %s\n",host_ent->h_name);
   	namep = host_ent->h_aliases;
	if (*namep == 0)
		printf("no aliases found\n");
	else
	{	while(*namep != 0)
		{  printf("Alias is %s\n",*namep);
		   namep++;
		}
	}
   }

   if (argc == 1)
   {
	printf("Doing local lookup of %s\n",NMMonServName);
	gr = netname_look_up(name_server_port,"",NMMonServName,&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %d\n",NMMonServName,(int)nm_port);

	printf("Doing broadcast lookup of %s\n",NMMonServName);
	gr = netname_look_up(name_server_port,"*",NMMonServName,&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %d\n",NMMonServName,(int)nm_port);

	printf("Doing host-specific lookup of %s\n",NMMonServName);
	gr = netname_look_up(name_server_port,hostname,NMMonServName,&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %d\n",NMMonServName,(int)nm_port);
   }
   else 
   {    if (argc != 3)
	{	printf("usage is testn [hostname portname]\n");
		exit (1);
	}
	printf("Doing local lookup of %s\n",argv[2]);
	gr = netname_look_up(name_server_port,"",argv[2],&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %d\n",argv[2],(int)nm_port);

	printf("Doing broadcast lookup of %s\n",argv[2]);
	gr = netname_look_up(name_server_port,"*",argv[2],&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %d\n",argv[2],(int)nm_port);

	printf("Doing host-specific lookup of %s\n",argv[2]);
	gr = netname_look_up(name_server_port,argv[1],argv[2],&nm_port);
	if (gr != KERN_SUCCESS)
	{	printf("net_name_lookup failed - %s\n",mach_error_string(gr));
		exit_value = 3;
	}
	else printf("%s is %d\n",argv[2],(int)nm_port);
   }
   exit (exit_value);
}
