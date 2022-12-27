/*
 **********************************************************************
 * HISTORY
 * $Log:	release.c,v $
 * Revision 2.12  89/08/14  22:09:22  mja
 * 	Change to stamp new log message text extracted from old log message
 * 	with author, date and stage from that release.
 * 
 * Revision 2.11  89/08/06  22:32:22  mbj
 * 	Don't strip Multimax executables which still have relocation
 * 	information present.
 * 	[89/08/06  22:30:22  mbj]
 * 
 * Revision 2.10  89/07/28  00:44:59  mbj
 * 	Strip Multimax binaries.  We test for executables ourselves and
 * 	then hand off to /bin/strip to do the job when we find one.
 * 	[89/07/28  00:43:41  mbj]
 * 
 * Revision 2.9  89/07/11  01:38:24  mbj
 * 	Conditionally compile out strip code for multimax since it uses COFF.
 * 	[89/06/23            mbj]
 * 
 * Revision 2.8  89/07/04  17:42:44  gm0w
 * 	Reversed the order of log messages, now newer messages preceed
 * 	older ones like modification histories.  Changed to no longer
 * 	strip all white space between target name and message from log,
 * 	preserving intentional indentation.  Changed to no longer use
 * 	default log message.  Added code to use new release database, but
 * 	as this is conditional upon a new project description directive
 * 	that is not defined it is not being used yet.
 * 	[89/07/04            gm0w]
 * 
 * Revision 2.7  89/02/28  15:34:32  gm0w
 * 	Reimplemented/reordered log-message/post/check-state code.
 * 	[89/02/28            gm0w]
 * 
 * Revision 2.6  89/02/25  17:33:40  gm0w
 * 	Fixed file stripping code.
 * 	[89/02/25            gm0w]
 * 
 * Revision 2.5  89/02/18  15:43:05  gm0w
 * 	Added code to verify that -uselog finds a message.  Added
 * 	new -replaceok switch to indicate that the destination targets
 * 	are known to exist and that the new versions are intended as
 * 	replacements.
 * 	[89/02/18            gm0w]
 * 
 * Revision 2.4  89/02/08  09:25:26  gm0w
 * 	Fixed minor bug in previous edit.
 * 	[89/02/08            gm0w]
 * 
 * Revision 2.3  89/02/06  15:09:00  gm0w
 * 	Fixed bug in release lock message code.  Added -logfile switch
 * 	for providing message for release to use.  Added code to get
 * 	log message before release stages are used when -uselog or
 * 	-logfile is specified.
 * 	[89/02/05            gm0w]
 * 
 * Revision 2.2  89/01/28  23:04:57  gm0w
 * 	Created release program from release.csh.
 * 	[88/11/22            gm0w]
 * 
 **********************************************************************
 */
/*
 *
 *   Release a program either from the build area (-fromfile) or move
 *   through release stages (-fromstage).
 *
 *   One of either -fromfile or -fromstage must given and they are mutually
 *   exclusive.  Each of these switches can be used to name 'modes' of
 *   operation.  In '-fromfile' mode, the source is taken to be the name
 *   given after the switch and can be anywhere.  For all of the other
 *   switches e.g. '-m', the appropriate actions are performed.
 *
 *   In '-fromstage' mode, the assumption is that the file(s) are to be
 *   released from one stage to the next (e.g. alpha to beta).  Some of
 *   the switches have no meaning (mainly various 'stripping' switches)
 *   in this mode, so are ignored.  If the target file(s) is/are created
 *   successfully, then the source is automatically removed, unless the
 *   '-norm' switch is specified.
 *
 *   The meaning of the switches:
 *
 *	-fromfile <file>   -- The file to be copied into the release area;
 *			      to be used when a 'program' is first built.
 *	-fromstage <stage> -- The stage directory prefix in the release area
 *			      to be concatenated with a file name in the
 *			      local file system to form the name for the source
 *			      file; to be used in moving a program through
 *			      various releases.
 *	-tostage   <stage> -- The stage directory prefix in the release are to
 *			      be concatenated with a file name in the local
 *			      file system to form the name of the destination
 *			      file; this is used with either of the above 2
 *			      switches and is required.
 *	-m	<mode>     -- The file protection of the target.
 *	-o	<owner>    -- The owner who should own the target.
 *	-g	<group>    -- The group who should own the target.
 *	-nostrip	   -- The target is not a binary, so 'strip' should not
 *			      be run on it.
 *	-nopost		   -- The log message is gathered up for inclusion in
 *			      the log but not posted to the bulletin board.
 *	-norm		   -- The source files are not removed after being
 *			      copied with -fromstage.
 *	-logfile <file>    -- The file containing the log message to use.
 *	-uselog <target>   -- The target to be used when searching for a log
 *			      message instead of the target being installed.
 *			      This log message is extracted from the -tostage
 *			      directory under the assumption that the current
 *			      target should receive the same log message as
 *			      this previously installed related target.
 *	-q		   -- "Quick" mode.  No prompting to continue when
 *			      a non-fatal problem occurs, but the script
 *			      continues.
 *
 *   The remaining arguments are taken to be names in the local file system
 *   for the target.  The first name is taken to be the canonical name.
 *   Any other names are taken to be (hard) links to the canonical name,
 *   so the links will be made in the release area.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
extern int errno;
#include <stdio.h>
#include <a.out.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <varargs.h>
#include <libc.h>
#include <c.h>
#include "project.h"
#include "release_db.h"

#ifdef	lint
/*VARARGS1*//*ARGSUSED*/
int quit(status) {};
/*VARARGS2*//*ARGSUSED*/
char *concat(buf, len) char *buf; int len; { return buf; };
#endif	lint

char *fixuppath();

#define	O_WRNEW	O_WRONLY|O_CREAT|O_EXCL

struct stage {
    struct stage *next;
    char **names;
    char **roots;
    int flags;
#define	STAGE_NORM	1
#define	STAGE_NOPOST	2
};

char *proj_project_base;
char *proj_source_base;
char *proj_rcs_base;
char *proj_release_base;
char *proj_release_lock;
struct stage *proj_stages;
char *proj_release_database;

char *prog;

char *project;
struct project proj;

int havecmdlineinfo;
char *user;
char *owner;
char *group;
int uid;
int gid;
u_short mode;
int strip;
int post;
int rm;
int replaceok;
int debug;
int quick;
char *fromfile;
char *fromstage;
char *tostage;
struct stage *fromstage_p;
struct stage *tostage_p;
char *uselog;
char *logfile;
char logfilebuf[MAXPATHLEN];
char *target;
char **links;
int nlinks;
int targetlen;
int lock_fd;

int check;

struct timeval source_tv[2];

char frombase[MAXPATHLEN];
char tobase[MAXPATHLEN];
char fullsource[MAXPATHLEN];
char fulltarget[MAXPATHLEN];
char fromlogdb[MAXPATHLEN];
char fromwhodb[MAXPATHLEN];
char tologdb[MAXPATHLEN];
char towhodb[MAXPATHLEN];

#define HL_FILE	1
#define HL_FROM	2
#define HL_TO	4

int havelog;

enum option { EDIT, TYPE, LOG, POST, SUBJECT, NOCOMMAND };

char *posttab[] = {
    "edit		edit notice",
    "type		print notice for inspection",
    "subject		change \"Subject\" line",
    "log		log notice as planned, no post",
    "post		post notice as planned",
    0
};

enum option postopt[] = {
    EDIT,
    TYPE,
    SUBJECT,
    LOG,
    POST
};

char *logtab[] = {
    "edit		edit notice",
    "type		print notice for inspection",
    "log		log notice as planned, no post",
    0
};

enum option logopt[] = {
    EDIT,
    TYPE,
    LOG
};

char *arc_table[] = {
    "abort",
#define ABORT	0
    "retry",
#define RETRY	1
    "continue",
#define CONTIN	2
    NULL
};

char *aur_table[] = {
    "abort",
    "unlock",
#define UNLOCK	1
    "replace",
#define REPLACE	2
    NULL
};

char *ar_table[] = {
    "abort",
    "retry",
    NULL
};

getarc()
{
    int key;

    if (quick)
	return(check ? -1 : 1);
    if (check)
	key = getstab("Abort or retry?", ar_table, "retry");
    else
	key = getstab("Abort, retry or continue?", arc_table, "retry");
    if (key == ABORT)
	return(-1);
    if (key == CONTIN)
	return(1);
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
    struct stage *stage_p;

    /*
     * parse command line
     */
    parse_command_line(argc, argv);

    /*
     * read project description
     */
    if (parse_project(project, &proj) != 0)
	fatal("unable to parse project");

    /*
     * check project description
     */
    check_project();

    /*
     * get component information
     */
    get_component_info();

    /*
     * check and lock all release stages
     */
    if (check_release_stages() != 0)
	exit(1);

    /*
     * get default message for log/post
     */
    if (get_log_message() != 0)
	exit(1);

    /*
     * release from object directory
     */
    if (fromstage_p == NULL) {
	do_stage_release((struct stage *)NULL, proj_stages);
	fromstage_p = proj_stages;
    }
    strip = FALSE;
    fromfile = NULL;

    /*
     * release through all remaining stages
     */
    for (stage_p = fromstage_p; stage_p != tostage_p; stage_p = stage_p->next)
	do_stage_release(stage_p, stage_p->next);

    /*
     * make post
     */
    make_post(tostage_p);

    /*
     * all done
     */
    (void) unlink(logfile);

    exit(0);
}

