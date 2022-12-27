/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)crt0.c	5.4 (Berkeley) 1/18/88";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * $Log:	crt0.c,v $
 * Revision 1.2  89/01/13  15:44:30  mrt
 * picked up /usr/cs/lib/libc/VAX version with Mach changes
 * 
 * Revision 1.2  88/11/17  18:48:17  gm0w
 * 	Added MACH support.
 * 	[88/11/17            gm0w]
 * 
 **********************************************************************
 */
/*
 *	C start up routine.
 *	Robert Henry, UCB, 20 Oct 81
 *
 *	We make the following (true) assumptions:
 *	1) when the kernel calls start, it does a jump to location 2,
 *	and thus avoids the register save mask.  We are NOT called
 *	with a calls!  see sys1.c:setregs().
 *	2) The only register variable that we can trust is sp,
 *	which points to the base of the kernel calling frame.
 *	Do NOT believe the documentation in exec(2) regarding the
 *	values of fp and ap.
 *	3) We can allocate as many register variables as we want,
 *	and don't have to save them for anybody.
 *	4) Because of the ways that asm's work, we can't have
 *	any automatic variables allocated on the stack, because
 *	we must catch the value of sp before any automatics are
 *	allocated.
 */

char **environ = (char **)0;
#ifdef paranoid
static int fd;
#endif paranoid

#if	MACH
int	(*mach_init_routine)();
int	(*_cthread_init_routine)();
int	(*_cthread_exit_routine)();
int	(*_StrongBox_init_routine)();
extern int exit();
#endif	MACH
asm("#define _start start");
asm("#define _eprol eprol");
extern	unsigned char	etext;
extern	unsigned char	eprol;
start()
{
	struct kframe {
		int	kargc;
		char	*kargv[1];	/* size depends on kargc */
		char	kargstr[1];	/* size varies */
		char	kenvstr[1];	/* size varies */
	};
	/*
	 *	ALL REGISTER VARIABLES!!!
	 */
	register int r11;		/* needed for init */
	register struct kframe *kfp;	/* r10 */
	register char **targv;
	register char **argv;
	extern int errno;

#ifdef lint
	kfp = 0;
	initcode = initcode = 0;
#else not lint
	asm("	movl	sp,r10");	/* catch it quick */
#endif not lint
	for (argv = targv = &kfp->kargv[0]; *targv++; /* void */)
		/* void */ ;
	if (targv >= (char **)(*argv))
		--targv;
	environ = targv;
#if	MACH
	if (mach_init_routine)
		(void) mach_init_routine();
#endif	MACH
asm("eprol:");

#ifdef paranoid
	/*
	 * The standard I/O library assumes that file descriptors 0, 1, and 2
	 * are open. If one of these descriptors is closed prior to the start 
	 * of the process, I/O gets very confused. To avoid this problem, we
	 * insure that the first three file descriptors are open before calling
	 * main(). Normally this is undefined, as it adds two unnecessary
	 * system calls.
	 */
	do	{
		fd = open("/dev/null", 2);
	} while (fd >= 0 && fd < 3);
	close(fd);
#endif paranoid

#ifdef MCRT0
	monstartup(&eprol, &etext);
#endif MCRT0
	errno = 0;
#if	MACH
	if (_cthread_init_routine) (*_cthread_init_routine)();
	if (_StrongBox_init_routine) (*_StrongBox_init_routine)();
	(* (_cthread_exit_routine ? _cthread_exit_routine : exit))
		(main(kfp->kargc, argv, targv));
#else	MACH
	exit(main(kfp->kargc, argv, environ));
#endif	MACH
}
asm("#undef _start");
asm("#undef _eprol");

#ifdef MCRT0
/*ARGSUSED*/
exit(code)
	register int code;	/* r11 */
{
	monitor(0);
	_cleanup();
	_exit(code);
}
#endif MCRT0

#ifdef CRT0
/*
 * null mcount and moncontrol,
 * just in case some routine is compiled for profiling
 */
moncontrol(val)
	int val;
{

}
asm("	.globl	mcount");
asm("mcount:	rsb");
#endif CRT0
