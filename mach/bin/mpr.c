/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	mpr.c,v $
 * Revision 1.2  89/05/05  18:25:37  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 * Abstract:
 *
 * This program allows the user to customize a task's registered ports.
 * The kernel keeps a certain number of port rights (currently always
 * four) on behalf of a task; mach_ports_lookup and mach_ports_register
 * manipulate these rights.  The standard task initialization code in
 * libmach uses them (if present) to determine the name server's port,
 * environment manager's port, and service server's port.
 *
 * Syntax Synopsis:
 * mpr <0 or more port specs> -- <command & args>
 * A port spec may be
 *   name	Look up name locally.
 *   name@host	Look up name at remote host.
 *   ,		Leave this port unchanged.
 *   -		Pass PORT_NULL in this position.
 *
 * Examples:
 *   To run "foo" without an environment manager:
 *	mpr , - -- foo
 *   To run "bar" with a fake name server, registered locally as "Ersatz":
 *	mpr Ersatz -- bar
 *   To run "baz" so that it thinks it is on the moon:
 *	mpr NameServer@moon EnvironmentManager@moon ServiceServer@moon -- baz
*/

#include <stdio.h>
#include <strings.h>
#include <mach/mach_types.h>
#include <mach.h>
#include <mach_error.h>
#include <servers/netname.h>


#define	streql(a, b)	(strcmp((a), (b)) == 0)

static char *program;

static port_t LookUp();

void
main(argc, argv)
    int argc;
    char *argv[];
{
    port_array_t ports;
    unsigned int count;		/* length of ports array */

    int i;			/* steps through argv */
    int j;			/* steps through ports */
    kern_return_t kr;

    program = argv[0];

    /* get current registered ports */

    kr = mach_ports_lookup(task_self(), &ports, &count);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_ports_lookup: %s\n",
	     program, mach_error_string(kr));

    /* mutate the ports array as desired */

    for (i = 1, j = 0; i < argc; i++, j++)
    {
	char *at;

	if (streql(argv[i], "--"))
	{
	    i++;
	    break;
	}
	else if (j >= count)
	    quit(1, "%s: too many ports - limit is %u\n", program, count);
	else if (streql(argv[i], ","))
	    /* just leave this entry alone */
	    ;
	else if (streql(argv[i], "-"))
	    /* make this entry be PORT_NULL */
	    ports[j] = PORT_NULL;
	else if ((at = index(argv[i], '@')) != NULL)
	{
	    netname_name_t name;

	    /* avoid stepping on argv; it's not portable */
	    (void) strncpy(name, argv[i], at - argv[i]);
	    name[at - argv[i]] = '\0';
	    ports[j] = LookUp(name, at+1);
	}
	else
	    /* default to local lookup if no host specified */
	    ports[j] = LookUp(argv[i], "");
    }

    if (i >= argc)
	quit(1, "%s: no command?\n", program);

    /* register the mutated ports array with the kernel */

    kr = mach_ports_register(task_self(), ports, count);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: mach_ports_register: %s\n",
	     program, mach_error_string(kr));

    /* now that the ports array is unneeded, release the VM */

    kr = vm_deallocate(task_self(), (vm_address_t) ports,
		       (vm_size_t) (count*sizeof(port_t)));
    if (kr != KERN_SUCCESS)
	quit(1, "%s: vm_deallocate: %s\n",
	     program, mach_error_string(kr));

    /* do something useful */

    if (execvp(argv[i], &argv[i]) < 0)
	quit(1, "%s: execvp failed\n", program);
}

static port_t
LookUp(name, host)
    netname_name_t name, host;
{
    port_t port;
    kern_return_t kr;

    kr = netname_look_up(name_server_port, host, name, &port);
    if (kr != KERN_SUCCESS)
	quit(1, "%s: netname_look_up(%s@%s): %s\n",
	     program, name, host, mach_error_string(kr));

    return port;
}
