/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	ipc_test.c,v $
 * Revision 1.4  89/06/08  18:02:10  mrt
 * 	Changed task_data() to thread_reply() as the former is no longer
 * 	exported by the kernel.
 * 	[89/05/28            mrt]
 * 
 * Revision 1.3  89/05/05  19:04:15  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  6-Feb-89  David Black (dlb) at Carnegie-Mellon University
 *	Add missing 3rd arg to vm_deallocate.
 *
 *  8-Jan-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added argument parsing.
 *
 *  6-Oct-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Created from several test programs by Rick Rashid; in the
 *	process, converted to use appropriate Mach headers and functions.
 *
 */

#include <stdio.h>
#include <mach.h>
#include <mach_error.h>
#include <mach/message.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <strings.h>
#include <servers/env_mgr.h>
#include <sys/wait.h>


int		data_size = 0;
int		iterations = 1;
boolean_t	inline = TRUE;
boolean_t	paired = FALSE;
boolean_t	use_rpc = FALSE;
boolean_t	do_select = FALSE;
boolean_t	dealloc = FALSE;
msg_option_t	rcv_option = MSG_OPTION_NONE;
port_t		my_port;
port_t		test;

#define	MAXDATA	1024

struct trial {
	msg_header_t	h;
	msg_type_t	t;
	union {
		int		inline_data[MAXDATA];
		int		*out_of_line_data;
	} d;
} msg_rcv, msg_xmt;

int	*foo;




void setup_reply(msg_xmt, msg_rcv)
	struct trial	*msg_xmt, *msg_rcv;
{
	msg_xmt->h.msg_remote_port = msg_rcv->h.msg_remote_port;
	msg_xmt->h.msg_local_port = PORT_NULL;
	msg_xmt->h.msg_id = 0x12345678;
	msg_xmt->h.msg_size = sizeof(msg_header_t) + sizeof(msg_type_t) + 
		sizeof(int);
	msg_xmt->h.msg_type = MSG_TYPE_NORMAL;
	msg_xmt->h.msg_simple = TRUE;

	msg_xmt->t.msg_type_name = MSG_TYPE_INTEGER_32;
	msg_xmt->t.msg_type_size = 32;
	msg_xmt->t.msg_type_number = 1;
	msg_xmt->t.msg_type_inline = TRUE;
	msg_xmt->t.msg_type_longform = FALSE;
	msg_xmt->t.msg_type_deallocate = FALSE;
}





void setup_request(msg_xmt, i)
	register
	struct trial	*msg_xmt;
	int		i;
{
	msg_xmt->h.msg_local_port = thread_reply();
	msg_xmt->h.msg_remote_port = my_port;
	if (inline)
		msg_xmt->h.msg_size = sizeof(msg_header_t) + 
			sizeof(msg_type_t) + data_size * sizeof(int);
	 else
		msg_xmt->h.msg_size = sizeof(msg_header_t) + 
			sizeof(msg_type_t) + sizeof(int *);

	msg_xmt->h.msg_id = 0x12345678;
	msg_xmt->h.msg_type = MSG_TYPE_NORMAL;
	msg_xmt->h.msg_simple = inline;

	msg_xmt->t.msg_type_name = MSG_TYPE_INTEGER_32;
	msg_xmt->t.msg_type_size = 32;
	msg_xmt->t.msg_type_number = data_size;
	msg_xmt->t.msg_type_inline = inline;
	msg_xmt->t.msg_type_longform = FALSE;
	msg_xmt->t.msg_type_deallocate = FALSE;

	if (inline)
		msg_xmt->d.inline_data[0] = i;
	 else {
		foo[20] = i;
		msg_xmt->d.out_of_line_data = foo;
	}
}





