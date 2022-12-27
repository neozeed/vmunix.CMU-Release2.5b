/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	uid.c,v $
 * Revision 1.2  89/06/08  18:16:09  mrt
 * 	Updated for Mach 2.5
 * 
 */
/*
 * Abstract:
 *	Random things to start a logged-in process on a vax
 *
 */
#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>

extern int errno;

#define LEN 32
char CurrentPassword[LEN] = "";	/* Don`t worry: only encripted */
char CurrentUserid[LEN]   = "";

static void
 EncryptPassword(password)
char password[];
{
	int             i;

	for (i = 0; password[i]; i++)
		password[i] = (password[i] ^ (((i + 1) * 11) % 128)) + 1;
}


int GetPassword(u)
{
	register char  *p;
	char            yn = '\0';
	extern char    *getpass();

	if (CurrentUserid[0] != '\0') {
		printf("Using previous user-id and password, right ? [y/n]");
		scanf("%c", &yn);
	}
	if (yn != 'y') {
		printf("login :");
		fgets(CurrentUserid, LEN, stdin);
		if (yn != '\0')
			fgets(CurrentUserid, LEN, stdin); /* scanf.. */
		CurrentUserid[strlen(CurrentUserid) - 1] = '\0';
		if (strlen(CurrentUserid) == 0)
			return (0);

		p = getpass("Password:");
		strncpy(CurrentPassword, p, LEN);

		EncryptPassword(CurrentPassword);
	}
	return (1);
}



