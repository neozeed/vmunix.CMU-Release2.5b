/*
 * program to build programs in projects
 *
 **********************************************************************
 * HISTORY
 * $Log:	build.c,v $
 * Revision 2.7  89/08/08  16:07:04  gm0w
 * 	Changed to use searchp() to find make along project environment
 * 	instead of hard-wired "/usr/cs/bin/make" or assuming that the
 * 	environ_base was "/afs/cs.cmu.edu/@sys/<xxx>".
 * 	[89/08/08            gm0w]
 * 
 * Revision 2.6  89/05/13  18:30:25  gm0w
 * 	Added build_makeflags to project for build to use for things like
 * 	"-mc" which the systems project uses but other projects may not.
 * 	[89/05/13            gm0w]
 * 
 * Revision 2.5  89/02/19  11:17:49  gm0w
 * 	Added code to set PROJECT environment variable to new
 * 	"project" field of project description.
 * 	[89/02/19            gm0w]
 * 
 * Revision 2.4  89/02/18  17:02:41  gm0w
 * 	Added support for -replaceok and -logfile release options.
 * 	[89/02/18            gm0w]
 * 
 * Revision 2.3  89/01/29  12:47:45  gm0w
 * 	Removed code to run makepath for build -co.
 * 	[89/01/29            gm0w]
 * 
 * Revision 2.2  89/01/28  23:06:28  gm0w
 * 	Created build program from build.csh.
 * 	[88/11/28            gm0w]
 * 
 **********************************************************************
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <a.out.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <libc.h>
extern uid_t getuid();
extern gid_t getgid();
extern char *concat();
#include <c.h>
#include "project.h"

#ifdef	lint
/*VARARGS1*//*ARGSUSED*/
int quit(status) {};
/*VARARGS2*//*ARGSUSED*/
char *concat(buf, len) char *buf; int len; { return buf; };
#endif	lint

#define	RCSBIN		"/usr/misc/.rcs/bin"

char RCSCO[MAXPATHLEN];

struct stage {
    struct stage *next;
    char **names;
};

struct environ {
    struct environ *next;
    char **names;
    char *path;
};

char *proj_project_base;
char *proj_project_owner;
char *proj_project_cover;
char *proj_rcs_base;
char *proj_source_base;
char *proj_source_owner;
char *proj_source_cover;
char *proj_object_base;
char *proj_object_owner;
char *proj_object_cover;
char *proj_release_base;
char *proj_release_owner;
char *proj_release_cover;
char *proj_environ_base;
char *proj_shadow_base;
char *proj_shadow_rcs_base;
char *proj_shadow_source_base;
char *proj_shadow_object_base;
char *proj_build_makeflags;

struct stage *proj_stages;
struct environ *proj_environs;
struct arg_list *proj_paths;

char *prog;

char *user;
char makecmd[MAXPATHLEN];

char *project;
struct project proj;

int fromsource;
int noshadow;
char fromstagebuf[BUFSIZ] = "FROMSTAGE=";
char tostagebuf[BUFSIZ] = "TOSTAGE=";
char *fromstage;
char *tostage;
struct stage *fromstage_p;
struct stage *tostage_p;

char *env;
char env_path[BUFSIZ];

int release;
int lintflag;
int cleanflag;
int checkout;
char *checkoutfile;
int here;
int debug;

char **makeargs;
int nmakeargs;
int maxmakeargs;
char reloptsbuf[BUFSIZ] = "RELEASE_OPTIONS";
char *relopts;

char *deftargets[] = {
    "build_all",
    NULL
};
char *deftargetdirs[2];
char **targets;
char **targetdirs;
int ntargets;

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
     * check release consistancy
     */
    check_release();

    /*
     * check out new rcs files into source area
     */
    if (checkout)
	check_it_out();

    /*
     * set search paths
     */
    set_paths();

    /*
     * process arguments
     */
    check_targets();

    /*
     * Make targets, if requested
     */
    if (fromsource)
	build_targets();

    /*
     * Release targets, if requested
     */
    if (release)
	release_targets();

    exit(0);
}

resize_makeargs()
{
    if (maxmakeargs == 0) {
	maxmakeargs = 16;
	makeargs = (char **) malloc((unsigned) maxmakeargs * sizeof(char *));
    } else {
	maxmakeargs <<= 1;
	makeargs = (char **) realloc((char *) makeargs,
				    (unsigned) maxmakeargs * sizeof(char *));
    }
    if (makeargs == NULL)
	quit(1, "%s: makeargs malloc failed\n", prog);
}

