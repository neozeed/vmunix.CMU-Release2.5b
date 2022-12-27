/*
 * program to finish working on programs
 *
 **********************************************************************
 * HISTORY
 * $Log:	workoff.c,v $
 * Revision 2.4  89/06/18  16:21:16  gm0w
 * 	Added "-no <pass>" switch to disable individual workoff passes.
 * 	Added confirmation to abort for continuation during errors.
 * 	[89/06/18            gm0w]
 * 
 * Revision 2.3  89/02/06  15:09:26  gm0w
 * 	Added code to print reldir.
 * 	[89/02/06            gm0w]
 * 
 * Revision 2.2  89/01/28  23:06:49  gm0w
 * 	Created.
 * 	[88/11/27            gm0w]
 * 
 **********************************************************************
 */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <libc.h>
extern char *concat();
#include <c.h>
#include "project.h"

#define	MAXCMDARGS	16

#define ECHO	"/bin/echo"

char *proj_project_base;
char *proj_rcs_base;
char *proj_rcs_owner;
char *proj_rcs_cover;
char *proj_source_base;
char *proj_shadow_base;
char *proj_shadow_rcs_base;
char *proj_shadow_source_base;

char *prog;

char *project;
struct project proj;

char *branch_name;
char branch[BUFSIZ];
char branch_pref[BUFSIZ];
int preflen;
char *trunk;
int debug;
int quiet;
int allflag;
int autoflag;
char *allb[MAXCMDARGS+3];
char **files;
int nfiles;
char curdirbuf[MAXPATHLEN];
char *curdir;
char reldirbuf[MAXPATHLEN];
char *reldir;

int cmd_flags;
#define	BRANCH_CI	001
#define	UNLOCK		002
#define	MERGE		004
#define	TRUNK_CI	010
#define	CHECKOUT	020
#define	OUTDATE		040

exists(path)
char *path;
{
    struct stat statb;

    return(stat(path, &statb));
}

