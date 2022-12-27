/* @(#)rex.h	1.3 87/03/16 NFSSRC */
# ifdef lint
static char sccsid[] = "@(#)rex.h 1.1 86/09/25 Copyr 1985 Sun Micro";
# endif lint

/*
 * rex - remote execution server definitions
 **********************************************************************
 * HISTORY
 * 25-May-89  Paul Parker (parker) at Carnegie-Mellon University
 *	Changed ttysize conditional to not define at all as it is defined
 *	always in CMUCS sys/ioctl.h.
 *
 * 21-Feb-89  Paul Parker (parker) at Carnegie-Mellon University
 *	Conditionalized struct ttysize under "sun" as CMUCS sys/ioctl.h
 *	defines it under "sun".
 *
 * $Log:	rex.h,v $
 * Revision 2.0  89/06/15  16:43:25  dimitris
 *   Organized into a misc collection and SSPized
 * 
 *
 **********************************************************************
 *
 * Copyright (c) 1985 Sun Microsystems, Inc.
 */

#define	REXPROG		100017
#define	REXPROC_NULL	0	/* no operation */
#define	REXPROC_START	1	/* start a command */
#define	REXPROC_WAIT	2	/* wait for a command to complete */
#define	REXPROC_MODES	3	/* send the tty modes */
#define REXPROC_WINCH	4	/* signal a window change */
#define REXPROC_SIGNAL	5	/* other signals */

#define	REXVERS	1

/* flags for rst_flags field */
#define REX_INTERACTIVE		1	/* Interative mode */

struct rex_start {
  /*
   * Structure passed as parameter to start function
   */
	char	**rst_cmd;	/* list of command and args */
	char	*rst_host;	/* working directory host name */
	char	*rst_fsname;	/* working directory file system name */
	char	*rst_dirwithin;	/* working directory within file system */
	char	**rst_env;	/* list of environment */
	u_short	rst_port0;	/* port for stdin */
	u_short	rst_port1;	/* port for stdin */
	u_short	rst_port2;	/* port for stdin */
	u_long	rst_flags;	/* options - see #defines above */
};

bool_t xdr_rex_start();

struct rex_result {
  /*
   * Structure returned from the start function
   */
   	int	rlt_stat;	/* integer status code */
	char	*rlt_message;	/* string message for human consumption */
};
bool_t xdr_rex_result();

struct rex_ttymode {
    /*
     * Structure sent to set-up the tty modes
     */
	struct sgttyb basic;	/* standard unix tty flags */
	struct tchars more;	/* interrupt, kill characters, etc. */
	struct ltchars yetmore;	/* special Bezerkeley characters */
	u_long andmore;		/* and Berkeley modes */
};

bool_t xdr_rex_ttymode();
bool_t xdr_rex_ttysize();

#if	CMUCS
#else	/* CMUCS */
struct ttysize {
	int     ts_lines;       /* number of lines on terminal */
	int     ts_cols;        /* number of columns on terminal */
};
#endif	/* CMUCS */