parse_command_line(argc, argv)
int argc;
char **argv;
{
    int len;

    if (debug)
	printf("[ parsing command line ]\n");
    if (argc > 0) {
	if ((prog = rindex(argv[0], '/')) != NULL)
	    prog++;
	else
	    prog = argv[0];
    } else
	prog = "build";

    if ((user = getenv("USER")) == NULL)
	quit(1, "%s: USER not found in environment\n", prog);

    nmakeargs = 0;
    maxmakeargs = 0;
    relopts = reloptsbuf + strlen(reloptsbuf);
    fromstage = fromstagebuf + strlen(fromstagebuf);
    tostage = tostagebuf + strlen(tostagebuf);

    while (--argc > 0) {
	argv++;
	if (argv[0][0] != '-') {
	    if (index(argv[0], '=') == NULL)
		break;
	    if (nmakeargs == maxmakeargs)
		resize_makeargs();
	    makeargs[nmakeargs++] = argv[0];
	    continue;
	}
	switch (argv[0][1]) {
	case 'a':
	    if (strcmp(argv[0], "-as") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		env = argv[0];
		continue;
	    }
	    if (strcmp(argv[0], "-al") == 0 ||
		strcmp(argv[0], "-aslatest") == 0) {
		env = "latest";
		continue;
	    }
	    if (strcmp(argv[0], "-aa") == 0 ||
		strcmp(argv[0], "-asalpha") == 0) {
		env = "alpha";
		continue;
	    }
	    if (strcmp(argv[0], "-ab") == 0 ||
		strcmp(argv[0], "-asbeta") == 0) {
		env = "beta";
		continue;
	    }
	    if (strcmp(argv[0], "-ad") == 0 ||
		strcmp(argv[0], "-asdefault") == 0 ||
		strcmp(argv[0], "-ao") == 0 ||
		strcmp(argv[0], "-asomega") == 0) {
		env = "omega";
		continue;
	    }
	    break;
	case 'f':
	    if (strcmp(argv[0], "-fromstage") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		(void) strcpy(fromstage, argv[0]);
		continue;
	    }
	    if (strcmp(argv[0], "-fs") == 0 ||
		strcmp(argv[0], "-fromsource") == 0) {
		(void) strcpy(fromstage, "source");
		continue;
	    }
	    if (strcmp(argv[0], "-fl") == 0 ||
		strcmp(argv[0], "-fromlatest") == 0) {
		(void) strcpy(fromstage, "latest");
		continue;
	    }
	    if (strcmp(argv[0], "-fa") == 0 ||
		strcmp(argv[0], "-fromalpha") == 0) {
		(void) strcpy(fromstage, "alpha");
		continue;
	    }
	    if (strcmp(argv[0], "-fb") == 0 ||
		strcmp(argv[0], "-frombeta") == 0) {
		(void) strcpy(fromstage, "beta");
		continue;
	    }
	    if (strcmp(argv[0], "-fd") == 0 ||
		strcmp(argv[0], "-fromdefault") == 0 ||
		strcmp(argv[0], "-fo") == 0 ||
		strcmp(argv[0], "-fromomega") == 0) {
		(void) strcpy(fromstage, "omega");
		continue;
	    }
	    break;
	case 't':
	    if (strcmp(argv[0], "-tostage") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		(void) strcpy(tostage, argv[0]);
		continue;
	    }
	    if (strcmp(argv[0], "-tl") == 0 ||
		strcmp(argv[0], "-tolatest") == 0) {
		(void) strcpy(tostage, "latest");
		continue;
	    }
	    if (strcmp(argv[0], "-ta") == 0 ||
		strcmp(argv[0], "-toalpha") == 0) {
		(void) strcpy(tostage, "alpha");
		continue;
	    }
	    if (strcmp(argv[0], "-tb") == 0 ||
		strcmp(argv[0], "-tobeta") == 0) {
		(void) strcpy(tostage, "beta");
		continue;
	    }
	    if (strcmp(argv[0], "-td") == 0 ||
		strcmp(argv[0], "-todefault") == 0 ||
		strcmp(argv[0], "-to") == 0 ||
		strcmp(argv[0], "-toomega") == 0) {
		(void) strcpy(tostage, "omega");
		continue;
	    }
	    break;
	case 'c':
	    if (strcmp(argv[0], "-clean") == 0) {
		cleanflag = TRUE;
		continue;
	    }
	    if (strcmp(argv[0], "-co") == 0 ||
		strcmp(argv[0], "-checkout") == 0) {
		checkout = TRUE;
		continue;
	    }
	    if (strcmp(argv[0], "-cofile") == 0 ||
		strcmp(argv[0], "-checkoutfile") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		checkout = TRUE;
		checkoutfile = argv[0];
		continue;
	    }
	    break;
	case 'd':
	    if (strcmp(argv[0], "-debug") == 0) {
		debug = TRUE;
		continue;
	    }
	    break;
	case 'h':
	    if (strcmp(argv[0], "-here") == 0) {
		here = TRUE;
		continue;
	    }
	    break;
	case 'l':
	    if (strcmp(argv[0], "-lint") == 0) {
		lintflag = TRUE;
		continue;
	    }
	    if (strcmp(argv[0], "-logfile") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		relopts = concat(relopts,
				 reloptsbuf+sizeof(reloptsbuf)-relopts,
				 " -logfile ", argv[0], NULL);
		continue;
	    }
	    break;
	case 'n':
	    if (strcmp(argv[0], "-ns") == 0 ||
		strcmp(argv[0], "-noshadow") == 0) {
		noshadow = TRUE;
		continue;
	    }
	    if (strcmp(argv[0], "-nopost") == 0) {
		relopts = concat(relopts,
				 reloptsbuf+sizeof(reloptsbuf)-relopts,
				 " -nopost", NULL);
		continue;
	    }
	    break;
	case 'u':
	    if (strcmp(argv[0], "-uselog") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		relopts = concat(relopts,
				 reloptsbuf+sizeof(reloptsbuf)-relopts,
				 " -uselog ", argv[0], NULL);
		continue;
	    }
	    break;
	case 'r':
	    if (strcmp(argv[0], "-replaceok") == 0) {
		relopts = concat(relopts,
				 reloptsbuf+sizeof(reloptsbuf)-relopts,
				 " -replaceok", NULL);
		continue;
	    }
	    break;
	case 'p':
	    if (strcmp(argv[0], "-project") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		project = argv[0];
		continue;
	    }
	    break;
	default:
	    break;
	}
	if (nmakeargs == maxmakeargs)
	    resize_makeargs();
	makeargs[nmakeargs++] = argv[0];
    }

    len = strlen("RELEASE_OPTIONS");
    if (reloptsbuf[len] == '\0')
	reloptsbuf[0] = '\0';
    else
	reloptsbuf[len] = '=';

    /*
     * no targets implies "build_all" in the current directory when
     * not running as root.
     */
    if (argc == 0) {
	if (checkout && checkoutfile == NULL)
	    quit(1, "%s: no target specified\n", prog);
	ntargets = 1;
	targets = deftargets;
	targetdirs = deftargetdirs;
	return;
    }
    if (checkoutfile != NULL)
	quit(1, "%s: targets should not be specified with -checkoutfile\n",
	     prog);
    ntargets = argc;
    targets = argv;
    targetdirs = (char **) malloc((unsigned) ntargets * sizeof(char *));
    if (targetdirs == (char **)NULL)
	quit(1, "%s: targetdirs malloc failed\n", prog);
}

