/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	test_service.c,v $
 * Revision 1.2  89/05/05  19:04:44  mrt
 * 	Cleanup for Mach 2.5
 * 
 */

#include <servers/service.h>
#include <mach/message.h>
#include <mach_init.h>

main()
{
	int		slot;
	port_t		*ports;
	int		ports_count;
	port_t		mine;
	kern_return_t	retcode;

	printf("name_server_port is %d\n",name_server_port);
	printf("environment_port is %d\n",environment_port);
	printf("service_port is %d\n",service_port);

	slot = 3;  /* extra slot to play with */
	retcode = mach_ports_lookup(task_self(), &ports, &ports_count);
	if (retcode != KERN_SUCCESS)
	{	printf("Error from mach_ports_lookup: %d\n",retcode);
		exit (1);
	}
	else 
	{  	printf("port cnt is %d, array is %d %d %d %d\n",
				ports_count,ports[0],ports[1],ports[2],ports[3]);
		printf("port to checkin is %d\n",ports[slot]);
		init_service(PORT_NULL);
		retcode = service_checkin(service_port,ports[slot], &mine);
		if (retcode != KERN_SUCCESS)
		{	printf("service_checkin failed with error code: %d\n", retcode);
			exit(1);
		}
		else printf("port %d is now mine\n", mine);
	}
	exit(0);

}
