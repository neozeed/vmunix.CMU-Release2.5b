/*
 * program to create subshell for working on projects
 *
 **********************************************************************
 * HISTORY
 * $Log:	workon.c,v $
 * Revision 2.5  89/07/05  13:24:32  gm0w
 * 	Added "bcstemp" project description directive to set BCSTEMP in
 * 	the environment.
 * 	[89/07/05            gm0w]
 * 
 * Revision 2.4  89/03/12  13:46:02  gm0w
 * 	Added -debug support.
 * 	[89/03/12            gm0w]
 * 
 * Revision 2.3  89/02/19  11:18:17  gm0w
 * 	Added support for "project" description field.
 * 	[89/02/19            gm0w]
 * 
 * Revision 2.2  89/01/28  23:04:44  gm0w
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
#include <c.h>
#include "project.h"

#define	MAXCMDARGS	16

#define ECHO	"/bin/echo"

char *proj_project_base;
int proj_dosearch;
char *proj_rcs_base;
char *proj_rcs_owner;
char *proj_rcs_cover;
char *proj_source_base;
char *proj_object_base;
char *proj_shadow_base;
char *proj_shadow_rcs_base;
char *proj_shadow_source_base;
char *proj_shadow_object_base;
char *proj_bcstemp;

char *prog;

char *project;
struct project proj;

char reldir[MAXPATHLEN];
char *target_name;
char *branch_name;
char branch[BUFSIZ];
char branch_pref[BUFSIZ];
int preflen;
char *trunk;
int debug;

struct syspath {
    char *path;
    int pathlen;
} syspaths[] = {
    { "PATH",	4 },
    { "CPATH",	5 },
    { "LPATH",	5 },
    { "MPATH",	5 },
    { "EPATH",	5 },
    { NULL,	0 }
};

isdir(path)
char *path;
{
    struct stat statb;

    if (stat(path, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
	return(-1);
    return(0);
}

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
    char *path, *ptr;
    char buf[MAXPATHLEN];
    int status;
    int newpath;
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
     * set project for subshell
     */
    if (project != NULL && setenv("PROJECT", project, 1) < 0)
	quit(1, "%s: PROJECT setenv failed\n", prog);

    /*
     * create shadow paths if necessary
     */
    if (proj_shadow_source_base != NULL) {
	if (isdir(proj_shadow_source_base) < 0 &&
	    ckmakepath(proj_shadow_source_base, proj_source_base))
	    quit(1, "%s: makepath failed for %s\n",
		 prog, proj_shadow_source_base);
	if (chdir(proj_shadow_source_base) < 0)
	    quit(1, "%s: chdir %s: %s\n",
		 prog, proj_shadow_source_base, errmsg(-1));
	if (proj_shadow_object_base != NULL &&
	    isdir(proj_shadow_object_base) < 0 &&
	    ckmakepath(proj_shadow_object_base, proj_object_base))
	    quit(1, "%s: makepath failed for %s\n",
		 prog, proj_shadow_object_base);
    } else if (chdir(proj_source_base) < 0)
	quit(1, "%s: chdir %s: %s\n", prog, proj_source_base, errmsg(-1));

    /*
     * Setup for BCS
     */
    if ((path = getwd(buf)) == NULL)
	quit(1, "%s: getwd: %s\n", buf);
    if (setenv("BCSBBASE", buf, 1) < 0)
	quit(1, "%s: BCSBBASE setenv failed\n", prog);
    printf("[ BCSBBASE %s ]\n", buf);
    if (setenv("BCSSBASE", proj_source_base, 1) < 0)
	quit(1, "%s: BCSSBASE setenv failed\n", prog);
    printf("[ BCSSBASE %s ]\n", proj_source_base);
    if (proj_shadow_rcs_base != NULL)
	ptr = concat(buf, sizeof(buf), proj_shadow_rcs_base, ":", NULL);
    else
	ptr = buf;
    (void) concat(ptr, buf+sizeof(buf)-ptr, proj_rcs_base, NULL);
    if (setenv("BCSVBASE", buf, 1) < 0)
	quit(1, "%s: BCSVBASE setenv failed\n", prog);
    printf("[ BCSVBASE %s ]\n", buf);
    if (proj_rcs_owner != NULL) {
	if (setenv("RCS_AUTHCOVER", proj_rcs_cover, 1) < 0)
	    quit(1, "%s: RCS_AUTHCOVER setenv failed\n", prog);
	if (setenv("AUTHCOVER_USER", proj_rcs_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	if (setenv("AUTHCOVER_TESTDIR", proj_rcs_base, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	printf("[ RCS_AUTHCOVER %s (owner %s) ]\n",
	       proj_rcs_cover, proj_rcs_owner);
    }
    if (proj_bcstemp != NULL) {
	if (setenv("BCSTEMP", proj_bcstemp, 1) < 0)
	    quit(1, "%s: BCSTEMP setenv failed\n", prog);
	printf("[ BCSTEMP %s ]\n", proj_bcstemp);
    }

    if (exists(".BCSVBASE") < 0) {
	printf("[ running bcs to create .BCSVBASE ]\n");
	i = 0;
	if (debug)
	    av[i++] = "echo";
	av[i++] = "bcs";
	if (debug)
	    av[i++] = "-debug";
	av[i++] = "-branch";
	av[i++] = "TRUNK";
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
	    quit(1, "%s: runvp(bcs) status %#o\n", prog, status);
    }

    /*
     * Generate branch name from target
     */
    name_branch();
    newpath = (*reldir == '\0');

    /*
     * Generate the name of the target
     */
    find_target();

    /*
     * Change to relative source subdirectory
     */
    relative_subdir();
    if (newpath) {
	printf("[ running bcs to create .BCSpath-%s ]\n", branch);
	i = 0;
	if (debug)
	    av[i++] = "echo";
	av[i++] = "bcs";
	if (debug)
	    av[i++] = "-debug";
	av[i++] = "-branch";
	av[i++] = branch;
	av[i++] = "-newpath";
	av[i++] = NULL;
	i = 0;
	if (debug) {
	    (void) runv(ECHO, av);
	    i++;
	}
	if ((status = runvp("bcs", &av[i])) != 0)
	    quit(1, "%s: runvp(bcs) status %#o\n", prog, status);
    }

    /*
     * All done, exec work shell
     */
    if ((path = getenv("SHELL")) == NULL)
	ptr = path = "csh";
    else if ((ptr = rindex(path, '/')) != NULL)
	ptr++;
    else
	ptr = path;
    execlp(path, ptr, 0);
}

parse_command_line(argc, argv)
int argc;
char **argv;
{
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN];
    char *ptr;

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

    if ((ptr = getenv("USER")) == NULL)
	quit(1, "%s: USER not found in environment\n", prog);
    (void) concat(branch_pref, sizeof(branch_pref), ptr, "_", NULL);
    preflen = strlen(branch_pref);
    target_name = NULL;
    branch_name = NULL;
    trunk = "TRUNK";
    debug = FALSE;

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
	default:
	    usage(argv[0]);
	}
	argc--; argv++;
    }

    if (argc == 0) {
	if (branch_name == NULL) {
	    fprintf(stderr, "%s: must specify branch or target\n", prog);
	    usage((char *)NULL);
	}
	if ((*branch_name < 'A' || *branch_name > 'Z') &&
	    strncmp(branch_name, branch_pref, preflen) != 0)
	    (void) concat(branch, sizeof(branch),
			  branch_pref, branch_name, NULL);
	else
	    (void) strcpy(branch, branch_name);
	return;
    }

    if (argc > 1) {
	fprintf(stderr, "%s: too many arguments\n", prog);
	usage((char *)NULL);
    }

    target_name = argv[0];
    if (branch_name != NULL) {
	if ((*branch_name < 'A' || *branch_name > 'Z') &&
	    strncmp(branch_name, branch_pref, preflen) != 0)
	    (void) concat(branch, sizeof(branch),
			  branch_pref, branch_name, NULL);
	else
	    (void) strcpy(branch, branch_name);
    } else {
	path(target_name, dirbuf, filbuf);
	if ((ptr = index(filbuf, '.')) != NULL)
	    *ptr = '\0';
	if (*filbuf >= 'A' && *filbuf <= 'Z')
	    *filbuf = (*filbuf - 'A') + 'a';
	(void) concat(branch, sizeof(branch), branch_pref, filbuf, NULL);
    }
}