main(argc, argv)
int argc;
char **argv;
{
    char buf[MAXPATHLEN];
    char *ptr;
    int status;
    FILE *fp;
    int i;
    char *av[MAXCMDARGS];

    /*
     * parse command line
     */
    parse_command_line(argc, argv);

    /*
     * read project description
     */
    if (parse_project(project, &proj) != 0)
	quit(1, "%s: unable to parse project\n", prog);

    /*
     * check project description
     */
    check_project();

    /*
     * cd to correct directory
     * remember path from base to current directory
     */
    if (proj_shadow_source_base != NULL)
	ptr = proj_shadow_source_base;
    else
	ptr = proj_source_base;
    if ((curdir = getwd(curdirbuf)) == NULL)
	quit(1, "%s: getwd: %s\n", prog, curdirbuf);
    if (chdir(ptr) < 0)
	quit(1, "%s: chdir %s: %s\n", prog, ptr, errmsg(-1));
    if (getwd(buf) == NULL)
	quit(1, "%s: getwd: %s\n", buf);
    if (strncmp(buf, curdir, strlen(buf)) != 0) {
	printf("[ current directory not in source base ]\n");
	curdir = NULL;
    } else {
	curdir += strlen(buf);
	if (*curdir != '\0' && *curdir != '/')
	    curdir = NULL;
	else
	    while (*curdir == '/')
		curdir++;
    }

    /*
     * Setup for BCS
     */
    if (setenv("BCSBBASE", buf, 1) < 0)
	quit(1, "%s: BCSBBASE setenv failed\n", prog);
    if (!quiet)
	printf("[ BCSBBASE %s ]\n", buf);
    if (setenv("BCSSBASE", proj_source_base, 1) < 0)
	quit(1, "%s: BCSSBASE setenv failed\n", prog);
    if (!quiet)
	printf("[ BCSSBASE %s ]\n", proj_source_base);
    if (proj_shadow_rcs_base != NULL)
	ptr = concat(buf, sizeof(buf), proj_shadow_rcs_base, ":", NULL);
    else
	ptr = buf;
    (void) concat(ptr, buf+sizeof(buf)-ptr, proj_rcs_base, NULL);
    if (setenv("BCSVBASE", buf, 1) < 0)
	quit(1, "%s: BCSVBASE setenv failed\n", prog);
    if (!quiet)
	printf("[ BCSVBASE %s ]\n", buf);
    if (proj_rcs_owner != NULL) {
	if (setenv("RCS_AUTHCOVER", proj_rcs_cover, 1) < 0)
	    quit(1, "%s: RCS_AUTHCOVER setenv failed\n", prog);
	if (setenv("AUTHCOVER_USER", proj_rcs_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	if (setenv("AUTHCOVER_TESTDIR", proj_rcs_base, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	if (!quiet)
	    printf("[ RCS_AUTHCOVER %s (owner %s) ]\n",
		   proj_rcs_cover, proj_rcs_owner);
    }
    if (exists(".BCSVBASE") < 0) {
	if (!quiet)
	    printf("[ running bcs to create .BCSVBASE ]\n");
	i = 0;
	if (debug)
	    av[i++] = "echo";
	av[i++] = "bcs";
	if (debug)
	    av[i++] = "-debug";
	if (quiet)
	    av[i++] = "-quiet";
	if (autoflag)
	    av[i++] = "-auto";
	av[i++] = "-branch";
	av[i++] = trunk;
	av[i++] = "-vbase";
	av[i++] = buf;
	av[i++] = "-newvbase";
	av[i++] = "-newconfig";
	av[i++] = NULL;
	i = 0;
	if (debug) {
	    (void) runv(ECHO, av);
	    i++;
	}
	if ((status = runvp("bcs", &av[i])) != 0)
	    quit(1, "%s: runvp(bcs) status %d\n", prog, status);
    }
    (void) concat(buf, sizeof(buf), ".BCSconfig-", trunk, NULL);
    if (exists(buf) < 0) {
	i = 0;
	if (debug)
	    av[i++] = "echo";
	av[i++] = "bcs";
	if (debug)
	    av[i++] = "-debug";
	if (quiet)
	    av[i++] = "-quiet";
	if (autoflag)
	    av[i++] = "-auto";
	av[i++] = "-branch";
	av[i++] = trunk;
	av[i++] = "-newconfig";
	av[i++] = NULL;
	i = 0;
	if (debug) {
	    (void) runv(ECHO, av);
	    i++;
	}
	if ((status = runvp("bcs", &av[i])) != 0)
	    quit(1, "%s: runvp(bcs) status %d\n", prog, status);
    }
    (void) concat(buf, sizeof(buf), ".BCSpath-", branch, NULL);
    if ((fp = fopen(buf, "r")) != NULL) {
	reldir = fgets(reldirbuf, sizeof(reldirbuf), fp);
	if (reldir != NULL) {
	    if ((ptr = index(reldir, '\n')) != NULL)
		*ptr = '\0';
	    if (*reldir == '.' && *(reldir+1) == '\0')
		reldir = NULL;
	}
	(void) fclose(fp);
    } else
	reldir = NULL;
    if (!quiet && reldir != NULL)
	printf("[ relative directory %s ]\n", reldir);

    /*
     * Create Makeconf (if necessary)
     */
    if (proj_shadow_source_base != NULL && exists("Makeconf") < 0) {
	if (!quiet)
	    printf("[ linking ./Makeconf ]\n");
	(void) concat(buf, sizeof(buf),
		      proj_source_base, "/Makeconf", NULL);
	if (symlink(buf, "Makeconf") < 0)
	    quit(1, "%s: symlink %s: %s\n", prog, buf, errmsg(-1));
    }

    /*
     * Generate final list of files
     */
    make_filelist();

    /*
     * Make changes for branch
     */

    if (!quiet)
	printf("[ branch %s ]\n", branch);
    if (setenv("BCSBRANCH", branch, 1) < 0)
	quit(1, "%s: BCSBRANCH setenv failure\n", prog);
    (void) concat(buf, sizeof(buf), ".BCSconfig-", branch, NULL);
    if (exists(buf) < 0)
	quit(1, "%s: branch %s does not exist\n", prog, branch);

    /*
     * Execute branch commands
     */
    branch_cmds();

    exit(0);
}

parse_command_line(argc, argv)
int argc;
char **argv;
{
    char *up;

    if (argc > 0) {
	if ((prog = rindex(argv[0], '/')) != NULL)
	    prog++;
	else
	    prog = argv[0];
	argc--; argv++;
    } else
	prog = "project";

    if (argc == 0)
	usage((char *)NULL);

    if ((up = getenv("USER")) == NULL)
	quit(1, "%s: USER not found in environment\n", prog);
    (void) concat(branch_pref, sizeof(branch_pref), up, "_", NULL);
    preflen = strlen(branch_pref);

    trunk = "TRUNK";
    autoflag = FALSE;
    allflag = FALSE;
    debug = FALSE;
    quiet = FALSE;
    cmd_flags = (BRANCH_CI|MERGE|TRUNK_CI|CHECKOUT|OUTDATE);

    while (argc > 0) {
	if (argv[0][0] != '-')
	    break;
	switch (argv[0][1]) {
	case 'd':
	    if (strcmp(argv[0], "-debug") == 0)
		debug = TRUE;
	    else
		usage(argv[0]);
	    break;
	case 'q':
	    if (strcmp(argv[0], "-quiet") == 0)
		quiet = TRUE;
	    else
		usage(argv[0]);
	    break;
	case 'p':
	    if (strcmp(argv[0], "-project") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		project = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'b':
	    if (strcmp(argv[0], "-branch") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		branch_name = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 't':
	    if (strcmp(argv[0], "-trunk") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		trunk = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'a':
	    if (strcmp(argv[0], "-all") == 0)
		allflag = TRUE;
	    else if (strcmp(argv[0], "-auto") == 0)
		autoflag = TRUE;
	    else
		usage(argv[0]);
	    break;
	case 'u':
	    if (strcmp(argv[0], "-unlock") == 0)
		cmd_flags |= UNLOCK;
	    else
		usage(argv[0]);
	    break;
	case 'n':
	    if (strcmp(argv[0], "-no") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		if (strcmp(argv[0], "branch_ci") == 0)
		    cmd_flags &= ~BRANCH_CI;
		else if (strcmp(argv[0], "merge") == 0)
		    cmd_flags &= ~MERGE;
		else if (strcmp(argv[0], "trunk_ci") == 0)
		    cmd_flags &= ~TRUNK_CI;
		else if (strcmp(argv[0], "checkout") == 0)
		    cmd_flags &= ~CHECKOUT;
		else if (strcmp(argv[0], "outdate") == 0)
		    cmd_flags &= ~OUTDATE;
		else
		    quit(1, "%s: use branch_ci, merge, trunk_ci, checkout or outdate with -no\n", prog);
	    } else
		usage(argv[0]);
	    break;
	default:
	    usage(argv[0]);
	}
	argc--; argv++;
    }

    /*
     * Check branch name
     */
    if (branch_name == NULL)
	branch_name = getenv("BCSBRANCH");
    if (branch_name == NULL)
	quit(1, "%s: no branch specified\n", prog);
    if ((*branch_name < 'A' || *branch_name > 'Z') &&
	strncmp(branch_name, branch_pref, preflen) != 0)
	(void) concat(branch, sizeof(branch), branch_pref, branch_name, NULL);
    else
	(void) strcpy(branch, branch_name);

    /*
     * check argument list consistancy
     */
    if (argc == 0) {
	if (!allflag) {
	    fprintf(stderr, "%s: must specify -all or file list\n", prog);
	    usage((char *)NULL);
	}
	return;
    }

    if (allflag) {
	fprintf(stderr, "%s: must specify either -all or file list\n", prog);
	usage((char *)NULL);
    }

    files = argv;
    nfiles = argc;

    while (argc > 0) {
	if (strcmp(argv[0], "-all")  == 0)
	    usage((char *)NULL);
	argc--; argv++;
    }
}

usage(opt)
char *opt;
{
    if (opt != NULL)
	fprintf(stderr, "%s: unknown switch %s\n", prog, opt);
    quit(1,
	 "usage: %s [ -project <project> ] [ -branch <branch> ] [ -auto ] [ -all | files ...]\n",
	 prog);
}

check_project()
{
    char temp[MAXPATHLEN];
    struct field *field_p;
    struct arg_list *args_p;

    if (project_field(&proj, "project_base", &field_p) != 0)
	proj_project_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper project_base\n", prog);
	proj_project_base = args_p->tokens[0];
	if (*proj_project_base != '/')
	    quit(1, "%s: project_base must be absolute\n", prog);
    }
    if (project_field(&proj, "rcs_base", &field_p) != 0)
	quit(1, "%s: rcs_base not defined\n", prog);
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	quit(1, "%s: improper rcs_base\n", prog);
    proj_rcs_base = args_p->tokens[0];
    if (*proj_rcs_base != '/') {
	if (proj_project_base == NULL)
	    quit(1, "%s: project_base not defined, rcs_base relative\n",
		 prog);
	(void) concat(temp, sizeof(temp),
		      proj_project_base, "/", proj_rcs_base, NULL);
	if ((proj_rcs_base = salloc(temp)) == NULL)
	    quit(1, "%s: rcs_base salloc failure\n", prog);
    }
    if (exists(proj_rcs_base) < 0)
	quit(1, "%s: rcs_base %s not found\n", prog, proj_rcs_base);
    (void) concat(temp, sizeof(temp), proj_rcs_base, "/Makeconf,v", NULL);
    if (exists(temp) < 0)
	quit(1, "%s: no rcs Makeconf file\n", prog);
    if (project_field(&proj, "rcs_owner", &field_p) != 0)
	proj_rcs_owner = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper rcs_owner\n", prog);
	proj_rcs_owner = args_p->tokens[0];
	if (project_field(&proj, "rcs_cover", &field_p) != 0)
	    quit(1, "%s: rcs_cover not defined\n", prog);
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper rcs_cover\n", prog);
	proj_rcs_cover = args_p->tokens[0];
	if (*proj_rcs_cover != '/') {
	    if (proj_project_base == NULL)
		quit(1, "%s: project_base not defined, rcs_cover relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_rcs_cover, NULL);
	    if ((proj_rcs_cover = salloc(temp)) == NULL)
		quit(1, "%s: rcs_cover salloc failure\n", prog);
	}
	if (access(proj_rcs_cover, X_OK) < 0)
	    quit(1, "%s: rcs_cover %s not executable\n",
		 prog, proj_rcs_cover);
    }
    if (project_field(&proj, "source_base", &field_p) != 0)
	quit(1, "%s: source_base not defined\n", prog);
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	quit(1, "%s: improper source_base\n", prog);
    proj_source_base = args_p->tokens[0];
    if (*proj_source_base != '/') {
	if (proj_project_base == NULL)
	    quit(1, "%s: project_base not defined, source_base relative\n",
		 prog);
	(void) concat(temp, sizeof(temp),
		      proj_project_base, "/", proj_source_base, NULL);
	if ((proj_source_base = salloc(temp)) == NULL)
	    quit(1, "%s: source_base salloc failure\n", prog);
    }
    if (exists(proj_source_base) < 0)
	quit(1, "%s: source_base %s not found\n", prog, proj_source_base);
    (void) concat(temp, sizeof(temp), proj_source_base, "/Makeconf", NULL);
    if (exists(temp) < 0)
	quit(1, "%s: no source Makeconf file\n", prog);
    if (project_field(&proj, "shadow_base", &field_p) != 0)
	proj_shadow_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper shadow_base\n", prog);
	proj_shadow_base = args_p->tokens[0];
	if (*proj_shadow_base != '/') {
	    if (proj_project_base == NULL)
		quit(1, "%s: project_base not defined, shadow_base relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_shadow_base, NULL);
	    if ((proj_shadow_base = salloc(temp)) == NULL)
		quit(1, "%s: shadow_base salloc failure\n", prog);
	}
	if (exists(proj_shadow_base) < 0)
	    quit(1, "%s: shadow_base %s not found\n", prog, proj_shadow_base);
    }
    if (project_field(&proj, "shadow_rcs_base", &field_p) != 0)
	proj_shadow_rcs_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper shadow_rcs_base\n", prog);
	proj_shadow_rcs_base = args_p->tokens[0];
	if (*proj_shadow_rcs_base != '/') {
	    if (proj_shadow_base == NULL && proj_project_base == NULL)
		quit(1, "%s: base not defined, shadow_rcs_base relative\n",
		     prog);
	    if (proj_shadow_base != NULL)
		(void) concat(temp, sizeof(temp),
			      proj_shadow_base, "/", proj_shadow_rcs_base,
			      NULL);
	    else
		(void) concat(temp, sizeof(temp),
			      proj_project_base, "/", proj_shadow_rcs_base,
			      NULL);
	    if ((proj_shadow_rcs_base = salloc(temp)) == NULL)
		quit(1, "%s: shadow_rcs_base salloc failure\n", prog);
	}
	if (strcmp(proj_shadow_rcs_base, proj_rcs_base) == 0)
	    quit(1, "%s: shadow_rcs_base and rcs_base match\n", prog);
    }
    if (project_field(&proj, "shadow_source_base", &field_p) != 0)
	proj_shadow_source_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper shadow_source_base\n", prog);
	proj_shadow_source_base = args_p->tokens[0];
	if (*proj_shadow_source_base != '/') {
	    if (proj_shadow_base == NULL && proj_project_base == NULL)
		quit(1, "%s: base not defined, shadow_source_base relative\n",
		     prog);
	    if (proj_shadow_base != NULL)
		(void) concat(temp, sizeof(temp),
			      proj_shadow_base, "/", proj_shadow_source_base,
			      NULL);
	    else
		(void) concat(temp, sizeof(temp),
			      proj_project_base, "/", proj_shadow_source_base,
			      NULL);
	    if ((proj_shadow_source_base = salloc(temp)) == NULL)
		quit(1, "%s: shadow_source_base salloc failure\n", prog);
	}
	if (strcmp(proj_shadow_source_base, proj_source_base) == 0)
	    quit(1, "%s: shadow_source_base and source_base match\n", prog);
    }
}

/*
 * generate the list of files being "worked off"
 */
make_filelist()
{
    char pathbuf[MAXPATHLEN];
    char filebuf[MAXPATHLEN];
    char **pp, *ptr;
    int i;

    if (allflag) {
	allb[MAXCMDARGS] = "-allb";
	allb[MAXCMDARGS+1] = branch;
	allb[MAXCMDARGS+2] = NULL;
	files = allb + MAXCMDARGS;
	return;
    }

    pp = (char **) malloc((unsigned)(MAXCMDARGS+nfiles+1)*sizeof(char *));
    if (pp == NULL)
	quit(1, "%s: unable to allocate argument buffer\n");
    pp += MAXCMDARGS;
    ptr = pathbuf;
    *ptr = '\0';
    if (curdir != NULL)
	ptr = concat(ptr, sizeof(pathbuf), curdir, ":", NULL);
    if (reldir != NULL)
	(void) concat(ptr, pathbuf+sizeof(pathbuf)-ptr, reldir, ":", NULL);
    for (i = 0; i < nfiles; i++) {
	ptr = files[i];
	if (*ptr == '/') {
	    if (exists(++ptr) < 0)
		quit(1, "%s: unable to locate %s\n", prog, ptr);
	} else if (*pathbuf == '\0') {
	    if (exists(ptr) < 0)
		quit(1, "%s: unable to locate %s\n", prog, ptr);
	} else {
	    if (searchp(pathbuf, ptr, filebuf, exists) < 0)
		quit(1, "%s: unable to locate %s\n", prog, ptr);
	    ptr = salloc(filebuf);
	}
	pp[i] = ptr;
    }
    pp[i] = NULL;
    files = pp;
}

check_status(msg, status)
char *msg;
int status;
{
    fprintf(stderr, "%s: %s: status %d\n", prog, msg, status);
    return(!getbool("Do you wish to continue?", 0));
}

branch_cmds()
{
    char buf[MAXPATHLEN];
    int status;
    char **av;

    if (allflag) {
	(void) concat(buf, sizeof(buf), ".BCSset-", branch, NULL);
	if (exists(buf) < 0) {
	    av = files;
	    *(--av) = "-o1-";
	    *(--av) = "-okwrite";
	    *(--av) = "-rm";
	    if (autoflag)
		*(--av) = "-auto";
	    if (quiet)
		*(--av) = "-quiet";
	    if (debug)
		*(--av) = "-debug";
	    *(--av) = "bcs";
	    if (debug) {
		*(--av) = "echo";
		(void) runv(ECHO, av++);
	    }
	    if ((status = runvp("bcs", av)) != 0) {
		if (check_status("runvp(bcs)", status))
		    exit(1);
	    }
	    return;
	}
    }

    /*
     * check in any changes
     */
    if (cmd_flags&BRANCH_CI) {
	av = files;
	if (autoflag)
	    *(--av) = "-auto";
	if (quiet)
	    *(--av) = "-quiet";
	if (debug)
	    *(--av) = "-debug";
	*(--av) = "bci";
	if (debug) {
	    *(--av) = "echo";
	    (void) runv(ECHO, av++);
	}
	if ((status = runvp("bci", av)) != 0) {
	    if (check_status("runvp(bci)", status))
		exit(1);
	}
    }

    /*
     * unlock branch and trunk versions
     */
    if (cmd_flags&UNLOCK) {
	av = files;
	*(--av) = "-u";
	if (autoflag)
	    *(--av) = "-auto";
	if (quiet)
	    *(--av) = "-quiet";
	if (debug)
	    *(--av) = "-debug";
	*(--av) = "bcs";
	if (debug) {
	    *(--av) = "echo";
	    (void) runv(ECHO, av++);
	}
	if ((status = runvp("bcs", av)) != 0) {
	    if (check_status("runvp(bcs)", status))
		exit(1);
	}
	av = files;
	*(--av) = "-u";
	*(--av) = trunk;
	*(--av) = "-branch";
	if (autoflag)
	    *(--av) = "-auto";
	if (quiet)
	    *(--av) = "-quiet";
	if (debug)
	    *(--av) = "-debug";
	*(--av) = "bcs";
	if (debug) {
	    *(--av) = "echo";
	    (void) runv(ECHO, av++);
	}
	if ((status = runvp("bcs", av)) != 0) {
	    if (check_status("runvp(bcs)", status))
		exit(1);
	}
    }

    /*
     * merge branch version with the trunk
     */
    if (cmd_flags&MERGE) {
	(void) concat(buf, sizeof(buf), "-r", branch, NULL);
	av = files;
	*(--av) = buf;
	*(--av) = trunk;
	*(--av) = "-branch";
	if (autoflag)
	    *(--av) = "-auto";
	if (quiet)
	    *(--av) = "-quiet";
	if (debug)
	    *(--av) = "-debug";
	*(--av) = "bmerge";
	if (debug) {
	    *(--av) = "echo";
	    (void) runv(ECHO, av++);
	}
	if ((status = runvp("bmerge", av)) != 0) {
	    if (check_status("runvp(bmerge)", status))
		exit(1);
	}
    }

    /*
     * Check merged version back onto the trunk
     */
    if (cmd_flags&TRUNK_CI) {
	av = files;
	*(--av) = branch;
	*(--av) = "-xlog";
	*(--av) = trunk;
	*(--av) = "-branch";
	if (autoflag)
	    *(--av) = "-auto";
	if (quiet)
	    *(--av) = "-quiet";
	if (debug)
	    *(--av) = "-debug";
	*(--av) = "bci";
	if (debug) {
	    *(--av) = "echo";
	    (void) runv(ECHO, av++);
	}
	if ((status = runvp("bci", av)) != 0) {
	    if (check_status("runvp(bci)", status))
		exit(1);
	}
    }

    /*
     * checkout new sources into master source latest area
     */
    if (cmd_flags&CHECKOUT) {
	if (allflag) {
	    (void) concat(buf, sizeof(buf), ".BCSset-", branch, NULL);
	    if (debug)
		printf("build -cofile %s\n", buf);
	    if ((status = runp("build", "build", "-cofile", buf, NULL)) != 0) {
		if (check_status("runp(build)", status))
		    exit(1);
	    }
	} else {
	    av = files;
	    *(--av) = "-co";
	    *(--av) = "build";
	    if (debug) {
		*(--av) = "echo";
		(void) runv(ECHO, av++);
	    }
	    if ((status = runvp("build", av)) != 0) {
		if (check_status("runvp(build)", status))
		    exit(1);
	    }
	}
    }

    /*
     * no longer need shadow files
     */
    if (cmd_flags&OUTDATE) {
	av = files;
	*(--av) = "-o1-";
	*(--av) = "-okwrite";
	*(--av) = "-rm";
	if (autoflag)
	    *(--av) = "-auto";
	if (quiet)
	    *(--av) = "-quiet";
	if (debug)
	    *(--av) = "-debug";
	*(--av) = "bcs";
	if (debug) {
	    *(--av) = "echo";
	    (void) runv(ECHO, av++);
	}
	if ((status = runvp("bcs", av)) != 0) {
	    if (check_status("runvp(bcs)", status))
		exit(1);
	}
    }
}