check_project()
{
    char temp[MAXPATHLEN];
    struct field *field_p;
    struct arg_list *args_p;
    struct environ *env_p;
    char *ptr;

    if (debug)
	printf("[ checking project ]\n");
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
    if (project_field(&proj, "project_owner", &field_p) != 0)
	proj_project_owner = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper project_owner\n", prog);
	proj_project_owner = args_p->tokens[0];
	if (project_field(&proj, "project_cover", &field_p) != 0)
	    quit(1, "%s: project_cover not defined\n", prog);
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper project_cover\n", prog);
	proj_project_cover = args_p->tokens[0];
	if (*proj_project_cover != '/') {
	    if (proj_project_base == NULL)
		quit(1,
		     "%s: project_base not defined, project_cover relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_project_cover, NULL);
	    if ((proj_project_cover = salloc(temp)) == NULL)
		quit(1, "%s: project_cover salloc failure\n", prog);
	}
	if (access(proj_project_cover, X_OK) < 0)
	    quit(1, "%s: project_cover %s not executable\n",
		 prog, proj_project_cover);
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
    if (project_field(&proj, "source_owner", &field_p) != 0)
	proj_source_owner = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper source_owner\n", prog);
	proj_source_owner = args_p->tokens[0];
	if (project_field(&proj, "source_cover", &field_p) != 0)
	    quit(1, "%s: source_cover not defined\n", prog);
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper source_cover\n", prog);
	proj_source_cover = args_p->tokens[0];
	if (*proj_source_cover != '/') {
	    if (proj_project_base == NULL)
		quit(1,
		     "%s: project_base not defined, source_cover relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_source_cover, NULL);
	    if ((proj_source_cover = salloc(temp)) == NULL)
		quit(1, "%s: source_cover salloc failure\n", prog);
	}
	if (access(proj_source_cover, X_OK) < 0)
	    quit(1, "%s: source_cover %s not executable\n",
		 prog, proj_source_cover);
    }
    if (project_field(&proj, "object_base", &field_p) != 0)
	quit(1, "%s: object_base not defined\n", prog);
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
    if (exists(proj_object_base) < 0)
	quit(1, "%s: object_base %s not found\n", prog, proj_object_base);
    if (project_field(&proj, "object_owner", &field_p) != 0)
	proj_object_owner = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper object_owner\n", prog);
	proj_object_owner = args_p->tokens[0];
	if (project_field(&proj, "object_cover", &field_p) != 0)
	    quit(1, "%s: object_cover not defined\n", prog);
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper object_cover\n", prog);
	proj_object_cover = args_p->tokens[0];
	if (*proj_object_cover != '/') {
	    if (proj_project_base == NULL)
		quit(1,
		     "%s: project_base not defined, object_cover relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_object_cover, NULL);
	    if ((proj_object_cover = salloc(temp)) == NULL)
		quit(1, "%s: object_cover salloc failure\n", prog);
	}
	if (access(proj_object_cover, X_OK) < 0)
	    quit(1, "%s: object_cover %s not executable\n",
		 prog, proj_object_cover);
    }
    if (project_field(&proj, "release_base", &field_p) != 0)
	quit(1, "%s: release_base not defined\n", prog);
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	quit(1, "%s: improper release_base\n", prog);
    proj_release_base = args_p->tokens[0];
    if (*proj_release_base != '/') {
	if (proj_project_base == NULL)
	    quit(1, "%s: project_base not defined, release_base relative\n",
		 prog);
	(void) concat(temp, sizeof(temp),
		      proj_project_base, "/", proj_release_base, NULL);
	if ((proj_release_base = salloc(temp)) == NULL)
	    quit(1, "%s: release_base salloc failure\n", prog);
    }
    if (exists(proj_release_base) < 0)
	quit(1, "%s: release_base %s not found\n", prog, proj_release_base);
    if (project_field(&proj, "release_owner", &field_p) != 0)
	proj_release_owner = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper release_owner\n", prog);
	proj_release_owner = args_p->tokens[0];
	if (project_field(&proj, "release_cover", &field_p) != 0)
	    quit(1, "%s: release_cover not defined\n", prog);
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper release_cover\n", prog);
	proj_release_cover = args_p->tokens[0];
	if (*proj_release_cover != '/') {
	    if (proj_project_base == NULL)
		quit(1,
		     "%s: project_base not defined, release_cover relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_release_cover, NULL);
	    if ((proj_release_cover = salloc(temp)) == NULL)
		quit(1, "%s: release_cover salloc failure\n", prog);
	}
	if (access(proj_release_cover, X_OK) < 0)
	    quit(1, "%s: release_cover %s not executable\n",
		 prog, proj_release_cover);
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
	if (exists(proj_shadow_base) < 0)
	    quit(1, "%s: shadow_base %s not found\n", prog, proj_shadow_base);
    }
    if (*fromstage != '\0' || *tostage != '\0') {
	if (project_field(&proj, "stage", &field_p) != 0)
	    quit(1, "%s: release stages not defined\n", prog);
	parse_stages(field_p->args);
    }
    if (*fromstage != '\0') {
	if (strcmp(fromstage, "source") == 0)
	    fromsource = TRUE;
	else if (find_stage(fromstage, &fromstage_p))
	    quit(1, "%s: stage %s not defined\n", prog, fromstage);
    }
    if (*tostage != '\0' && find_stage(tostage, &tostage_p))
	quit(1, "%s: stage %s not defined\n", prog, tostage);
    if (project_field(&proj, "environ_base", &field_p) != 0) {
	proj_environ_base = NULL;
	if (project_field(&proj, "environ", &field_p) == 0 ||
	    project_field(&proj, "path", &field_p) == 0)
	    quit(1, "%s: environ_base, environ and path %s\n",
		 prog, "must be all defined or undefined");
	env = NULL;
    } else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper environ_base\n", prog);
	proj_environ_base = args_p->tokens[0];
	if (*proj_environ_base != '/') {
	    if (proj_project_base == NULL)
		quit(1, "%s: project_base not set, environ_base relative\n",
		     prog);
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_environ_base, NULL);
	    if ((proj_environ_base = salloc(temp)) == NULL)
		quit(1, "%s: environ_base salloc failure\n", prog);
	}
	if (exists(proj_environ_base) < 0)
	    quit(1, "%s: environ_base %s not found\n",
		 prog, proj_environ_base);
	if (project_field(&proj, "environ", &field_p) != 0)
	    quit(1, "%s: environ_base, environ and path %s\n",
		 prog, "must be all defined or undefined");
	parse_environs(field_p->args);
	if (env == NULL) {
	    if (project_field(&proj, "environ_default", &field_p) != 0) {
		if ((env_p = proj_environs) == NULL)
		    quit(1, "%s: environ_base, environ and path %s\n",
			 prog, "must be all defined or undefined");
		while (env_p->next != NULL)
		    env_p = env_p->next;
	    } else {
		args_p = field_p->args;
		if (args_p->ntokens != 1)
		    quit(1, "%s: improper environ_default\n", prog);
		env = args_p->tokens[0];
		if (find_environ(env, &env_p))
		    quit(1, "%s: environ %s not defined\n", prog, env);
	    }
	} else if (find_environ(env, &env_p))
	    quit(1, "%s: environ %s not defined\n", prog, env);
	env = env_p->names[0];
	(void) concat(env_path, sizeof(env_path),
		      proj_environ_base, "/", env_p->path, NULL);
	if (project_field(&proj, "path", &field_p) != 0)
	    quit(1, "%s: environ_base, environ and path %s\n",
		 prog, "must be all defined or undefined");
	proj_paths = field_p->args;
	for (args_p = proj_paths; args_p != NULL; args_p = args_p->next)
	    if (args_p->ntokens != 2)
		quit(1, "%s: bad path in project description\n", prog);
    }
    if (project_field(&proj, "build_makeflags", &field_p) != 0)
	proj_build_makeflags = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    quit(1, "%s: improper build_makeflags\n", prog);
	proj_build_makeflags = args_p->tokens[0];
    }
}