usage(opt)
char *opt;
{
    if (opt != NULL)
	fprintf(stderr, "%s: unknown switch %s\n", prog, opt);
    quit(1,
	 "usage: %s [ -project <project> ] [ -branch <branch> ] [ program ]\n",
	 prog);
}

check_project()
{
    char temp[MAXPATHLEN];
    struct field *field_p;
    struct arg_list *args_p;
    int i;

    if (project_field(&proj, "project", &field_p) == 0) {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper project\n", prog);
	project = args_p->tokens[0];
    }
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
    proj_dosearch = FALSE;
    if (project_field(&proj, "flags", &field_p) == 0) {
	for (args_p = field_p->args; args_p != NULL; args_p = args_p->next) {
	    if (args_p->ntokens == 0)
		quit(1, "%s: improper flags\n", prog);
	    for (i = 0; i < args_p->ntokens; i++)
		if (strcmp("dosearch", args_p->tokens[i]) == 0)
		    break;
	    if (i < args_p->ntokens) {
		proj_dosearch = TRUE;
		break;
	    }
	}
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
    if (isdir(proj_rcs_base) < 0)
	quit(1, "%s: rcs_base %s not a directory\n", prog, proj_rcs_base);
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
    if (isdir(proj_source_base) < 0)
	quit(1, "%s: source_base %s not a directory\n",
	     prog, proj_source_base);
    (void) concat(temp, sizeof(temp), proj_source_base, "/Makeconf", NULL);
    if (exists(temp) < 0)
	quit(1, "%s: no source Makeconf file\n", prog);
    if (project_field(&proj, "object_base", &field_p) != 0)
	proj_object_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper object_base\n", prog);
	proj_object_base = args_p->tokens[0];
	if (*proj_object_base != '/') {
	    if (proj_project_base == NULL)
		quit(1, "%s: project_base not defined, object_base relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_object_base, NULL);
	    if ((proj_object_base = salloc(temp)) == NULL)
		quit(1, "%s: object_base salloc failure\n", prog);
	}
	if (isdir(proj_object_base) < 0)
	    quit(1, "%s: object_base %s not a directory\n",
		 prog, proj_object_base);
    }
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
	if (isdir(proj_shadow_base) < 0)
	    quit(1, "%s: shadow_base %s not a directory\n",
		 prog, proj_shadow_base);
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
    if (project_field(&proj, "shadow_object_base", &field_p) != 0)
	proj_shadow_object_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper shadow_object_base\n", prog);
	proj_shadow_object_base = args_p->tokens[0];
	if (proj_object_base == NULL)
	    quit(1, "%s: object_base not defined\n", prog);
	if (proj_shadow_source_base == NULL)
	    quit(1, "%s: shadow_source_base not defined\n", prog);
	if (*proj_shadow_object_base != '/') {
	    if (proj_shadow_base == NULL && proj_project_base == NULL)
		quit(1, "%s: base undefined, shadow_object_base relative\n",
		     prog);
	    if (proj_shadow_base != NULL)
		(void) concat(temp, sizeof(temp),
			      proj_shadow_base, "/", proj_shadow_object_base,
			      NULL);
	    else
		(void) concat(temp, sizeof(temp),
			      proj_project_base, "/", proj_shadow_object_base,
			      NULL);
	    if ((proj_shadow_object_base = salloc(temp)) == NULL)
		quit(1, "%s: shadow_object_base salloc failure\n", prog);
	}
	if (strcmp(proj_shadow_object_base, proj_object_base) == 0)
	    quit(1, "%s: shadow_object_base and object_base match\n", prog);
    }
    if (project_field(&proj, "bcstemp", &field_p) != 0)
	proj_bcstemp = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper bcstemp\n", prog);
	proj_bcstemp = args_p->tokens[0];
    }
}