port_t		LookFor(name)
	env_name_t	name;
{
	port_t		result;
	kern_return_t	error;
	int		tries = 0;
	
	do {
		if ((error = env_get_port(environment_port, name, 
			&result)) != KERN_SUCCESS) {
			if (tries++ > 10) {
				mach_error("CHILD: env_get_port returned ",
					error);
				exit(1);
			}
			sleep(2);
		}
	} while (error != KERN_SUCCESS);
#if !TIMING
	printf("CHILD: Successful env_get_port.\n");
#endif !TIMING
	return(result);
}




	
port_t		Register(name)
	env_name_t		name;
{
	port_t		result;
	kern_return_t	error;
	int		tries = 0;
	
	if ((error = port_allocate(task_self(), &result)) != KERN_SUCCESS) {
		printf("PARENT: port_allocate failed with %d: %s\n", 
			error, mach_error_string(error));
		exit(1);
	}
	
#if !TIMING
	printf("PARENT: Successful port_allocate.\n");
#endif !TIMING

	do {
		if ((error = env_set_port(environment_port, name, 
			result)) != KERN_SUCCESS) {
			if (tries++ > 10) {
				mach_error("PARENT: env_set_port ",
					error);
				exit(1);
			}
			sleep(2);
		}
	} while (error != KERN_SUCCESS);
#if !TIMING
	printf("PARENT: Successful env_set_port.\n");
#endif !TIMING
	return(result);
}




double	time_diff(t2, t1)
	struct	timeval	*t1, *t2;
{
	return((t2->tv_usec - t1->tv_usec) * 1e-06 +
		(t2->tv_sec - t1->tv_sec));
}




#if TIMING
void Summarize(iterations, t1, t2, r1, r2)
	int		iterations;
	struct timeval	*t1, *t2;
	struct rusage	*r1, *r2;
{
	double		sr = iterations;
	
	printf("Elapsed time\t%9.6lf seconds\n", time_diff(t2, t1) / sr);
	printf("User time\t%9.6lf seconds\n",
		time_diff(&r2->ru_utime, &r1->ru_utime) / sr);
	printf("System time\t%9.6lf seconds\n",
		time_diff(&r2->ru_stime, &r1->ru_stime) / sr);
}
#endif TIMING


void help_routine()

{
	printf("The following options are available: \n");
	printf("\n");
	printf("port <name>: give the port a specified name \n");
	printf("\t\tUse this option if sender and receiver are two\n");
	printf("\t\tdifferent processes.\n");
	printf("size <number>: size of the data sent in the msg \n");
	printf("iterations <number>: number of message transfers timed \n");
	printf("\t\tdefault value is 1\n");
	printf("backlog <number>: default value is 0.  This calls\n");
	printf("\t\tport_set_backlog for the port being used.\n");
	printf("complex: inline is false, deallocate is true\n");
	printf("inline: inline is true, deallocate is false\n");
	printf("clutter: deallocate is false\n");
	printf("paired: sends msg back signalling receipt of primary msg\n");
	printf("sender: operates as the sender only\n");
	printf("receiver: operates as the receiver only\n");
	printf("timeout: sets the receiver's option parameter to receive\n");
	printf("\t\tto be RCV_TIMEOUT, and the timeout to be 60000 \n");
	printf("\t\tmilliseconds.\n");
	printf("rpc: sender uses rpc, receiver is as if option paired \n");
	printf("\t\twas used, it does a receive and then a send.\n");
	printf("\n");
	printf("example: ipc_test port test_port size 70 iterations 100\n");
	printf("\n");
	printf("Note if you are running two processes, one as the sender\n");
	printf("and the other as the receiver, you must use the port \n");
	printf("option for both processes with the same name.\n");
}