char **
alloc_ptrs(name)
char *name;
{
    char *ptr, *p, **pp;
    int i;

    if ((ptr = salloc(name)) == NULL)
	quit(1, "%s: unable to allocate ptrs\n", prog);
    i = 1;
    for (p = ptr; *name != '\0'; p++, name++) {
	*p = *name;
	if (*p == ',')
	    i++;
    }
    if ((pp = (char **)calloc((unsigned)i+1, sizeof(char *))) == NULL)
	quit(1, "%s: unable to allocate ptrs\n", prog);
    i = 0;
    p = ptr;
    do {
	pp[i++] = nxtarg(&p, ",");
    } while (*p != '\0');
    pp[i] = NULL;
    return(pp);
}

parse_stages(args_p)
struct arg_list *args_p;
{
    struct stage **stage_pp;

    stage_pp = &proj_stages;
    do {
	*stage_pp = (struct stage *)calloc(1, sizeof(struct stage));
	if (*stage_pp == (struct stage *)NULL)
	    quit(1, "%s: unable to allocate stage\n", prog);
	if (args_p->ntokens < 2)
	    quit(1, "%s: bad stage in project description\n", prog);
	(*stage_pp)->names = alloc_ptrs(args_p->tokens[0]);
	stage_pp = &((*stage_pp)->next);
	args_p = args_p->next;
    } while (args_p != NULL);
}

find_stage(stage, stage_pp)
char *stage;
struct stage **stage_pp;
{
    struct stage *stage_p;
    char **pp;