ckmakepath(destpath, refpath)
char *destpath, *refpath;
{
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN];
    int status;

    status = makepath(destpath, refpath, TRUE, TRUE);
    if (status != 0)
	return(status);
    path(destpath, dirbuf, filbuf);
    if (isdir(dirbuf) < 0)
	return(-1);
    return(0);
}

wanttarget(name)
char *name;
{
    char buf[BUFSIZ];

    if (exists(name) < 0)
	return(-1);
    (void) concat(buf, sizeof(buf), "Target ", name, " ok?", NULL);
    if (!getbool(buf, 1))
	return(-1);
    return(0);
}

/*
 * Generate branch name from target
 */
name_branch()
{
    char buf[BUFSIZ];
    FILE *fp;
    int status;
    char *ptr;
    int haveconfig;
    int i;
    char *av[MAXCMDARGS];

    if ((ptr = index(branch, '-')) != NULL) {
	do {
	    *ptr++ = '_';
	} while ((ptr = index(branch, '-')) != NULL);
	branch_name = NULL;
    }
    if (branch_name == NULL) {
	if ((*branch < 'A' || *branch > 'Z') &&
	    strncmp(branch, branch_pref, preflen) != 0)
	    (void) concat(buf, sizeof(buf), branch_pref, branch, NULL);
	else
	    (void) strcpy(buf, branch);
	(void) getstr("Branch", buf, buf);
	if ((*buf < 'A' || *buf > 'Z') &&
	    strncmp(buf, branch_pref, preflen) != 0)
	    (void) concat(branch, sizeof(branch), branch_pref, buf, NULL);
	else
	    (void) strcpy(branch, buf);
    }
    if (index(branch, '-') != NULL) {
	name_branch();
	return;
    }
    (void) concat(buf, sizeof(buf), ".BCSconfig-", branch, NULL);
    haveconfig = (exists(buf) == 0);
    if ((target_name != NULL && haveconfig &&
	 !getbool("Use existing branch", TRUE)) ||
	(target_name == NULL && !haveconfig &&
	 !getbool("Create new branch", TRUE))) {
	branch_name = NULL;
	name_branch();
	return;
    }
    printf("[ branch %s ]\n", branch);
    if (setenv("BCSBRANCH", branch, 1) < 0)
	quit(1, "%s: BCSBRANCH setenv failure\n", prog);
    if (!haveconfig) {
	printf("[ running bcs to create %s ]\n", buf);
	i = 0;
	if (debug)
	    av[i++] = "echo";
	av[i++] = "bcs";
	if (debug)
	    av[i++] = "-debug";
	av[i++] = "-branch";
	av[i++] = branch;
	av[i++] = "-newconfig";
	av[i++] = NULL;
	i = 0;
	if (debug) {
	    (void) runv(ECHO, av);
	    i++;
	}
	if ((status = runvp("bcs", &av[i])) != 0)
	    quit(1, "%s: runvp(bcs) status %#o\n", prog, status);
    }
    (void) concat(buf, sizeof(buf), ".BCSpath-", branch, NULL);
    if (exists(buf) == 0) {
	if ((fp = fopen(buf, "r")) != NULL) {
	    if (fgets(reldir, sizeof(reldir), fp) != NULL) {
		if ((ptr = index(reldir, '\n')) != NULL)
		    *ptr = '\0';
	    } else
		*reldir = '\0';
	    (void) fclose(fp);
	}
	if (*reldir == '\0')
	    (void) unlink(buf);
    }
}

