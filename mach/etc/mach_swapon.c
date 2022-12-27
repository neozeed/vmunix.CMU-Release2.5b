/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	mach_swapon.c,v $
 * Revision 1.3  89/05/05  18:31:13  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 * 25-May-88  Mary R. Thompson (mrt) @ Carnegie Mellon
 *	Added the program_name argument to the help printf
 *	Dereferenced argv in calls to arg_expected
 **********************************************************************
 */ 

#include <mach/mach_traps.h>
#include <mach/mach_types.h>
#include <strings.h>
#include <stdio.h>
#include <mach_error.h>

#include <fstab.h>
#include <sys/types.h>
#include <sys/stat.h>

#define		streql(a,b)	( strcmp((a), (b)) == 0 )


static
char		*program_name;

void		arg_expected(cmd)
	char		*cmd;
{
	fprintf(stderr, "%s:\t%s command requires an argument\n",
		program_name, cmd);
	exit(1);
}

dev_t		lookup(fs_name)
	char		*fs_name;
{
	struct stat	fs_stats;

	if (stat(fs_name, &fs_stats)) {
		perror(fs_name);
		exit(3);
	}
	if ((fs_stats.st_mode & S_IFMT) != S_IFBLK) {
		fprintf(stderr, "%s:\targument %s not a block device\n", program_name, fs_name);
		exit(4);
	}
	return(fs_stats.st_rdev);
}

void		kern_return_msg(a, fs_name, doing_what)
	kern_return_t	a;
	char		*fs_name;
	char		*doing_what;
{
	if (a != KERN_SUCCESS) {
		fprintf(stderr, "%s:\t%s on %s encountered error: %s\n",
			program_name, doing_what, fs_name, mach_error_string(a));
		exit(3);
	}
}

#define		prefer(fs_name)	kern_return_msg( \
					inode_swap_preference(lookup((fs_name)), TRUE, FALSE), \
					fs_name, "prefer")
#define		never(fs_name)	kern_return_msg( \
					inode_swap_preference(lookup((fs_name)), FALSE, TRUE), \
					fs_name, "never")
#define		allow(fs_name)	kern_return_msg( \
					inode_swap_preference(lookup((fs_name)), FALSE, FALSE), \
					fs_name, "allow")

int		main(argc, argv)
	int		argc;
	char		**argv;
{
	if (argc <= 0)
		return(-1);

	if (*(program_name = rindex(*argv, '/')) == '/')
		program_name++;
	 else
	 	program_name = *argv;

	if (argc <= 1) {
		fprintf(stderr, "Usage:\t%s [default] [prefer <device>] [allow <device>] [never <device>] ...\n",program_name);
		return(1);
	}

	while (*++argv != (char *) 0) {
		register
		char		*cmd = *argv;

		if (streql(cmd, "default")) {
			struct fstab	*fs;

			if (setfsent() == 0) {
				perror(FSTAB);
				return(2);
			}
			while ((fs = getfsent()) != (struct fstab *) 0)
				if (streql(fs->fs_type, FSTAB_SW))
					prefer(fs->fs_spec);
			(void) endfsent();
		} else if (streql(cmd, "prefer")) {
			if (*++argv == (char *) 0)
				arg_expected(*--argv);
			 else
			 	prefer(*argv);
		} else if (streql(cmd, "never")) {
			if (*++argv == (char *) 0)
				arg_expected(*--argv);
			 else
			 	never(*argv);
		} else if (streql(cmd, "allow")) {
			if (*++argv == (char *) 0)
				arg_expected(*--argv);
			 else
			 	allow(*argv);
		} else {
			fprintf(stderr, "%s:\tunknown command %s\n", program_name, cmd);
			return(1);
		}
	}
	return(0);
}