    for (stage_p = proj_stages; stage_p != NULL; stage_p = stage_p->next)
	for (pp = stage_p->names; *pp != NULL; pp++)
	    if (strcmp(stage, *pp) == 0) {
		*stage_pp = stage_p;
		return(0);
	    }
    return(1);
}

parse_environs(args_p)
struct arg_list *args_p;
{
    struct environ **env_pp;

    env_pp = &proj_environs;
    do {
	*env_pp = (struct environ *)calloc(1, sizeof(struct environ));
	if (*env_pp == (struct environ *)NULL)
	    quit(1, "%s: unable to allocate environ\n", prog);
	if (args_p->ntokens != 2)
	    quit(1, "%s: bad environ in project description\n", prog);
	(*env_pp)->names = alloc_ptrs(args_p->tokens[0]);
	(*env_pp)->path = args_p->tokens[1];
	env_pp = &((*env_pp)->next);
	args_p = args_p->next;
    } while (args_p != NULL);
}

find_environ(environ, env_pp)
char *environ;
struct environ **env_pp;
{
    struct environ *env_p;
    char **pp;

    for (env_p = proj_environs; env_p != NULL; env_p = env_p->next)
	for (pp = env_p->names; *pp != NULL; pp++)
	    if (strcmp(environ, *pp) == 0) {
		*env_pp = env_p;
		return(0);
	    }
    return(1);
}