parse_command_line(argc, argv)
int argc;
char **argv;
{
    int i;

    if (argc > 0) {
	if ((prog = rindex(argv[0], '/')) != NULL)
	    prog++;
	else
	    prog = argv[0];
	argc--; argv++;
    } else
	prog = "release";

    havecmdlineinfo = FALSE;
    owner = NULL;
    group = NULL;
    mode = (u_short)-1;
    fromfile = NULL;
    fromstage = NULL;
    tostage = NULL;
    rm = TRUE;
    strip = TRUE;
    post = TRUE;
    logfile = NULL;
    uselog = NULL;
    replaceok = FALSE;
    debug = FALSE;
    quick = FALSE;
    project = NULL;

    while (argc > 0) {
	if (argv[0][0] != '-')
	    break;
	switch (argv[0][1]) {
	case 'f':
	    if (strcmp(argv[0], "-fromfile") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		fromfile = fixuppath(argv[0]);
	    } else if (strcmp(argv[0], "-fromstage") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		fromstage = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 't':
	    if (strcmp(argv[0], "-tostage") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		tostage = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'm':
	    if (argv[0][2] == '\0') {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		if ((i = atoo(argv[1])) < 0)
		    fatal("invalid argument to %s: %s", argv[0], argv[1]);
		argc--; argv++;
		mode = (u_short)i;
		havecmdlineinfo = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'o':
	    if (argv[0][2] == '\0') {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		owner = salloc(argv[0]);
		havecmdlineinfo = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'g':
	    if (argv[0][2] == '\0') {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		group = salloc(argv[0]);
		havecmdlineinfo = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'n':
	    if (strcmp(argv[0], "-norm") == 0) {
		rm = FALSE;
	    } else if (strcmp(argv[0], "-nostrip") == 0) {
		strip = FALSE;
	    } else if (strcmp(argv[0], "-nopost") == 0) {
		post = FALSE;
	    } else
		usage(argv[0]);
	    break;
	case 'l':
	    if (strcmp(argv[0], "-logfile") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		logfile = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'u':
	    if (strcmp(argv[0], "-uselog") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		uselog = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'r':
	    if (strcmp(argv[0], "-replaceok") == 0) {
		replaceok = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'd':
	    if (strcmp(argv[0], "-debug") == 0) {
		debug = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'q':
	    if (argv[0][2] == '\0') {
		quick = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'p':
	    if (strcmp(argv[0], "-project") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		project = argv[0];
	    } else
		usage(argv[0]);
	    break;
	default:
	    usage(argv[0]);
	}
	argc--; argv++;
    }

    /*
     * remaining arguments are target and optional hard links
     */
    if (argc == 0) {
	warn("must specify at least one target file");
	usage((char *) NULL);
    }
    if (argc > 1)
	havecmdlineinfo = TRUE;
    nlinks = argc;
    links = argv;
    for (i = 0; i < nlinks; i++) {
	if (*links[i] != '/')
	    warn("target \"%s\" not absolute", links[i]);
	links[i] = fixuppath(links[i]);
    }
    target = links[0];
    targetlen = strlen(target);
    if (target[targetlen-1] == 'o' && target[targetlen-2] == '.')
	strip = FALSE;

    if (fromstage != NULL) {
	if (fromfile != NULL) {
	    warn("-fromfile and -fromstage are mutually exclusive");
	    usage((char *) NULL);
	}
    } else {
	if (fromstage == NULL && fromfile == NULL) {
	    warn("must specify -fromfile or -fromstage");
	    usage((char *) NULL);
	}
    }
    if (tostage == NULL) {
	warn("must specify -tostage");
	usage((char *) NULL);
    }
}

usage(opt)
char *opt;
{
    if (opt != NULL)
	warn("unknown switch %s", opt);
    quit(1, "usage: %s %s %s %s target [ links ... ]\n", prog,
	 "[ -fromfile <file> | -fromstage <stage> ]",
	 "[ -tostage <stage> ]",
	 "[ -m <mode> -o <owner> -g <group> -norm -nostrip -nopost -q ]");
}

char *fixuppath(path)
char *path;
{
    char buf[MAXPATHLEN];
    register char *p = buf;

    *p = *path;
    while (*path != '\0') {
	path++;
	while (*p == '/' && *path == '/')
	    path++;
	*++p = *path;
    }
    return(salloc(buf));
}

check_project()
{
    char temp[MAXPATHLEN];
    struct stage *stage_p;
    struct field *field_p;
    struct arg_list *args_p;

    if (project_field(&proj, "project_base", &field_p) != 0)
	proj_project_base = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    fatal("improper project_base");
	proj_project_base = args_p->tokens[0];
	if (*proj_project_base != '/')
	    fatal("project_base must be absolute");
    }
    if (project_field(&proj, "source_base", &field_p) != 0)
	fatal("source_base not defined");
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	fatal("improper source_base");
    proj_source_base = args_p->tokens[0];
    if (*proj_source_base != '/') {
	if (proj_project_base == NULL)
	    fatal("project_base not defined, source_base relative");
	(void) concat(temp, sizeof(temp),
		      proj_project_base, "/", proj_source_base, NULL);
	if ((proj_source_base = salloc(temp)) == NULL)
	    fatal("source_base salloc failure");
    }
    if (exists(proj_source_base) < 0)
	fatal("source_base %s not found", proj_source_base);
    (void) concat(temp, sizeof(temp), proj_source_base, "/Makeconf", NULL);
    if (exists(temp) < 0)
	fatal("no source Makeconf file");
    if (project_field(&proj, "rcs_base", &field_p) != 0)
	fatal("rcs_base not defined");
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	fatal("improper rcs_base");
    proj_rcs_base = args_p->tokens[0];
    if (*proj_rcs_base != '/') {
	if (proj_project_base == NULL)
	    fatal("project_base not defined, rcs_base relative");
	(void) concat(temp, sizeof(temp),
		      proj_project_base, "/", proj_rcs_base, NULL);
	if ((proj_rcs_base = salloc(temp)) == NULL)
	    fatal("rcs_base salloc failure");
    }
    if (exists(proj_rcs_base) < 0)
	fatal("rcs_base %s not found", proj_rcs_base);
    (void) concat(temp, sizeof(temp), proj_rcs_base, "/Makeconf,v", NULL);
    if (exists(temp) < 0)
	fatal("no rcs Makeconf file");
    if (project_field(&proj, "release_base", &field_p) != 0)
	fatal("release_base not defined");
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	fatal("improper release_base");
    proj_release_base = args_p->tokens[0];
    if (*proj_release_base != '/') {
	if (proj_project_base == NULL)
	    fatal("project_base not defined, release_base relative");
	(void) concat(temp, sizeof(temp),
		      proj_project_base, "/", proj_release_base, NULL);
	if ((proj_release_base = salloc(temp)) == NULL)
	    fatal("release_base salloc failure");
    }
    if (exists(proj_release_base) < 0)
	fatal("release_base %s not found", proj_release_base);
    if (project_field(&proj, "release_lock", &field_p) != 0)
	proj_release_lock = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    fatal("improper release_lock");
	proj_release_lock = args_p->tokens[0];
	if (*proj_release_lock != '/') {
	    if (proj_project_base == NULL)
		fatal("project_base not defined, release_lock relative");
	    (void) concat(temp, sizeof(temp),
			  proj_project_base, "/", proj_release_lock, NULL);
	    if ((proj_release_lock = salloc(temp)) == NULL)
		fatal("release_lock salloc failure");
	}
    }
    if (project_field(&proj, "stage", &field_p) != 0)
	fatal("release stages not defined");
    parse_stages(field_p->args);
    if (find_stage(tostage, &tostage_p))
	fatal("release stage %s not defined", tostage);
    if (fromstage != NULL) {
	if (find_stage(fromstage, &fromstage_p))
	    fatal("release stage %s not defined", fromstage);
	for (stage_p = proj_stages; stage_p != NULL; stage_p = stage_p->next) {
	    if (stage_p == fromstage_p) {
		if (stage_p == tostage_p)
		    fatal("release stages %s and %s are the same",
			  fromstage, tostage);
		break;
	    } else if (stage_p == tostage_p)
		fatal("unable to release to previous stage %s", tostage);
	}
    }
    if (project_field(&proj, "release_database", &field_p) != 0)
	proj_release_database = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    fatal("improper release_database");
	proj_release_database = args_p->tokens[0];
	if (*proj_release_database != '/') {
	    (void) concat(temp, sizeof(temp),
			  proj_release_base, "/", proj_release_database, NULL);
	    if ((proj_release_database = salloc(temp)) == NULL)
		fatal("release_database salloc failure");
	}
    }
}

char **
alloc_ptrs(name)
char *name;
{
    char *ptr, *p, **pp;
    int i;

    if ((ptr = salloc(name)) == NULL)
	fatal("unable to allocate ptrs");
    i = 1;
    for (p = ptr; *name != '\0'; p++, name++) {
	*p = *name;
	if (*p == ',')
	    i++;
    }
    if ((pp = (char **)calloc((unsigned)i+1, sizeof(char *))) == NULL)
	fatal("unable to allocate ptrs");
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
    int i;

    stage_pp = &proj_stages;
    do {
	*stage_pp = (struct stage *)calloc(1, sizeof(struct stage));
	if (*stage_pp == (struct stage *)NULL)
	    fatal("unable to allocate stage");
	if (args_p->ntokens < 2)
	    fatal("bad stage in project description");
	(*stage_pp)->names = alloc_ptrs(args_p->tokens[0]);
	(*stage_pp)->roots = alloc_ptrs(args_p->tokens[1]);
	for (i = 2; i < args_p->ntokens; i++) {
	    if (strcmp(args_p->tokens[i], "norm") == 0)
		(*stage_pp)->flags |= STAGE_NORM;
	    else if (strcmp(args_p->tokens[i], "nopost") == 0)
		(*stage_pp)->flags |= STAGE_NOPOST;
	    else
		fatal("unknown stage token %s", args_p->tokens[i]);
	}
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

get_default_info()
{
    struct passwd *pw;
    struct group *gr;

    if (mode == (u_short)-1)
	mode = 0755;
    if (owner == NULL) {
	if (mode&S_ISUID)
	    fatal("must specify -o owner for setuid permission");
	uid = (int) getuid();
	if ((pw = getpwuid(uid)) == NULL)
	    fatal("uid %d not found in passwd database", uid);
	owner = salloc(pw->pw_name);
    } else {
	if ((pw = getpwnam(owner)) == NULL)
	    fatal("user %s not found in passwd database", owner);
	uid = pw->pw_uid;
    }
    if (group == NULL) {
	if (mode&S_ISGID)
	    fatal("must specify -g group for setgid permission");
	gid = (int) getgid();
	if ((gr = getgrgid(gid)) == NULL)
	    fatal("gid %d not found in group database", gid);
	group = salloc(gr->gr_name);
    } else {
	if ((gr = getgrnam(group)) == NULL)
	    fatal("user %s not found in group database", group);
	gid = gr->gr_gid;
    }
    return;
}

get_backup_database()
{
    char buf[MAXPATHLEN];
    int status;

    if (!getbool("Restore from backup copy?", FALSE)) {
	unlock_database();
	exit(1);
    }
    initialize_database();
    (void) concat(buf, sizeof(buf),
		  proj_release_database, ".backup", NULL);
    if ((status = restore_database(buf)) != 0) {
	if (status > 0)
	    warn("Encountered %d errors reading backup database", status);
	warn("Unable to restore database from backup copy");
	if (!getbool("Rebuild database from description?", FALSE)) {
	    unlock_database();
	    exit(1);
	}
	initialize_database();
	(void) concat(buf, sizeof(buf),
		      proj_release_database, ".description", NULL);
	if ((status = read_database_description(buf)) != 0) {
	    if (status > 0)
		warn("Encountered %d errors reading database", status);
	    unlock_database();
	    fatal("Unable to rebuild release database, cannot continue");
	}
    }
}

compare_component_info(file_ptr, file_info_ptr)
struct file *file_ptr;
struct file_info *file_info_ptr;
{
    char buf[MAXPATHLEN];
    char srcdir[MAXPATHLEN];
    char *ptr;
    char **newlinks;
    int i;
    int temp;
    struct file *link_file_ptr;
    int makechanges;

    makechanges = FALSE;
    if (owner != NULL &&
	strcmp(owner, file_info_ptr->owner->data) != 0) {
	diag("owner difference: database %s vs. command line %s",
	     file_info_ptr->owner->data, owner);
	if (getbool("Replace database information?", FALSE))
	    makechanges = TRUE;
	else {
	    (void) free(owner);
	    owner = salloc(file_info_ptr->owner->data);
	}
    }
    if (group != NULL &&
	strcmp(group, file_info_ptr->group->data) != 0) {
	diag("group difference: database %s vs. command line %s",
	     file_info_ptr->group->data, group);
	if (getbool("Replace database information?", FALSE))
	    makechanges = TRUE;
	else {
	    (void) free(group);
	    group = salloc(file_info_ptr->group->data);
	}
    }
    if (mode != (u_short)-1 && mode != file_info_ptr->mode) {
	diag("mode difference: database %#o vs. command line %#o",
	     file_info_ptr->mode, mode);
	if (getbool("Replace database information?", FALSE))
	    makechanges = TRUE;
	else
	    mode = file_info_ptr->mode;
    }
    if ((ptr = getenv("MAKEDIR")) != NULL) {
	directory_path(file_ptr->srcdir, srcdir);
	if (strcmp(ptr, srcdir) != 0) {
	    diag("%s: database %s vs. environment %s",
		 "source directory difference", srcdir, ptr);
	    if (getbool("Replace database information?", FALSE))
		makechanges = TRUE;
	}
    }
    temp = 1;
    if (file_ptr->links != NULL) {
	link_file_ptr = file_ptr->links;
	while (link_file_ptr != file_ptr) {
	    temp++;
	    link_file_ptr = link_file_ptr->links;
	}
    }
    if (nlinks == temp) {
	if (nlinks == 1)
	    return(makechanges);
	link_file_ptr = file_ptr->links;
	while (link_file_ptr != file_ptr) {
	    component_path(link_file_ptr, buf);
	    for (i = 1; i < nlinks; i++)
		if (strcmp(buf, links[i]) == 0)
		    break;
	    if (i == nlinks)
		break;
	    link_file_ptr = link_file_ptr->links;
	}
	if (link_file_ptr == file_ptr)
	    return(makechanges);
    }
    diag("link difference:");
    diag("database:");
    link_file_ptr = file_ptr->links;
    while (link_file_ptr != file_ptr) {
	component_path(link_file_ptr, buf);
	diag("  %s", buf);
	link_file_ptr = link_file_ptr->links;
    }
    diag("command line:");
    for (i = 1; i < nlinks; i++)
	diag("  %s", links[i]);
    if (getbool("Replace database information?", FALSE))
	return(TRUE);
    if (temp > nlinks) {
	links = (char **) calloc((unsigned)temp, sizeof(char *));
	if (links == NULL) {
	    unlock_database();
	    fatal("Unable to allocate new link table");
	}
    }
    links[0] = target;
    nlinks = temp;
    if (nlinks == 1)
	return(makechanges);
    link_file_ptr = file_ptr->links;
    for (i = 1; i < nlinks; i++) {
	component_path(link_file_ptr, buf);
	links[i] = fixuppath(buf);
	link_file_ptr = link_file_ptr->links;
    }
    if (link_file_ptr != file_ptr) {
	unlock_database();
	fatal("Inconsistant component links");
    }
    return(makechanges);
}

get_component_info()
{
    char buf[MAXPATHLEN];
    char srcdir[MAXPATHLEN];
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN];
    char *p, *ptr;
    int i;
    struct file *file_ptr;
    struct file_info *file_info_ptr;
    int status;
    int temp;
    struct string *comp_owner, *comp_group;
    u_short comp_mode;
    int makechanges;
    int save_database_changes;
    struct passwd *pw;
    struct group *gr;

    if ((user = getenv("USER")) == NULL)
	fatal("USER environment variable not set");
    if (proj_release_database == NULL) {
	get_default_info();
	return;
    }
    initialize_database();
    if (lock_database(proj_release_database) != 0)
	fatal("Unable to lock release database");
    save_database_changes = FALSE;
    if ((status = restore_database(proj_release_database)) != 0) {
	if (status > 0)
	    warn("Encountered %d errors restoring release database", status);
	warn("Unable to restore release database");
	get_backup_database();
	save_database_changes = TRUE;
    }
    makechanges = FALSE;
    if (lookup_component(target, &file_ptr, &file_info_ptr) != 0) {
	warn("Unable to find component %s in release database", target);
	if (!getbool("Do you wish to continue?", FALSE)) {
	    unlock_database();
	    exit(1);
	}
	if (create_component(target, &file_ptr, &file_info_ptr) != 0) {
	    unlock_database();
	    fatal("Unable to create component");
	}
	makechanges = TRUE;
    } else {
	/*
	 * target may have been a link name, so we follow the list of links
	 * until we are at the main component name.
	 */
	while (file_ptr->links != NULL && file_ptr->srcdir == NULL)
	    file_ptr = file_ptr->links;
	if (havecmdlineinfo)
	    makechanges = compare_component_info(file_ptr, file_info_ptr);
    }
    if (makechanges) {
	diag("[ target %s ]", target);
	if (owner == NULL)
	    owner = file_info_ptr->owner->data;
	(void) getstr("Owner", owner, buf);
	if (create_string(buf, &comp_owner) != 0) {
	    unlock_database();
	    fatal("Unable to create owner string");
	}
	if (group == NULL)
	    group = file_info_ptr->group->data;
	(void) getstr("Group", group, buf);
	if (create_string(buf, &comp_group) != 0) {
	    unlock_database();
	    fatal("Unable to create group string");
	}
	if (mode == (u_short)-1)
	    mode = file_info_ptr->mode;
	(void) sprintf(buf, "%#o", mode);
	(void) getstr("Mode", buf, buf);
	if ((temp = atoo(buf)) < 0) {
	    unlock_database();
	    fatal("Invalid mode %s", buf);
	}
	comp_mode = (u_short) temp;
	path(target, dirbuf, filbuf);
	if (rcssrcdir(target))
	    (void) strcpy(buf, target);
	else {
	    ptr = target;
	    p = buf;
	    while (*p = *ptr++)
		p++;
	    ptr = NULL;
	    while (p >= buf && *p != '/') {
		if (*p == '.')
		    ptr = p;
		p--;
	    }
	    if (ptr != NULL && ptr != p + 1)
		*ptr = '\0';
	    else
		ptr = NULL;
	    if (ptr == NULL || !rcssrcdir(buf))
		(void) strcpy(buf, dirbuf);
	}
	if (buf[0] == '.' && buf[1] == '\0')
	    buf[0] = '\0';
	if (!rcssrcdir(buf)) {
	    unlock_database();
	    fatal("source directory %s is not a directory", buf);
	}
	(void) getstr("Source directory path", buf, srcdir);
	if (nlinks == 1)
	    diag("No other links to %s", target);
	else {
	    diag("Links to %s:", target);
	    for (i = 1; i < nlinks; i++)
		diag("  %s", links[i]);
	}
	if (!getbool("Is this link information correct?", TRUE)) {
	    (void) getstr("How many links are there including the target?",
			  "1", buf);
	    if ((temp = atoi(buf)) < 0) {
		unlock_database();
		fatal("Invalid link count");
	    }
	    if (temp > nlinks) {
		links = (char **) calloc((unsigned)temp, sizeof(char *));
		if (links == NULL) {
		    unlock_database();
		    fatal("Unable to allocate new link table");
		}
		links[0] = target;
	    }
	    nlinks = temp;
	    for (i = 1; i < nlinks; i++) {
		(void) getstr("Link path", "", buf);
		if (*buf != '/')
		    warn("target \"%s\" not absolute", buf);
		links[i] = fixuppath(buf);
	    }
	}
	diag("");
	diag("[ target %s ]", target);
	diag("  [ owner %s ]", comp_owner->data);
	diag("  [ group %s ]", comp_group->data);
	diag("  [ mode %#o ]", comp_mode);
	diag("  [ source directory %s ]", srcdir);
	for (i = 1; i < nlinks; i++)
	    diag("  [ linked to %s ]", links[i]);
	if (!getbool("Use this information?", FALSE)) {
	    unlock_database();
	    exit(1);
	}
	if (set_component_info(file_ptr, &file_info_ptr,
			       srcdir, nlinks, links,
			       comp_owner, comp_group, comp_mode) != 0) {
	    unlock_database();
	    fatal("Unable to set component information");
	}
	save_database_changes = TRUE;
    }
    if (save_database_changes &&
	getbool("Update changes to database?", FALSE)) {
	(void) concat(buf, sizeof(buf),
		      proj_release_database, ".new", NULL);
	(void) unlink(buf);
	if ((status = save_database(buf)) != 0) {
	    if (status > 0)
		warn("Encountered %d errors saving database", status);
	    unlock_database();
	    fatal("Unable to update database changes");
	}
	(void) concat(buf, sizeof(buf),
		      proj_release_database, ".newdesc", NULL);
	(void) unlink(buf);
	if ((status = write_database_description(buf)) != 0) {
	    if (status > 0)
		warn("Encountered %d errors writing database", status);
	    unlock_database();
	    fatal("Unable to update database changes");
	}
    }
    unlock_database();

    owner = file_info_ptr->owner->data;
    if ((pw = getpwnam(owner)) == NULL)
	fatal("user %s not found in passwd database", owner);
    uid = pw->pw_uid;
    group = file_info_ptr->group->data;
    if ((gr = getgrnam(group)) == NULL)
	fatal("user %s not found in group database", group);
    gid = gr->gr_gid;
    mode = file_info_ptr->mode;
}

rcssrcdir(target)
char *target;
{
    char buf[MAXPATHLEN];
    struct stat statb;

    (void) concat(buf, sizeof(buf), proj_rcs_base, target, NULL);
    if (stat(buf, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
	return(TRUE);
    (void) concat(buf, sizeof(buf), proj_source_base, target, NULL);
    if (stat(buf, &statb) == 0 && (statb.st_mode&S_IFMT) == S_IFDIR)
	return(TRUE);
    return(FALSE);
}

check_release_stages()
{
    int status;
    struct stage *stage_p;
    struct stat statb;
    int fd;

    for (;;) {
	/*
	 * lock release base directory
	 */
	if (lock_release() != 0)
	    return(1);

	/*
	 * check release from object directory
	 */
	if ((stage_p = fromstage_p) == NULL) {
	    while (stat(fromfile, &statb) < 0) {
		ewarn("stat %s", fromfile);
		if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
		    return(1);
	    }
	    if ((statb.st_mode&S_IFMT) != S_IFREG)
		fatal("%s is not a regular file", fromfile);
	    while ((fd = open(fromfile, O_RDONLY, 0)) < 0) {
		ewarn("open %s", fromfile);
		if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
		    return(1);
	    }
	    (void) close(fd);
	    stage_p = proj_stages;
	}

	/*
	 * check release through intermediate stages
	 */
	for (;;) {
	    if ((status = check_release_stage(stage_p)) < 0)
		return(1);
	    if (status > 0)
		break;
	    if (stage_p == tostage_p)
		return(0);
	    stage_p = stage_p->next;
	}

	/*
	 * unlock and wait
	 */
	(void) close(lock_fd);
	if (!getbool("Ready to re-lock?", TRUE))
	    return(1);
    }
}

lock_release()
{
    char buf[MAXPATHLEN], datebuf[MAXPATHLEN];
    char *ptr;
    struct tm *tm;
    time_t now;
    char *tty;

    if (proj_release_lock == NULL) {
	diag("[ release not locked for exclusive access ]");
	return(0);
    }
    for (;;) {
	if ((lock_fd = open(proj_release_lock, O_RDWR|O_CREAT, 0600)) < 0)
	    ewarn("open %s", proj_release_lock);
	else {
	    if (flock(lock_fd, LOCK_EX|LOCK_NB) == 0)
		break;
	    if (errno != EWOULDBLOCK)
		ewarn("flock");
	    else {
		diag("release lock busy:");
		(void) putc('\t', stderr);
		(void) fflush(stderr);
		(void) filecopy(lock_fd, fileno(stderr));
		if (!getbool("Do you want to wait for the lock?", TRUE))
		    return(1);
		diag("[ waiting for exclusive access lock ]");
		if (flock(lock_fd, LOCK_EX) == 0)
		    break;
		ewarn("flock");
	    }
	    (void) close(lock_fd);
	}
	if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    if ((tty = ttyname(0)) == NULL)
	tty = "tty??";
    now = time((time_t *) 0);
    tm = localtime(&now);
    fdate(datebuf, "%3Month %02day %year %02hour:%02min:%02sec", tm);
    ptr = concat(buf, sizeof(buf),
		 user, " on ", tty, " at ", datebuf, "\n", NULL);
    errno = 0;
    if (lseek(lock_fd, (long)0, L_SET) == -1 && errno != 0)
	ewarn("lseek");
    if (write(lock_fd, buf, ptr-buf) != ptr-buf)
	ewarn("write");
    if (ftruncate(lock_fd, (off_t)(ptr-buf)) < 0)
	ewarn("ftruncate");
    diag("[ release locked for exclusive access ]");
    return(0);
}

check_release_stage(stage_p)
struct stage *stage_p;
{
    char base[MAXPATHLEN];
    struct stat statb;
    char *ptr;

    ptr = stage_p->roots[0];
    if (*ptr == '/')
	(void) strcpy(base, ptr);
    else
	(void) concat(base, sizeof(base), proj_release_base, "/", ptr, NULL);
    if (stat(base, &statb) < 0)
	efatal("stat %s", base);
    if ((statb.st_mode&S_IFMT) != S_IFDIR)
	fatal("%s: not a directory", base);

    if (check_release(base) != 0)
	return(-1);

    if (stage_p == fromstage_p || stage_p->next == NULL)
	return(0);

    return(check_log(base));
}

check_release(base)
char *base;
{
    char temp[MAXPATHLEN];
    int havetarget;
    int i;
    int fd;
    struct stat statb;

    (void) concat(fulltarget, sizeof(fulltarget), base, target, NULL);
    if (stat(fulltarget, &statb) < 0) {
	while (ckmakepath(fulltarget, base, target) != 0) {
	    warn("makepath failed for %s", fulltarget);
	    if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
		return(1);
	}
	havetarget = FALSE;
    } else {
	havetarget = TRUE;
	if ((statb.st_mode&S_IFMT) != S_IFREG)
	    fatal("%s is not a regular file", fulltarget);
    }
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), base, links[i], NULL);
	if (exists(temp) < 0) {
	    if (ckmakepath(temp, base, links[i]) != 0) {
		warn("makepath failed for %s", temp);
		if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
		    return(1);
	    }
	}
    }
    if (havetarget && checkstate(fulltarget, &statb) > 0) {
	if (!getbool("Do you wish to continue?", 0))
	    return(1);
	diag("");
    }

    (void) concat(tologdb, sizeof(tologdb), base, "/.post.log", NULL);
    (void) concat(towhodb, sizeof(towhodb), base, "/.installers.log", NULL);

    /*
     * check that all log files exist and are readable
     */
    if (exists(tologdb) < 0) {
	while ((fd = open(tologdb, O_WRONLY|O_CREAT, 0644)) < 0) {
	    ewarn("open %s", tologdb);
	    if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
		return(1);
	}
	(void) close(fd);
    }
    if (exists(towhodb) < 0) {
	while ((fd = open(towhodb, O_WRONLY|O_CREAT, 0644)) < 0) {
	    ewarn("open %s", towhodb);
	    if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
		return(1);
	}
	(void) close(fd);
    }
    while ((fd = open(tologdb, O_RDONLY, 0)) < 0) {
	ewarn("open %s", tologdb);
	if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    (void) close(fd);
    while ((fd = open(towhodb, O_RDONLY, 0)) < 0) {
	ewarn("open %s", towhodb);
	if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    (void) close(fd);
    (void) concat(temp, sizeof(temp), base, "/.release.chk", NULL);
    while ((fd = open(temp, O_WRONLY|O_CREAT, 0644)) < 0) {
	ewarn("open %s", temp);
	if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    (void) close(fd);
    (void) unlink(temp);
    (void) concat(temp, sizeof(temp), fulltarget, ".chk", NULL);
    while ((fd = open(temp, O_WRONLY|O_CREAT, 0644)) < 0) {
	ewarn("open %s", temp);
	if (getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    (void) close(fd);
    (void) unlink(temp);
    return(0);
}

checkstate(file, statp)
char *file;
struct stat *statp;
{
    int errs = 0;

    /*
     * make from target checks
     */
    if (statp->st_uid != uid) {
	diag("\n*** WARNING ***");
	diag("The owner of '%s' is not consistent with", file);
	diag("the owner given on the command line.\n");
	diag("The uid of '%s' is %d.", file, statp->st_uid);
	diag("The uid given is %d.\n", uid);
	errs++;
    }
    if (statp->st_gid != gid) {
	diag("\n*** WARNING ***");
	diag("The group of '%s' is not consistent with", file);
	diag("the group given on the command line.\n");
	diag("The gid of '%s' is %d.", file, statp->st_gid);
	diag("The gid given is %d.\n", gid);
	errs++;
    }
    if ((statp->st_mode&07777) != (mode&07777)) {
	diag("\n*** WARNING ***");
	diag("The mode of '%s' is not consistent with", file);
	diag("the mode given on the command line.\n");
	diag("The mode of '%s' is %#o.", file, statp->st_mode&07777);
	diag("The mode given is %#o.\n", mode&07777);
	errs++;
    }
    if (statp->st_nlink != nlinks) {
	diag("\n*** WARNING ***");
	diag("The number of links to '%s' is not consistent with", file);
	diag("the number of files given on the command line.\n");
	diag("The number of links to '%s' is %d.", file, statp->st_nlink);
	diag("The number of files given is %d.\n", nlinks);
	errs++;
    }
    return(errs);
}

check_log(base)
char *base;
{
    char temp[MAXPATHLEN];
    char buf[MAXPATHLEN];
    int keptwho, keptlog;
    int fd;
    int query;
    int len;
    int key;
    FILE *fp;

    (void) strcpy(temp, "/tmp/release.XXXXXX");
    if ((fd = mkstemp(temp)) < 0) {
	ewarn("mkstemp %s", temp);
	return(-1);
    }
    (void) concat(tologdb, sizeof(tologdb), base, "/.post.log", NULL);
    (void) concat(towhodb, sizeof(towhodb), base, "/.installers.log", NULL);
    if (keep_lines(towhodb, fd, &keptwho) != 0) {
	(void) close(fd);
	(void) unlink(temp);
	return(-1);
    }
    if (keep_lines(tologdb, fd, &keptlog) != 0) {
	(void) close(fd);
	(void) unlink(temp);
	return(-1);
    }
    (void) close(fd);
    if (!keptwho && !keptlog) {
	(void) unlink(temp);
	return(0);
    }
    diag("\n*** WARNING ***");
    diag("There is already a verion of %s installed.\n", fulltarget);
    (void) fflush(stderr);
    if ((fp = fopen(temp, "r")) == NULL) {
	ewarn("fopen %s", temp);
	(void) unlink(temp);
	return(-1);
    }
    query = !replaceok;
    if (keptwho <= 0) {
	diag("%s\t???\tunknown");
	query = TRUE;
    } else {
	len = strlen(user);
	while (keptwho-- > 0) {
	    if (fgets(buf, sizeof(buf), fp) == NULL) {
		ewarn("fgets %s", temp);
		(void) fclose(fp);
		(void) unlink(temp);
		return(-1);
	    }
	    (void) fputs(buf, stdout);
	    if (strncmp(buf, user, len) != 0 ||
		(buf[len] != ' ' && buf[len] != '\t')) {
		query = TRUE;
		break;
	    }
	}
    }
    (void) ffilecopy(fp, stdout);
    (void) fclose(fp);
    (void) unlink(temp);
    diag("");
    if (!query)
	return(0);
    if (quick)
	return(-1);
    key = getstab("Abort, unlock or replace?", aur_table, "");
    if (key == ABORT)
	return(-1);
    diag("");
    if (key == REPLACE)
	return(0);
    return(1);
}

get_old_log(stage_p, fd_p, temp)
struct stage *stage_p;
int *fd_p;
char *temp;
{
    char buf[MAXPATHLEN];
    char ifile[MAXPATHLEN];
    char wbuf[BUFSIZ];
    char *ptr;
    int logfd;
    int kept;
    int keptwho;
    char *who;
    char *when;

    if (stage_p->next == NULL)
	return(0);
    if (stage_p != tostage_p)
	if (get_old_log(stage_p->next, fd_p, temp) != 0)
	    return(1);
    ptr = stage_p->roots[0];
    if (*ptr == '/') {
	(void) concat(buf, sizeof(buf), ptr, "/.post.log", NULL);
	(void) concat(ifile, sizeof(ifile), ptr, "/.installers.log", NULL);
    } else {
	(void) concat(buf, sizeof(buf),
		      proj_release_base, "/", ptr, "/.post.log", NULL);
	(void) concat(ifile, sizeof(ifile),
		      proj_release_base, "/", ptr, "/.installers.log", NULL);
    }
    if (keep_lines(ifile, *fd_p, &keptwho) != 0)
	return(1);
    (void) close(*fd_p);
    who = NULL;
    when = NULL;
    if (keptwho) {
	FILE *fp;

	fp = fopen(temp, "r");
	if (fp == NULL) {
	    ewarn("fopen %s", temp);
	    return(1);
	}
	if (fgets(wbuf, sizeof(wbuf)-1, fp) != NULL) {
	    char *p = wbuf;

	    who = nxtarg(&p, " \t\n");
	    when = nxtarg(&p, "\n");
	}
	(void) fclose(fp);
    }
    (void) unlink(temp);
    if ((*fd_p = open(temp, O_WRNEW, 0600)) < 0) {
	ewarn("open %s", temp);
	return(1);
    }
    if (who) {
	char sbuf[BUFSIZ];
	int i,l;

	(void) sprintf(sbuf, "[ Previous changes by %s (to %s on %s) ]\n",
		 who, stage_p->names[0], when);
	l = strlen(sbuf);
	i = write(*fd_p, sbuf, l);
	if (i != l) {
	    ewarn("write error (%d, not %d)", i, l);
	    return(1);
	}
    }
    if (keep_lines(buf, *fd_p, &kept) != 0)
	return(1);
    if (!kept)
	return(0);
    if (havelog&(HL_TO|HL_FROM)) {
	if ((logfd = open(logfilebuf, O_RDONLY, 0)) < 0) {
	    ewarn("open %s", logfilebuf);
	    return(1);
	}
	if (write(*fd_p, "\n", 1) != 1) {
	    ewarn("write");
	    return(1);
	}
	if (filecopy(logfd, *fd_p) < 0) {
	    ewarn("filecopy");
	    return(1);
	}
	(void) close(logfd);
    }
    (void) close(*fd_p);
    if (rename(temp, logfilebuf) < 0) {
	ewarn("rename");
	return(1);
    }
    if ((*fd_p = open(temp, O_WRNEW, 0600)) < 0) {
	ewarn("open %s", temp);
	return(1);
    }
    if (stage_p == fromstage_p)
	havelog |= HL_FROM;
    else
	havelog |= HL_TO;
    return(0);
}

get_log_message()
{
    char temp[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char *otarget;
    int otargetlen;
    int fd;
    int logfd;
    char *ptr;
    int kept;
    enum option command;
    char *defans;
    FILE *fp;
    struct stage *stage_p;
    struct stat statb;

    (void) strcpy(logfilebuf, "/tmp/release-log.XXXXXX");
    if ((logfd = mkstemp(logfilebuf)) < 0)
	efatal("mkstemp %s", logfilebuf);
    if (uselog != NULL) {
	otarget = target;
	otargetlen = targetlen;
	target = uselog;
	targetlen = strlen(uselog);
	ptr = tostage_p->roots[0];
	if (*ptr == '/')
	    (void) concat(buf, sizeof(buf), ptr, "/.post.log", NULL);
	else
	    (void) concat(buf, sizeof(buf),
			  proj_release_base, "/", ptr, "/.post.log", NULL);
	if (keep_lines(buf, logfd, &kept) != 0)
	    return(1);
	if (!kept)
	    fatal("log message not found for %s", uselog);
	target = otarget;
	targetlen = otargetlen;
	havelog = HL_FILE;
	if (close(logfd) < 0) {
	    ewarn("close");
	    return(1);
	}
    } else if (logfile != NULL) {
	if ((fd = open(logfile, O_RDONLY, 0)) < 0)
	    efatal("open %s", logfile);
	if (filecopy(fd, logfd) < 0)
	    efatal("filecopy %s", logfile);
	if (close(fd) < 0)
	    efatal("close");
	havelog = HL_FILE;
	if (close(logfd) < 0) {
	    ewarn("close");
	    return(1);
	}
    } else {
	if (close(logfd) < 0) {
	    ewarn("close");
	    return(1);
	}
	havelog = 0;
	(void) strcpy(temp, "/tmp/release.XXXXXX");
	if ((fd = mkstemp(temp)) < 0)
	    efatal("mkstemp %s", temp);
	if ((stage_p = fromstage_p) == NULL)
	    stage_p = proj_stages;
	if (get_old_log(stage_p, &fd, temp) != 0) {
	    (void) close(fd);
	    (void) unlink(temp);
	    return(1);
	}
	(void) close(fd);
	(void) unlink(temp);
    }
    logfile = logfilebuf;

    /*
     * if not posting, no conflicts and have log, just return
     */
    if ((havelog&HL_TO) == 0 && !post && (havelog&(HL_FILE|HL_FROM)))
	return(0);

    /*
     * If no conflicts and have log, type it, otherwise edit
     */
    if (havelog != 0) {
	command = TYPE;
	diag("\nPlease update the following message, if necessary.");
    } else {
	command = EDIT;
	diag("\nPlease compose a log message for %s.", fulltarget);
    }

    /*
     * parse commands
     */
    for (;;) {
	switch (command) {
	case EDIT:
	    (void) editor(logfile, "Entering editor.");
	    defans = "type";
	    break;
	case TYPE:
	    if ((fp = fopen(logfile, "r")) == NULL)
		break;
	    printf("\n");
	    (void) ffilecopy(fp, stdout);
	    (void) fclose(fp);
	    defans = "log";
	    break;
	case LOG:
	    defans = NULL;
	    break;
	}
	if (defans == NULL) {
	    if (stat(logfile, &statb) == 0 && statb.st_size > 0)
		break;
	    diag("You are not allowed to log an empty message");
	    command = EDIT;
	} else {
	    command = logopt[getstab("Edit, type, log", logtab, defans)];
	    clearerr(stdin);
	}
    }
    return(0);
}

do_stage_release(fstage_p, tstage_p)
struct stage *fstage_p, *tstage_p;
{
    struct stat statb;
    char *ptr;
    int saverm = rm, savestrip = strip;
    int i;

    /*
     * check source
     */
    if (fstage_p != NULL) {
	ptr = fstage_p->roots[0];
	if (*ptr == '/')
	    (void) strcpy(frombase, ptr);
	else
	    (void) concat(frombase, sizeof(frombase),
			  proj_release_base, "/", ptr, NULL);
	if (stat(frombase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
	    efatal("%s", frombase);
	(void) concat(fullsource, sizeof(fullsource),
		      frombase, target, NULL);
    } else
	(void) strcpy(fullsource, fromfile);

    /*
     * check target
     */
    ptr = tstage_p->roots[0];
    if (*ptr == '/')
	(void) strcpy(tobase, ptr);
    else
	(void) concat(tobase, sizeof(tobase),
		      proj_release_base, "/", ptr, NULL);
    if (stat(tobase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
	fatal("%s is not a directory", tobase);
    (void) concat(fulltarget, sizeof(fulltarget), tobase, target, NULL);

    printf("[ %s -> %s ]\n",
	   fstage_p == NULL ? "BUILD" : fstage_p->names[0],
	   tstage_p->names[0]);
    /*
     * set flags based on target (all false on backups)
     */
    check = TRUE;
    if (rm && (tstage_p->flags&STAGE_NORM))
	rm = FALSE;

    /*
     * release to main target directory
     */
    do_release();

    /*
     * release to backup target directories
     */
    if (tstage_p->roots[1] != NULL) {
	fromfile = NULL;
	(void) strcpy(fullsource, fulltarget);
	(void) strcpy(frombase, tobase);
	check = FALSE;
	rm = FALSE;
	strip = FALSE;
	for (i = 1; tstage_p->roots[i] != NULL; i++) {
	    ptr = tstage_p->roots[i];
	    if (*ptr == '/')
		(void) strcpy(tobase, ptr);
	    else
		(void) concat(tobase, sizeof(tobase),
			      proj_release_base, "/", ptr, NULL);
	    if (stat(tobase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
		warn("%s is not a directory", tobase);
	    else {
		(void) concat(fulltarget, sizeof(fulltarget),
			      tobase, target, NULL);
		do_release();
	    }
	}
    }

    /*
     * restore flags
     */
    rm = saverm;
    strip = savestrip;
}

do_release()
{
    char targettmp[MAXPATHLEN];
    char fromlogtmp[MAXPATHLEN];
    char fromwhotmp[MAXPATHLEN];
    char tologtmp[MAXPATHLEN];
    char towhotmp[MAXPATHLEN];
    int status = 0;

    /*
     * generate any files that we will need later
     */
    if (fromfile == NULL) {
	(void) concat(fromlogdb, sizeof(fromlogdb),
		      frombase, "/.post.log", NULL);
	if (check) {
	    (void) concat(fromwhodb, sizeof(fromwhodb),
			  frombase, "/.installers.log", NULL);
	    (void) concat(fromlogtmp, sizeof(fromlogtmp),
			  fromlogdb, ".tmp", NULL);
	    (void) concat(fromwhotmp, sizeof(fromwhotmp),
			  fromwhodb, ".tmp", NULL);
	}
    }
    (void) concat(tologdb, sizeof(tologdb), tobase, "/.post.log", NULL);
    (void) concat(towhodb, sizeof(towhodb), tobase, "/.installers.log", NULL);
    (void) concat(tologtmp, sizeof(tologtmp), tologdb, ".tmp", NULL);
    (void) concat(towhotmp, sizeof(towhotmp), towhodb, ".tmp", NULL);
    (void) concat(targettmp, sizeof(targettmp), fulltarget, ".tmp", NULL);

    /*
     * check out source for release
     */
    status = check_source_and_targets();

    /*
     * check any files that we will need later
     */
    if (status == 0)
	status = check_files();

    /*
     * bail out here if any problems so far
     */

    if (status < 0)
	exit(1);
    if (status > 0)
	return;

    /*
     * We are committed at this point, the only errors possible should
     * be file errors.  Other errors should have been checked already.
     * First, do as much as possible to temporary files.
     */

    /*
     * make a new post log
     */
    if (status == 0 && check && fromfile == NULL)
	status = create_old_log_db(fromlogtmp);
    if (status == 0)
	status = create_new_log_db(tologtmp);

    /*
     * make a new installer log
     */
    if (status == 0 && check && fromfile == NULL)
	status = create_old_who_db(fromwhotmp);
    if (status == 0)
	status = create_new_who_db(towhotmp);

    /*
     * make a new target
     */
    if (status == 0)
	status = create_new_target(targettmp);

    /*
     * done as much as we can before making targets and links so
     * install new targets
     */
    if (status == 0) {
	diag("%s: installing as %s", target, fulltarget);
	if (check && fromfile == NULL)
	    status = install_old_dbs(fromlogtmp, fromwhotmp);
    }
    if (status == 0)
	status = install_new_dbs(tologtmp, towhotmp);
    else {
	(void) unlink(fromlogtmp);
	(void) unlink(fromwhotmp);
    }
    if (status == 0)
	status = install_targets(targettmp);
    else {
	(void) unlink(tologtmp);
	(void) unlink(towhotmp);
    }
    if (status != 0)
	(void) unlink(targettmp);

    if (status < 0)
	exit(1);

    /*
     * clean up old stuff
     */
    if (status == 0 && rm)
	remove_old_sources();
}

check_source_and_targets()
{
    int i;
    int fd;
    struct stat tostatb;
    struct stat fromstatb;
    char temp[MAXPATHLEN];
    char *refbase = (fromfile != NULL) ? NULL : frombase;
    int status;

    while (stat(fullsource, &fromstatb) < 0) {
	ewarn("stat %s", fullsource);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }

    if ((fromstatb.st_mode&S_IFMT) != S_IFREG)
	fatal("%s is not a regular file", fullsource);
    source_tv[0].tv_sec = fromstatb.st_atime;
    source_tv[1].tv_sec = fromstatb.st_mtime;
    source_tv[0].tv_usec = source_tv[1].tv_usec = 0;
    while ((fd = open(fullsource, O_RDONLY, 0)) < 0) {
	ewarn("open %s", fullsource);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    (void) close(fd);

    /*
     * check out targets for release
     */
    if (stat(fulltarget, &tostatb) < 0) {
	while (ckmakepath(fulltarget, refbase, target) != 0) {
	    warn("makepath failed for %s", fulltarget);
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		return(status);
	    }
	}
    } else {
	if ((tostatb.st_mode&S_IFMT) != S_IFREG)
	    fatal("%s is not a regular file", fulltarget);
	if (fromstatb.st_dev == tostatb.st_dev &&
	    fromstatb.st_ino == tostatb.st_ino)
	    fatal("%s and %s are identical", fullsource, fulltarget);
    }
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), tobase, links[i], NULL);
	if (exists(temp) < 0) {
	    if (ckmakepath(temp, refbase, links[i]) != 0) {
		warn("makepath failed for %s", temp);
		if ((status = getarc()) != 0) {
		    diag("%s: aborting installation", fulltarget);
		    return(status);
		}
	    }
	}
    }

    return(0);
}

ckmakepath(pathname, refbase, refpath)
char *pathname, *refbase, *refpath;
{
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN];
    int status;

    path(pathname, dirbuf, filbuf);
    if (exists(dirbuf) == 0)
	return(0);
    if (refbase != NULL) {
	(void) concat(filbuf, sizeof(filbuf), refbase, refpath, NULL);
	refpath = filbuf;
    }
    if (exists(refpath) < 0)
	refpath = NULL;
    status = makepath(pathname, refpath, TRUE, TRUE);
    if (status != 0)
	return(status);
    if (exists(dirbuf) < 0)
	return(-1);
    return(0);
}

try_to_open(file, how, mode)
char *file;
int how, mode;
{
    int fd;
    int status;

    while ((fd = open(file, how, mode)) < 0) {
	ewarn("open %s", file);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    (void) close(fd);
    return(0);
}

check_files()
{
    char temp[MAXPATHLEN];
    int status;

    /*
     * check that all log files exist and are readable
     */
    if (fromfile == NULL) {
	if (exists(fromlogdb) < 0 &&
	    (status = try_to_open(fromlogdb,O_WRONLY|O_CREAT,0644)) != 0)
	    return(status);
	if (check && exists(fromwhodb) < 0 &&
	    (status = try_to_open(fromwhodb, O_WRONLY|O_CREAT, 0644)) != 0)
	    return(status);
	if ((status = try_to_open(fromlogdb, O_RDONLY, 0)) != 0)
	    return(status);
	if (check && (status = try_to_open(fromwhodb, O_RDONLY, 0)) != 0)
	    return(status);
    }
    if (exists(tologdb) < 0 &&
	(status = try_to_open(tologdb, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    if (exists(towhodb) < 0 &&
	(status = try_to_open(towhodb, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    if ((status = try_to_open(tologdb, O_RDONLY, 0)) != 0)
	return(status);
    if ((status = try_to_open(towhodb, O_RDONLY, 0)) != 0)
	return(status);

    /*
     * check that we can create files in target directories
     */
    if (check && fromfile == NULL) {
	(void) concat(temp, sizeof(temp), frombase, "/.release.chk", NULL);
	if ((status = try_to_open(temp, O_WRONLY|O_CREAT, 0644)) != 0)
	    return(status);
	(void) unlink(temp);
    }
    (void) concat(temp, sizeof(temp), tobase, "/.release.chk", NULL);
    if ((status = try_to_open(temp, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    (void) unlink(temp);
    (void) concat(temp, sizeof(temp), fulltarget, ".chk", NULL);
    if ((status = try_to_open(temp, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    (void) unlink(temp);
    return(0);
}

keep_lines(filename, fd, kept)
char *filename;
int fd;
int *kept;
{
    FILE *fp;
    char buf[BUFSIZ];
    int status;
    int len;
    char *ptr;

    if ((fp = fopen(filename, "r")) == NULL) {
	ewarn("reopen %s", filename);
	return(1);
    }
    ptr = buf + targetlen;
    *kept = FALSE;
    while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
	if (*buf != *target || bcmp(buf, target, targetlen) != 0 ||
	    (*ptr != ' ' && *ptr != '\t'))
	    continue;
	len = strlen(ptr+1);
	if ((status = write(fd, ptr+1, len)) != len) {
	    ewarn("write error (%d, not %d)", status, len);
	    (void) fclose(fp);
	    return(1);
	}
	*kept = TRUE;
    }
    if (ferror(fp) || fclose(fp) == EOF) {
	ewarn("error reading %s", filename);
	(void) fclose(fp);
	return(1);
    }
    return(0);
}

flush_lines(filename, fd)
char *filename;
int fd;
{
    FILE *fp;
    char buf[BUFSIZ];
    int status;
    int len;
    char *ptr;

    if ((fp = fopen(filename, "r")) == NULL) {
	ewarn("reopen %s", filename);
	return(1);
    }
    ptr = buf + targetlen;
    while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
	if (*buf == *target && bcmp(buf, target, targetlen) == 0 &&
	    (*ptr == ' ' || *ptr == '\t')) {
	    continue;
	}
	len = strlen(buf);
	if ((status = write(fd, buf, len)) != len) {
	    ewarn("write error (%d, not %d)", status, len);
	    (void) fclose(fp);
	    return(1);
	}
    }
    if (ferror(fp) || fclose(fp) == EOF) {
	ewarn("error reading %s", filename);
	(void) fclose(fp);
	return(1);
    }
    return(0);
}

create_old_log_db(fromlogtmp)
char *fromlogtmp;
{
    int fd;
    int status;

    for (;;) {
	if ((fd = open(fromlogtmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", fromlogtmp);
	else if (flush_lines(fromlogdb, fd) == 0) {
	    if (close(fd) < 0)
		ewarn("close %s", fromlogtmp);
	    else
		return(0);
	}
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(fromlogtmp);
    }
}

create_new_log_db(tologtmp)
char *tologtmp;
{
    FILE *fp;
    char buf[BUFSIZ];
    int status;
    int fd;
    int len;
    int bufsiz;
    char *ptr;
    int errs;

    for (;;) {
	fp = NULL;
	if ((fd = open(tologtmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", tologtmp);
	else if (flush_lines(tologdb, fd) == 0) {
	    if ((fp = fopen(logfile, "r")) == NULL)
		ewarn("reopen %s", logfile);
	    else {
		(void) strcpy(buf, target);
		ptr = buf + targetlen;
		*ptr++ = ' ';
		bufsiz = sizeof(buf) - targetlen - 2;
		errs = FALSE;
		while (fgets(ptr, bufsiz, fp) != NULL) {
		    len = strlen(ptr) + targetlen + 1;
		    if ((status = write(fd, buf, len)) != len) {
			ewarn("write error (%d, not %d)", status, len);
			errs = TRUE;
			break;
		    }
		}
		if (ferror(fp) || fclose(fp) == EOF)
		    ewarn("error reading %s", logfile);
		else if (close(fd) < 0)
		    ewarn("close %s", tologtmp);
		else if (!errs)
		    return(0);
	    }
	}
	if (fp != NULL)
	    (void) fclose(fp);
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(tologtmp);
    }
}

create_old_who_db(fromwhotmp)
char *fromwhotmp;
{
    int fd;
    int status;

    for (;;) {
	if ((fd = open(fromwhotmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", fromwhotmp);
	else if (flush_lines(fromwhodb, fd) == 0) {
	    if (close(fd) < 0)
		ewarn("close %s", fromwhotmp);
	    else
		return(0);
	}
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(fromwhotmp);
    }
}

create_new_who_db(towhotmp)
char *towhotmp;
{
    struct tm *tm;
    time_t now;
    char datebuf[64];
    char *ptr, buf[BUFSIZ];
    int status;
    int fd;

    for (;;) {
	if ((fd = open(towhotmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", towhotmp);
	else if (flush_lines(towhodb, fd) == 0) {
	    now = time((time_t *)0);
	    tm = localtime(&now);
	    fdate(datebuf, "%3Month %02day %02hour:%02min:%02sec %year", tm);
	    ptr = concat(buf, sizeof(buf),
			  target, "\t", user, "\t", datebuf, "\n", NULL);
	    if ((status = write(fd, buf, ptr-buf)) != ptr-buf)
		ewarn("write error (%d, not %d)", status, ptr-buf);
	    else if (close(fd) < 0)
		ewarn("close %s", towhotmp);
	    else
		return(0);
	}
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(towhotmp);
    }
}

create_new_target(targettmp)
char *targettmp;
{
    int sfd;
    int tfd;
    int status;

    diag("%s: copying from %s", target, fullsource);
    for (;;) {
	for (;;) {
	    if ((sfd = open(fullsource, O_RDONLY, 0)) < 0)
		ewarn("reopen %s", fullsource);
	    else if ((tfd = open(targettmp, O_RDWR|O_CREAT|O_EXCL, 0600)) < 0)
		ewarn("reopen %s", targettmp);
	    else if (filecopy(sfd, tfd) < 0)
		ewarn("filecopy %s to %s failed", fullsource, targettmp);
	    else if (close(sfd) < 0)
		ewarn("close %s", fullsource);
	    else
		break;
	    (void) close(sfd);
	    (void) close(tfd);
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		return(status);
	    }
	    (void) unlink(targettmp);
	}
	while (stripfile(target, tfd, targettmp) != 0) {
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		if (status < 0) {
		    (void) close(tfd);
		    return(status);
		}
		break;
	    }
	}
	diag("%s: owner %s, group %s, mode %#o", target, owner, group, mode);
	while (fchown(tfd, uid, gid) < 0 && check) {
	    ewarn("chown %s", fulltarget);
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		(void) close(tfd);
		return(status);
	    }
	}
	while (fchmod(tfd, (int) mode) < 0) {
	    ewarn("chmod %s", fulltarget);
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		if (status < 0) {
		    (void) close(tfd);
		    return(status);
		}
		break;
	    }
	}
	while (utimes(targettmp, source_tv) < 0) {
	    ewarn("utimes %s", fulltarget);
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		if (status < 0) {
		    (void) close(tfd);
		    return(status);
		}
		break;
	    }
	}
	if (close(tfd) < 0)
	    ewarn("close %s", fulltarget);
	else
	    return(0);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    (void) close(tfd);
	    return(status);
	}
    }
}

stripfile(file, fd, tmpfile)
char *file; char *tmpfile;
register int fd;
{
#if	! multimax
    long size;
    struct exec head;
    int pagesize = getpagesize();

    if (!strip)
	return(0);
    (void) lseek(fd, (long)0, L_SET);
    if (read(fd, (char *)&head, sizeof(head)) != sizeof(head))
	return(0);
    if (N_BADMAG(head))
	return(0);
    if ((head.a_syms == 0) && (head.a_trsize == 0) && (head.a_drsize ==0)) {
	diag("%s: already stripped", file);
	return(0);
    }
    size = (long)head.a_text + head.a_data;
    head.a_syms = head.a_trsize = head.a_drsize = 0;
    if (head.a_magic == ZMAGIC)
	size += pagesize - sizeof(head);
    diag("%s: stripping", file);
    if (ftruncate(fd, size + sizeof(head)) < 0) {
	ewarn("ftruncate %s", file);
	return(1);
    }
    (void) lseek(fd, (long)0, L_SET);
    (void) write(fd, (char *)&head, sizeof(head));

#else	! multimax
#include <sgs.h>	/* For ISMAGIC */
    
    int pid, status, w;
    struct {
	struct filehdr fhdr;
	struct aouthdr ahdr;
    } hdrs;

    if (!strip)
	return(0);
    (void) lseek(fd, (long)0, L_SET);
    if (read(fd, (char *)&hdrs, sizeof(hdrs)) != sizeof(hdrs))
	return(0);
    if (! ISMAGIC(hdrs.fhdr.f_magic))
	return(0);
    if ((hdrs.fhdr.f_flags & F_RELFLG) == 0)
	return 0;	/* Relocation information present in file */
    if (hdrs.ahdr.magic != AOUT1MAGIC && hdrs.ahdr.magic != AOUT2MAGIC &&
	hdrs.ahdr.magic != PAGEMAGIC)
	return 0;
    if (hdrs.fhdr.f_symptr == 0) {
	diag("%s: already stripped", file);
	return(0);
    }

    diag("%s: stripping", file);
    if ((pid = vfork()) == 0) {
	execl("/bin/strip", "strip", tmpfile, 0);
	_exit(127);
    }
    while ((w = wait(&status)) != pid && w != -1) {}
#endif	! multimax
    return(0);
}

install_old_dbs(fromlogtmp, fromwhotmp)
char *fromlogtmp, *fromwhotmp;
{
    int status;

    while (rename(fromlogtmp, fromlogdb) < 0) {
	ewarn("rename %s to %s failed", fromlogtmp, fromlogdb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    while (rename(fromwhotmp, fromwhodb) < 0) {
	ewarn("rename %s to %s failed", fromwhotmp, fromwhodb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    return(0);
}

install_new_dbs(tologtmp, towhotmp)
char *tologtmp, *towhotmp;
{
    int status;

    while (rename(tologtmp, tologdb) < 0) {
	ewarn("rename %s to %s failed", tologtmp, tologdb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    while (rename(towhotmp, towhodb) < 0) {
	ewarn("rename %s to %s failed", towhotmp, towhodb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    return(0);
}

install_targets(targettmp)
char *targettmp;
{
    int i;
    int status;
    char temp[MAXPATHLEN];

    while (rename(targettmp, fulltarget) < 0) {
	ewarn("rename %s to %s failed", targettmp, fulltarget);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), tobase, links[i], NULL);
	for (;;) {
	    diag("%s: linking to %s", fulltarget, temp);
	    (void) unlink(temp);
	    if (link(fulltarget, temp) == 0)
		break;
	    ewarn("link %s to %s failed", fulltarget, temp);
	    if ((status = getarc()) != 0) {
		diag("%s: incomplete installation", fulltarget);
		if (status < 0)
		    return(status);
		break;
	    }
	}
    }
    return(0);
}

remove_old_sources()
{
    int i;
    char temp[MAXPATHLEN];

    /* remove source release targets */
    if (unlink(fullsource) == 0)
	diag("%s: being removed", fullsource);
    if (fromfile != NULL)
	return;
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), frombase, links[i], NULL);
	if (unlink(temp) == 0)
	    diag("%s: being removed", temp);
    }
}

make_post(stage_p)
struct stage *stage_p;
{
    char *ptr;
    char *av[16];
    struct stat statb;
    char subject[BUFSIZ];
    int i;
    int fd;
    FILE *fp;
    enum option command;
    char *defans;
    int child;
    int pid, omask;
    union wait w;

    if (!post || (stage_p->flags&STAGE_NOPOST) != 0)
	return;

    diag("\nPlease update the following message, if necessary.");
    diag("Your name and a default subject will be added automatically.");

    /*
     * default subject is tail of tobase and target
     */
    ptr = rindex(stage_p->roots[0], '/');
    if (ptr != NULL)
	ptr++;
    else
	ptr = stage_p->roots[0];
    (void) concat(subject, sizeof(subject), ptr, target, NULL);

    /*
     * copy log file to post template and stdout
     */
    command = TYPE;

    /*
     * parse commands
     */
    for (;;) {
	switch (command) {
	case EDIT:
	    (void) editor(logfile, "Entering editor.");
	    defans = "type";
	    break;
	case TYPE:
	    if ((fp = fopen(logfile, "r")) == NULL)
		break;
	    printf("\nSubject: %s\n", subject);
	    (void) ffilecopy(fp, stdout);
	    (void) fclose(fp);
	    defans = "post";
	    break;
	case SUBJECT:
	    (void) getstr("Subject", subject, subject);
	    defans = "type";
	    break;
	case POST:
	    defans = NULL;
	    break;
	case LOG:
	    return;
	}
	if (defans == NULL) {
	    if (stat(logfile, &statb) == 0 && statb.st_size > 0)
		break;
	    diag("You are not allowed to post an empty message");
	    command = EDIT;
	} else {
	    command = postopt[getstab("Edit, type, subject, post, log",
				      posttab, defans)];
	    clearerr(stdin);
	}
    }

    if ((fd = open(logfile, O_RDONLY, 0)) < 0) {
	ewarn("open %s", logfile);
	diag("Unable to make post");
	return;
    }
    i = 0;
    av[i++] = "post";
    av[i++] = "-";
    av[i++] = "-subject";
    av[i++] = subject;
    av[i++] = "unix-changelog";
    av[i++] = NULL;
    if ((child = vfork()) == -1) {
	ewarn("post vfork failed");
	diag("Unable to make post");
	return;
    }
    if (child == 0) {
	(void) setgid(getgid());
	(void) setuid(getuid());
	if (fd >= 0 && fd != 0) {
	    (void) dup2(fd, 0);
	    (void) close(fd);
	}
	execv("/usr/cs/bin/post", av);
	exit(0377);
    }
    (void) close(fd);
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
	ewarn("wait");
	diag("Unable to make post");
    } else if (WIFSIGNALED(w) || w.w_retcode == 0377) {
	warn("post killed");
	diag("Unable to make post");
    } else if (w.w_retcode != 0) {
	warn("post exited with status %d", w.w_retcode);
	diag("Unable to make post");
    }
    return;
}

/*
 * error routines
 */

/*VARARGS1*/
diag(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list vargs;

    (void) fflush(stdout);
    va_start(vargs);
    (void) vfprintf(stderr, fmt, vargs);
    va_end(vargs);
    (void) putc('\n', stderr);
}

/*VARARGS1*/
ewarn(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list vargs;
    int serrno = errno;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    va_start(vargs);
    (void) vfprintf(stderr, fmt, vargs);
    va_end(vargs);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    (void) fputs(errmsg(serrno), stderr);
    (void) putc('\n', stderr);
}

/*VARARGS1*/
efatal(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list vargs;
    int serrno = errno;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    va_start(vargs);
    (void) vfprintf(stderr, fmt, vargs);
    va_end(vargs);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    (void) fputs(errmsg(serrno), stderr);
    (void) putc('\n', stderr);
    exit(1);
}

/*VARARGS1*/
warn(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list vargs;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    va_start(vargs);
    (void) vfprintf(stderr, fmt, vargs);
    va_end(vargs);
    (void) putc('\n', stderr);
}

/*VARARGS1*/
fatal(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list vargs;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    va_start(vargs);
    (void) vfprintf(stderr, fmt, vargs);
    va_end(vargs);
    (void) putc('\n', stderr);
    exit(1);
}
