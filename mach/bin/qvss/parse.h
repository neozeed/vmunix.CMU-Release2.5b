/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	parse.h,v $
 * Revision 1.2  89/05/05  18:26:07  mrt
 * 	Cleanup for Mach 2.5
 * 
 */

struct cmd_entry {
	char *cmd;
	char **(*fn)();
	int flags;
	char *description;
};

/*		flag values		*/


struct cmd_entry *findcmd();
char **cmd_help();