check_release()
{
    char temp[MAXPATHLEN];
    struct stage *stage_p;
    struct field *field_p;
    struct arg_list *args_p;

    if (debug)
	printf("[ checking release consistency ]\n");

    /*
     * check for implied value of "tostage" from "fromstage" or vice-versa
     */
    if (tostage_p == NULL) {
	if (fromsource) {
	    tostage_p = proj_stages;
	} else if (fromstage_p != NULL) {
	    if ((tostage_p = fromstage_p->next) == NULL)
		quit(1, "%s: no release stage defined after %s\n",
		     prog, fromstage);
	}
    } else if (!fromsource) {
	if (fromstage_p == NULL) {
	    if ((fromstage_p = proj_stages) == tostage_p)
		fromsource = TRUE;
	    else {
		while (fromstage_p->next != tostage_p)
		    fromstage_p = fromstage_p->next;
		(void) strcpy(fromstage, fromstage_p->names[0]);
	    }
	} else {
	    for (stage_p = proj_stages; stage_p != NULL;
		 stage_p = stage_p->next) {
		if (stage_p == fromstage_p) {
		    if (stage_p == tostage_p)
			quit(1, "%s: release stages %s and %s are the same\n",
			     prog, fromstage, tostage);
		    break;
		} else if (stage_p == tostage_p)
		    quit(1, "%s: unable to release to previous stage %s\n",
			 prog, tostage);
	    }
	}
    }
    if (tostage_p != NULL)
	(void) strcpy(tostage, tostage_p->names[0]);
    if (fromstage_p != NULL)
	(void) strcpy(fromstage, fromstage_p->names[0]);

    if (release = (tostage_p != NULL)) {
	if (!fromsource && (cleanflag || lintflag))
	    quit(1, "%s: -clean and -lint flags not allowed with release\n",
		 prog);
	noshadow = TRUE;
    } else
	fromsource = TRUE;

    if (fromsource)
	fromstagebuf[0] = '\0';

    if (noshadow) {
	proj_shadow_rcs_base = NULL;
	proj_shadow_source_base = NULL;
	proj_shadow_object_base = NULL;
	return;
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
    if (proj_shadow_rcs_base == NULL &&
	proj_shadow_source_base == NULL &&
	proj_shadow_object_base == NULL) {
	noshadow = TRUE;
	return;
    }
    if (proj_shadow_source_base == NULL)
	quit(1, "%s: shadow_source_base not defined\n", prog);
    if (proj_shadow_object_base == NULL)
	quit(1, "%s: shadow_object_base not defined\n", prog);
}

/*
 * For the time being we special case the "checkout" function here.
 * It will eventually be done automatically as part of the make procedure
 */
check_it_out()
{
    char buf[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    char *ptr, *cmd;
    FILE *fp, *pp;
    int p[2];
    int pid;
    int i, j;
    char *av[16];

    if (proj_source_owner != NULL) {
	if (setenv("AUTHCOVER_USER", proj_source_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	if (setenv("AUTHCOVER_TESTDIR", proj_source_base, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	cmd = proj_source_cover;
    } else if (proj_project_owner != NULL) {
	if (setenv("AUTHCOVER_USER", proj_project_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	if (setenv("AUTHCOVER_TESTDIR", proj_source_base, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	cmd = proj_project_cover;
    }
    if (checkoutfile != NULL && (fp = fopen(checkoutfile, "r")) == NULL)
	quit(1, "%s: unable to open %s: %s\n", prog, checkoutfile, errmsg(-1));
    if ((ptr = getenv("RCSBIN")) != NULL && *ptr == '\0')
	(void) concat(RCSCO, sizeof(RCSCO), ptr, "/rcsco", NULL);
    else if (env == NULL)
	(void) concat(RCSCO, sizeof(RCSCO), RCSBIN, "/rcsco", NULL);
    else
	(void) concat(RCSCO, sizeof(RCSCO), env_path, RCSBIN, "/rcsco", NULL);
    if (debug)
	printf("[ checking out rcs revisions ]\n");
    i = 0;
    if (debug)
	av[i++] = "echo";
    if (proj_source_owner != NULL || proj_project_owner != NULL) {
	av[i++] = "authcover";
	av[i++] = RCSCO;
    } else {
	av[i++] = "rcsco";
	cmd = RCSCO;
    }
    (void) concat(buf, sizeof(buf), "-P", proj_source_base, NULL);
    av[i++] = buf;
    av[i++] = "-";
    for (j = 0; j < nmakeargs; j++)
	av[i++] = makeargs[j];
    av[i++] = NULL;
    (void) fflush(stdout);
    (void) fflush(stderr);
    if (debug) {
	(void) runv("/bin/echo", av);
	exit(0);
    }
    if (pipe(p) < 0)
	quit(1, "%s: pipe: %s\n", prog, errmsg(-1));
    pid = runcmdv(cmd, proj_rcs_base, p[1], p[0], av);
    if (pid == -1)
	exit(-1);
    (void) close(p[0]);
    if ((pp = fdopen(p[1], "w")) == NULL) {
	quit(1, "%s: fdopen: %s\n", prog, errmsg(-1));
	(void) endcmd(pid);
	exit(-1);
    }
    if (checkoutfile == NULL) {
	for (i = 0; i < ntargets; i++) {
	    if (targets[i][0] == '/')
		targets[i]++;
	    else if (targets[i][0] == '.' && targets[i][1] == '/')
		targets[i] += 2;
	    fprintf(pp, "./%s,v\n", targets[i]);
	}
    } else {
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
	    if ((ptr = index(buffer, '\n')) != NULL)
		*ptr = '\0';
	    ptr = buffer;
	    if (*ptr == '/')
		ptr++;
	    else if (*ptr == '.' && ptr[1] == '/')
		ptr += 2;
	    fprintf(pp, "%s,v\n", ptr);
	}
	(void) fclose(fp);
    }
    (void) fclose(pp);
    exit(endcmd(pid));
}

set_paths()
{
    char path[MAXPATHLEN];
    char *ptr, *p;
    struct arg_list *pathp;

    if (debug)
	printf("[ setting paths ]\n");

    if (env != NULL) {
	if (setenv("RELEASE_BASE", env_path, 1) < 0)
	    quit(1, "%s: RELEASE_BASE setenv failure: %s\n", prog, errmsg(-1));
	for (pathp = proj_paths; pathp != NULL; pathp = pathp->next) {
	    ptr = path;
	    p = pathp->tokens[1];
	    for (;;) {
		if (*p != '/') {
		    ptr = concat(ptr, path+sizeof(path)-ptr, env_path, NULL);
		    if (*p != ':' && *p != '\0')
			*ptr++ = '/';
		}
		while (*p != ':' && *p != '\0')
		    *ptr++ = *p++;
		if ((*ptr++ = *p++) == '\0')
		    break;
	    }
	    if (setenv(pathp->tokens[0], path, 1) < 0)
		quit(1, "%s: %s setenv failure: %s\n",
		     prog, pathp->tokens[0], errmsg(-1));
	}
    }

    if ((ptr = getenv("MAKE")) != NULL)
	(void) strcpy(makecmd, ptr);
    else {
	if ((ptr = getenv("PATH")) == NULL)
	    quit(1, "%s: PATH not defined in environment\n", prog);
	if (searchp(ptr, "make", path, exists) == 0)
	    (void) strcpy(makecmd, path);
	else
	    (void) strcpy(makecmd, "make");
    }

    if (noshadow)
	unsetenv("MASTERSOURCEDIR");
    else if (setenv("MASTERSOURCEDIR", proj_source_base, 1) < 0)
	quit(1, "%s: MASTERSOURCEDIR setenv failure: %s\n", errmsg(-1));

    printf("[ ");
    if (noshadow)
	fputs(proj_source_base, stdout);
    else
	fputs(proj_shadow_source_base, stdout);
    if (env != NULL)
	printf(" (as %s)", env);
    printf(" ]\n");
}

check_targets()
{
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN];
    char testtarget[MAXPATHLEN];
    char targetdir[MAXPATHLEN];
    char *fulltarget;
    char *ptr, *p;
    int absolute;
    int i;

    for (i = 0; i < ntargets; i++) {

	fulltarget = targets[i];

	if (debug)
	    printf("[ target %s ]\n", fulltarget);

	/*
	 * remember whether or not the target is absolute
	 */
	absolute = (*fulltarget == '/');
	if (!absolute && noshadow)
	    quit(1, "%s: target %s is not absolute\n", prog, fulltarget);
	if (here || strcmp(fulltarget, "build_all") == 0) {
	    if (strcmp(fulltarget, "build_all") == 0) {
		targetdir[0] = '\0';
	    } else {
		(void) strcpy(targetdir, fulltarget);
		if (targetdir[0] == '\0') {
		    targetdir[0] = '/';
		    targetdir[1] = '\0';
		}
	    }
	    filbuf[0] = '\0';
	} else {
	    path(fulltarget, dirbuf, filbuf);
	    if (filbuf[0] == '\0' || (filbuf[0] == '.' && filbuf[1] == '\0'))
		quit(1, "%s: invalid null target\n", prog);
	    if (rcssrcdir(absolute, fulltarget))
		(void) strcpy(targetdir, fulltarget);
	    else {
		ptr = fulltarget;
		p = testtarget;
		while (*p = *ptr++)
		    p++;
		ptr = NULL;
		while (p >= testtarget && *p != '/') {
		    if (*p == '.')
			ptr = p;
		    p--;
		}
		if (ptr != NULL && ptr != p + 1)
		    *ptr = '\0';
		else
		    ptr = NULL;
		if (ptr != NULL && rcssrcdir(absolute, testtarget))
		    (void) strcpy(targetdir, testtarget);
		else
		    (void) strcpy(targetdir, dirbuf);
	    }
	}
	if (targetdir[0] == '.' && targetdir[1] == '\0')
	    targetdir[0] = '\0';
	if (!rcssrcdir(absolute, targetdir))
	    quit(1, "%s: targetdir %s is not a directory\n", prog, targetdir);
	targets[i] = salloc(filbuf);

	/*
	 * If we are changing to some directory for the build (e.g. the
	 * original target contained a slash), figure out how to
	 * indicate this to the user.  If the target is relative to the
	 * current directory, just use the relative directory name.  If
	 * the target is absolute preceed this with the string "... ."
	 * to suggest that the directory is relative to the absolute
	 * base of the source tree.
	 */
	if (absolute || *targetdir != '\0')
	    targetdirs[i] = salloc(targetdir);
	else
	    targetdirs[i] = NULL;
    }
}

rcssrcdir(absolute, testtarget)
int absolute;
char *testtarget;
{
    char buf[MAXPATHLEN];
    struct stat statb;

    if (absolute) {
	if (proj_shadow_rcs_base != NULL) {
	    (void) concat(buf, sizeof(buf),
			  proj_shadow_rcs_base, testtarget, NULL);
	    if (stat(buf, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
		return(TRUE);
	}
	(void) concat(buf, sizeof(buf), proj_rcs_base, testtarget, NULL);
	if (stat(buf, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
	    return(TRUE);
	if (proj_shadow_source_base != NULL) {
	    (void) concat(buf, sizeof(buf),
			  proj_shadow_source_base, testtarget, NULL);
	    if (stat(buf, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
		return(TRUE);
	}
	(void) concat(buf, sizeof(buf), proj_source_base, testtarget, NULL);
	if (stat(buf, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
	    return(TRUE);
    } else {
	if (stat(testtarget, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
	    return(TRUE);
    }
    return(FALSE);
}

build_targets()
{
    char dirbuf[MAXPATHLEN];
    char build_target[MAXPATHLEN];
    char clean_target[MAXPATHLEN];
    char lint_target[MAXPATHLEN];
    char *cd, *target;
    int status;
    int pid;
    char *dirptr;
    char *ptr;
    int i, j;
    int firsti;
    char *av[64];
    char *cmd;

    dirptr = concat(dirbuf, sizeof(dirbuf),
		    noshadow ? proj_source_base : proj_shadow_source_base,
		    NULL);

    i = 0;
    if (debug)
	av[i++] = "echo";

    if (noshadow && proj_object_owner != NULL) {
	if (setenv("AUTHCOVER_USER", proj_object_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	if (setenv("AUTHCOVER_TESTDIR", proj_object_base, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	av[i++] = "authcover";
	av[i++] = makecmd;
	cmd = proj_object_cover;
    } else if (noshadow && proj_project_owner != NULL) {
	if (setenv("AUTHCOVER_USER", proj_project_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	if (setenv("AUTHCOVER_TESTDIR", proj_object_base, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	av[i++] = "authcover";
	av[i++] = makecmd;
	cmd = proj_project_cover;
    } else {
	if ((ptr = rindex(makecmd, '/')) != NULL)
	    ptr++;
	else
	    ptr = makecmd;
	av[i++] = ptr;
	cmd = makecmd;
    }
    if (proj_build_makeflags != NULL)
	av[i++] = proj_build_makeflags;
    for (j = 0; j < nmakeargs; j++)
	av[i++] = makeargs[j];
    firsti = i;

    /*
     * For each target:
     *   Run a "clean" operation first, if requested
     *   Run a "build" operation if we are not making a release or if we
     *     are releasing from sources.
     *   Run a "lint" operation if requested.
     */
    for (j = 0; j < ntargets; j++) {

	i = firsti;

	cd = targetdirs[j];
	target = targets[j];

	if (debug)
	    printf("[ building %s ]\n",
		   (*target != '\0') ? target : "build_all");

	if (cd != NULL) {
	    if (*cd != '/')
		printf("cd %s\n", cd);
	    else {
		printf("cd ... .%s\n", cd);
		(void) concat(dirptr, dirbuf+sizeof(dirbuf)-dirptr, cd, NULL);
		cd = dirbuf;
	    }
	}

	if (*target == '\0') {
	    if (cleanflag)
		av[i++] = "clean_all";
	    av[i++] = "build_all";
	    if (lintflag)
		av[i++] = "lint_all";
	} else {
	    if (cleanflag) {
		(void) concat(clean_target, sizeof(clean_target),
			      "_clean_prefix_", target, NULL);
		av[i++] = clean_target;
	    }
	    (void) concat(build_target, sizeof(build_target),
			  "_build_prefix_", target, NULL);
	    av[i++] = build_target;
	    if (lintflag) {
		(void) concat(lint_target, sizeof(lint_target),
			      "_lint_prefix_", target, NULL);
		av[i++] = lint_target;
	    }
	}
	av[i++] = NULL;

	(void) fflush(stdout);
	(void) fflush(stderr);
	if (debug) {
	    (void) runv("/bin/echo", av);
	    continue;
	}
	if ((pid = runcmdv(cmd, cd, -1, -1, av)) == -1)
	    exit(-1);
	if ((status = endcmd(pid)) != 0)
	    exit(status);
    }
}

release_targets()
{
    char buf[MAXPATHLEN];
    char dirbuf[MAXPATHLEN];
    char install_target[MAXPATHLEN];
    char *cd, *target;
    int status;
    int pid;
    char *dirptr;
    char *ptr;
    char *cmd;
    int i, j;
    int firsti;
    char *av[64];

    dirptr = concat(dirbuf, sizeof(dirbuf),
		    noshadow ? proj_source_base : proj_shadow_source_base,
		    NULL);

    i = 0;
    if (debug)
	av[i++] = "echo";

    if (project != NULL && setenv("PROJECT", project, 1) < 0)
	quit(1, "%s: PROJECT setenv failed\n", prog);
    if (proj_release_owner != NULL) {
	if (setenv("AUTHCOVER_USER", proj_release_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	(void) concat(buf, sizeof(buf),
		      proj_release_base, "/", tostage, NULL);
	if (exists(buf) < 0)
	    quit(1, "%s: release base %s not found\n", prog, buf);
	if (setenv("AUTHCOVER_TESTDIR", buf, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	av[i++] = "authcover";
	av[i++] = makecmd;
	cmd = proj_release_cover;
    } else if (proj_project_owner != NULL) {
	if (setenv("AUTHCOVER_USER", proj_release_owner, 1) < 0)
	    quit(1, "%s: AUTHCOVER_USER setenv failed\n", prog);
	(void) concat(buf, sizeof(buf),
		      proj_release_base, "/", tostage, NULL);
	if (exists(buf) < 0)
	    quit(1, "%s: release base %s not found\n", prog, buf);
	if (setenv("AUTHCOVER_TESTDIR", buf, 1) < 0)
	    quit(1, "%s: AUTHCOVER_TESTDIR setenv failed\n", prog);
	av[i++] = "authcover";
	av[i++] = makecmd;
	cmd = proj_project_cover;
    } else {
	if ((ptr = rindex(makecmd, '/')) != NULL)
	    ptr++;
	else
	    ptr = makecmd;
	av[i++] = ptr;
	cmd = makecmd;
    }
    if (proj_build_makeflags != NULL)
	av[i++] = proj_build_makeflags;
    for (j = 0; j < nmakeargs; j++)
	av[i++] = makeargs[j];
    if (*reloptsbuf != '\0')
	av[i++] = reloptsbuf;
    av[i++] = tostagebuf;
    if (*fromstagebuf != '\0')
	av[i++] = fromstagebuf;
    firsti = i;

    for (j = 0; j < ntargets; j++) {

	i = firsti;

	cd = targetdirs[j];
	target = targets[j];

	if (debug)
	    printf("[ installing %s ]\n",
		   (*target != '\0') ? target : "install_all");

	if (cd != NULL) {
	    if (*cd != '/')
		printf("cd %s\n", cd);
	    else {
		printf("cd ... .%s\n", cd);
		(void) concat(dirptr, dirbuf+sizeof(dirbuf)-dirptr, cd, NULL);
		cd = dirbuf;
	    }
	}

	if (*target == '\0')
	    av[i++] = "install_all";
	else {
	    (void) concat(install_target, sizeof(install_target),
			  "_install_prefix_", target, NULL);
	    av[i++] = install_target;
	}
	av[i++] = NULL;

	(void) fflush(stdout);
	(void) fflush(stderr);
	if (debug) {
	    (void) runv("/bin/echo", av);
	    continue;
	}
	if ((pid = runcmdv(cmd, cd, -1, -1, av)) == -1)
	    exit(-1);
	if ((status = endcmd(pid)) != 0)
	    exit(status);
    }
}

runcmdv(cmd, dir, closefd, infd, av)
char *cmd, *dir;
int closefd, infd;
char **av;
{
    int child;

    if ((child = vfork()) == -1) {
	fprintf(stderr, "%s: %s vfork failed: %s\n", prog, av[0], errmsg(-1));
	return(-1);
    }
    if (child == 0) {
	(void) setgid(getgid());
	(void) setuid(getuid());
	if (dir != NULL && chdir(dir) < 0)
	    quit(0377, "%s: %s chdir failed: %s\n", prog, dir, errmsg(-1));
	if (closefd >= 0)
	    (void) close(closefd);
	if (infd >= 0 && infd != 0) {
	    (void) dup2(infd, 0);
	    (void) close(infd);
	}
	execvp(cmd, av);
	exit(0377);
    }
    return(child);
}

endcmd(child)
int child;
{
    int pid, omask;
    union wait w;

    omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
    do {
	pid = wait3(&w, WUNTRACED, (struct rusage *)NULL);
	if (WIFSTOPPED(w)) {
	    (void) kill(0, SIGTSTP);
	    pid = 0;
	}
    } while (pid != child && pid != -1);
    (void) sigsetmask(omask);
    if (pid == -1) {
	fprintf(stderr, "%s: wait: %s\n", prog, errmsg(-1));
	return(-1);
    }
    if (WIFSIGNALED(w) || w.w_retcode == 0377)
	return(-1);
    return(w.w_retcode);
}