main (argc, argv)
	int		argc;
	char		**argv;
{
#if TIMING
	struct rusage	rusage;
	struct rusage	rusage2;
	struct timeval	tp;
	struct timeval	tp2;
	struct timezone	tzp;
#endif TIMING

	int		i;
	msg_return_t	ret;
	kern_return_t	error;
	boolean_t	is_sender;
	boolean_t	both_sides = TRUE;
	env_name_t	port_name;
	int		backlog = 0;
	union wait	status;
	
	/*
	 *	Look for any of many options.
	 */

#define	streql(a,b)	(strcmp(a,b) == 0)

	while (*++argv != (char *) 0) {
		if (streql(*argv, "port")) 
			strcpy(port_name, *++argv);
		 else if (streql(*argv, "size"))
		 	data_size = atoi(*++argv);
		 else if (streql(*argv, "iterations"))
		 	iterations = atoi(*++argv);
		 else if (streql(*argv, "backlog"))
		 	backlog = atoi(*++argv);
		 else if (streql(*argv, "complex"))
		 	inline = FALSE,	dealloc = TRUE;
		 else if (streql(*argv, "inline"))
		 	inline = TRUE, dealloc = FALSE;
		 else if (streql(*argv, "clutter"))
		 	dealloc = FALSE;
		 else if (streql(*argv, "paired"))
		 	paired = TRUE;
		 else if (streql(*argv, "sender"))
		 	both_sides = FALSE, is_sender = TRUE;
		 else if (streql(*argv, "receiver"))
		 	both_sides = FALSE, is_sender = FALSE;
		 else if (streql(*argv, "timeout"))
		 	rcv_option = RCV_TIMEOUT;
		 else if (streql(*argv, "rpc"))
		 	use_rpc = paired = TRUE;
		 else if (streql(*argv, "help") || streql(*argv, "-h") ||
			streql (*argv, "-help")) {
			help_routine();
			exit(1);
		}
	}

	if (streql(port_name, "")) {
		sprintf(port_name, "ipc_test_%d", getpid());
	}

	if (both_sides)
		is_sender = (fork() == 0);

	if (data_size == 0)
		data_size = inline ? 1 : MAXDATA;

	/*
	 *	Create/find a port.
	 */

	if ((my_port = is_sender ? LookFor(port_name) :
		Register(port_name)) == PORT_NULL)
		exit(1);

	if (backlog != 0)
		(void) port_set_backlog(task_self(), my_port, backlog);

	if (is_sender && !inline) {
	 	vm_address_t	where = 0;

		if ((error = vm_allocate(task_self(), &where, data_size * 
			sizeof(int), TRUE)) != KERN_SUCCESS) {
			mach_error("CHILD: port_allocate returned ",
				error);
			exit(1);
		}
		foo = (int *) where;
	}

	msg_rcv.h.msg_size = sizeof(msg_rcv);

	/*
	 *	Perform an initial exchange before timing, so
	 *	we know that both parties are alive and well.
	 */

#if TIMING
	if (is_sender) {
		setup_request(&msg_xmt, 0x12345678);
		if ((ret = msg_send(&msg_xmt.h, MSG_OPTION_NONE, 
			0)) != SEND_SUCCESS) {
			mach_error("CHILD: msg_send returned ", ret);
			exit(1);
		}
		if (paired) {
			msg_rcv.h.msg_local_port = thread_reply();
			if ((ret = msg_receive(&msg_rcv.h, MSG_OPTION_NONE, 
				0)) != RCV_SUCCESS) {
				mach_error("CHILD: msg_receive returned ",
					ret);
				exit(1);
			}
		}
	} else {
		msg_rcv.h.msg_local_port = my_port;
		if ((ret = msg_receive(&msg_rcv.h, MSG_OPTION_NONE, 
			0)) != RCV_SUCCESS) {
			mach_error("PARENT: msg_receive returned ", ret);
			exit(1);
		}

		if (dealloc)
			if ((ret = vm_deallocate(task_self(), (vm_address_t) 
				msg_rcv.d.out_of_line_data,
				data_size*sizeof(int))) !=
				KERN_SUCCESS){
				mach_error("PARENT: vm_deallocate ",
					ret);
				exit(1);
			}

		if (paired) {
			setup_reply(&msg_xmt, &msg_rcv);
			if ((ret = msg_send(&msg_xmt.h, MSG_OPTION_NONE, 
				0)) != SEND_SUCCESS) {
				mach_error("PARENT: msg_send returned ",
					ret);
				exit(1);
			}
		}
	}

	/*
	 *	Start timing
	 */

	gettimeofday (&tp, &tzp);
	getrusage (RUSAGE_SELF, &rusage);
#endif TIMING

#if !TIMING
	if (is_sender) 
		setup_request(&msg_xmt, 0x12345678);
	else 
		msg_rcv.h.msg_local_port = my_port;
#endif !TIMING


	for (i = 1; i <= iterations; i++) {
		if (is_sender) {
			if (inline)
				msg_xmt.d.inline_data[0] = i;
			 else
				foo[20] = i;

			if (use_rpc) {
				if ((ret = msg_rpc(&msg_xmt.h, 
					MSG_OPTION_NONE, sizeof(msg_xmt), 0, 
					0)) != RPC_SUCCESS) {
					mach_error("CHILD: msg_rpc ",
						ret);
					exit(1);
				}
#if !TIMING
				printf("CHILD: Successful msg_rpc.\n");
#endif !TIMING

			} 
			else {  /* else to rpc */
				if ((ret = msg_send(&msg_xmt.h, 
					MSG_OPTION_NONE, 0)) != SEND_SUCCESS){
					mach_error("CHILD: msg_send ",
						ret);
					exit(1);
				}
#if !TIMING
				printf("CHILD: Successful msg_send.\n");
#endif !TIMING

				msg_rcv.h.msg_local_port = thread_reply();
				if (paired && ((ret = msg_receive(&msg_rcv.h,
					MSG_OPTION_NONE, 0)) != RCV_SUCCESS)){
					mach_error("CHILD: msg_receive ",
						ret);
					exit(1);
				}
#if !TIMING
				if (paired) {
					printf("CHILD: Successful ");
					printf("msg_receive.\n");
				}
#endif !TIMING
			}  /* if rpc else closing paren. */
		}  /* if sender */
		 else {
			int		x;

			msg_rcv.h.msg_size = sizeof(msg_rcv);
			if ((ret = msg_receive(&msg_rcv.h, rcv_option, 60 * 
				1000)) != RCV_SUCCESS) {
				mach_error("PARENT: msg_receive ", ret);
				exit(1);
			}
#if !TIMING
			printf("PARENT: Successful msg_receive.\n");
#endif !TIMING
			if ((x = (inline ? msg_rcv.d.inline_data[0] : 
				msg_rcv.d.out_of_line_data[20])) != i){
					printf("PARENT: error in data ");
					printf("transfer.\n");
					exit(1);
			}
			if (dealloc)
				if ((ret = vm_deallocate(task_self(),
					(vm_address_t) 
					msg_rcv.d.out_of_line_data,
					data_size*sizeof(int))) !=	
					KERN_SUCCESS){
					mach_error("PARENT: vm_deallocate ",
						ret);
					exit(1);
				}

			if (paired) {
				setup_reply(&msg_xmt, &msg_rcv);
				if ((ret = msg_send(&msg_xmt.h, 
					MSG_OPTION_NONE, 0)) != SEND_SUCCESS){
					mach_error("PARENT: msg_send ",
						ret);
					exit(1);
				}
#if !TIMING
				printf("PARENT: Successful msg_send.\n");
#endif !TIMING
			}
		}
	}

	/*
	 *	Stop timing, print results.
	 */

#if TIMING
	gettimeofday (&tp2, tzp);
	getrusage (RUSAGE_SELF, &rusage2);
#endif TIMING


/*	if ((!paired && !is_sender) || (paired && is_sender)) {  */
	if (! is_sender) {

		if ((ret = env_del_port(environment_port, port_name))
			!= KERN_SUCCESS) {
			mach_error("PARENT: env_del_port returned ", ret);
			exit(1);
		}
		printf("PARENT: Successful env_del_port.\n");
	
		if ((ret = port_deallocate(task_self(), my_port)) 
			!= KERN_SUCCESS) {
			mach_error("PARENT: port_deallocate returned ", 
				ret);
			exit(1);
		}
		printf("PARENT: Successful port_deallocate.\n");
	}
#if TIMING

	if (is_sender) 
		printf("SEND results\n");
	else {
	 	if (both_sides) {
			int		pid;
			(void) wait(&pid);
		}
		printf("RECEIVE results\n");
	}
	Summarize(iterations, &tp, &tp2, &rusage, &rusage2);
#endif TIMING


	if (is_sender) {
		printf("CHILD: Finished successfully.\n");
		exit(0);
	}
	else {
		printf("PARENT: Finished successfully.\n");
		exit(wait(&status) ? status.w_retcode : 1);
	}		
}    