/*
 * Generate the name of the target
 */
find_target()
{
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN], pathbuf[MAXPATHLEN];
    char target[MAXPATHLEN];
    char buf[BUFSIZ], tempbuf[BUFSIZ];
    char *ptr;
    char *head;
    char *tail;
    char *root;
    struct syspath *sp;

    if (*reldir != '\0')
	return;
    if (target_name == NULL) {
	(void) strcpy(reldir, ".");
	return;
    }
    if (*target_name != '/' && proj_dosearch) {
	for (sp = syspaths; sp->path != NULL; sp++) {
	    if ((ptr = getenv(sp->path)) == NULL)
		continue;
	    if (searchp(ptr, target_name, target, wanttarget) < 0)
		continue;
	    break;
	}
	if (sp->path == NULL)
	    (void) getstr("Target", target_name, target);
	target_name = target;
    }
    while (*target_name == '/')
	target_name++;
    printf("[ target %s ]\n", target_name);
    path(target_name, dirbuf, filbuf);
    head = dirbuf;
    tail = filbuf;
    root = index(tail, '.');
    ptr = concat(pathbuf, sizeof(pathbuf), ":", proj_source_base, NULL);
    (void) concat(ptr, pathbuf+sizeof(pathbuf)-ptr, ":", proj_rcs_base, NULL);
    (void) concat(buf, sizeof(buf), head, "/", tail, NULL);
    if (root != NULL) {
	if (searchp(pathbuf, buf, tempbuf, isdir) == 0)
	    (void) strcpy(reldir, buf);
	else {
	    *root = '\0';
	    (void) concat(buf, sizeof(buf), head, "/", tail, NULL);
	}
    }
    if (*reldir == '\0' && searchp(pathbuf, buf, tempbuf, isdir) == 0)
	(void) strcpy(reldir, buf);
    else if (searchp(pathbuf, head, tempbuf, isdir) == 0)
	(void) strcpy(reldir, head);
    if (*reldir == '\0') {
	(void) strcpy(reldir, head);
	printf("[ unable to find source to %s, using %s ]\n",
	       target_name, reldir);
    }
}

/*
 * Change to relative source subdirectory
 */
relative_subdir()
{
    char relpath[MAXPATHLEN], pathbuf[MAXPATHLEN];
    char *relp = NULL;

    if (isdir(reldir) < 0) {
	(void) concat(pathbuf, sizeof(pathbuf), reldir, "/.", NULL);
	if (proj_shadow_source_base != NULL) {
	    (void) concat(relpath, sizeof(relpath),
			  proj_source_base, "/", pathbuf, NULL);
	    if (isdir(relpath) == 0)
		relp = relpath;
	}
	if (ckmakepath(pathbuf, relp))
	    quit(1, "%s: makepath failed for %s\n", prog, pathbuf);
    }
    if (chdir(reldir) < 0)
	quit(1, "%s: chdir %s: %s\n", prog, reldir, errmsg(-1));
    printf("[ current directory is %s ]\n", reldir);
}
