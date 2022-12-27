/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	qvss.c,v $
 * Revision 1.2  89/05/05  18:26:16  mrt
 * 	Cleanup for Mach 2.5
 * 
 *  4-Jun-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Slightly reformatted.
 *
 *
 */
/*
 * File:	qvss.c
 *
 * Abstract:
 * 	"stty" program for qvss options
 *
 * Author:	Robert V. Baron
 *		Copyright (c) 1985 by Robert V. Baron
 *		Created 09/23/85
 */
/*
 *		Program specific environment
 */

#include <vaxuba/qvioctl.h>

int qvssfd;
struct qv_info *qv_scn;
int debug = 0;

/*
 *		End Program Specific Environment
 */

#include <stdio.h>
#include "parse.h"

extern struct cmd_entry cmds[];

static struct cmd_entry ambiguous_cmd = {"ambiguous", (char **(*)()) -1, 0, ""};
static struct cmd_entry *ambiguous_cmd_entry = &ambiguous_cmd;

static struct cmd_entry not_found_cmd = {"notfound", (char **(*)()) -2, 0, ""};
static struct cmd_entry *not_found_cmd_entry = &not_found_cmd;

char ** cmd();

main(argc, argv)
	int argc;
	register char **argv;
{
	register char **argvend = &argv++[argc];

	/*
	 *	Program specific initialization
	 */

	register int status;

	qvssfd = open("/dev/console", 0);
	if (qvssfd < 0) {
		fprintf(stdout, "qvss: fd = %d.\n", qvssfd);
		perror("open qvscreen");
		exit(1);
	}

	status = ioctl(qvssfd, QIOCADDR, &qv_scn);
	if (status < 0) {
		printf("QIOCADDR: status = %d.\n", status);
		perror("QIOCADDR");
		exit(1);
	}

	/*
	 *	End Program specific initialization
	 */

	for ( argv ; argv < argvend ;) {
		register char *token = *argv++;

		if (*token != '-') {
			register struct cmd_entry *cmdp  = findcmd(token);

			argv = cmd(cmdp, argv, argvend);
		} else {
			parse_flags(token);
		}
	}
	exit (0);
}

char **cmd(cmdp, argv, argvend)
	register struct cmd_entry *cmdp;
	char **argv, **argvend;
{

	if (cmdp == ambiguous_cmd_entry) {
		fprintf(stderr, "qvss: \"%s\" Non unique command prefix.\n",
			argv[-1]);
	} else if (cmdp == not_found_cmd_entry) {
		fprintf(stderr, "qvss: \"%s\" Command not found.\n",
			argv[-1]);
	} else {
		if (debug)
			fprintf(stderr, "qvss: calling \"%s\"\n", cmdp->cmd);
		argv = (cmdp->fn)(cmdp, argv, argvend);
	}

	return(argv);
}

/*
 * 	Parsing Functions
 */

#include <ctype.h>

struct cmd_entry *findcmd(str)
	register char *str;
{
	register struct cmd_entry *cmdp = cmds;
	register struct cmd_entry *ansp = (struct cmd_entry *) 0;
	register char *cp = str;
	register int c;

	while (c = *cp) {
		if (isupper(c))
			*cp = tolower(c);
		cp++;
	}

	for (; *cmdp->cmd; cmdp++) {
		if (!strncmp(cmdp->cmd, str, strlen(str))) {
			if (ansp != (struct cmd_entry *) 0) {
				return (ambiguous_cmd_entry);
			}
			ansp = cmdp;
		}
	}
	if (ansp)
		return (ansp);
	else	
		return (not_found_cmd_entry);
}

char **cmd_help(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register struct cmd_entry *cmdp = cmds;

	for (; *cmdp->cmd; cmdp++) {

		printf("\t%s:\t%s\n",
			cmdp->cmd, cmdp->description);
	}
	return(++argv);
}

getnum(strp, num)
	register char *strp;
	int *num;
{
	register int ac = 0;
	register int base = 10;
	register int c;
	register int sign = 1;

	while ( (c =  *strp) && c == '\t' && c == ' ') strp++;
	if (!c)
		return (0);

	if (c == '-') {
		sign = -1;
		c = *strp++;
	}
	if (c == '0')
		switch ( c = *++strp) {
		case 't': case 'T':	base = 10 ; strp++ ; break;
		case 'x': case 'X':	base = 16 ; strp++ ; break;
		case '0': case '1': case '2':
		case '3': case '4':
		case '5': case '6': case '7':
					base = 8 ;	break;
		case 0:
			ac *= sign;
			*num = ac;
			return (1);
		default:
			return -1;
		}
		

	switch (base) {
	case 8:
		while (c = *strp++ ) {
			if (c <= '7' && c >= '0')
				ac <<=3 , ac += c - '0';
			else return -1;
		}
		break;
	case 10:
		while (c = *strp++ ) {
			if (c <= '9' && c >= '0')
				ac *=10 , ac += c - '0';
			else return -1;
		}
		break;
	case 16:
		while (c = *strp++ ) {
			if (c <= '9' && c >= '0')
				ac <<=4 , ac += c - '0';
			else if (c <= 'f' && c >= 'a')
				ac <<=4 , ac += c - 'a' + 10;
			else if (c <= 'F' && c >= 'A')
				ac <<=4 , ac += c - 'A' + 10;
			else return -1;
		}
		break;
	}

	ac *= sign;
	*num = ac;
	return (1);

}
