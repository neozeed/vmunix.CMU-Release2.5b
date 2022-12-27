/*
 *  authcover - authentication cover program
 *
 **********************************************************************
 * HISTORY
 * $Log:	authcover.c,v $
 * Revision 2.5  89/08/22  14:29:11  gm0w
 * 	Changed execv after setting tokens to runv followed by delete
 * 	local tokens.
 * 	[89/08/22            gm0w]
 * 
 * Revision 2.4  89/02/17  15:45:04  gm0w
 * 	Made okPath path buffer static.
 * 	[89/02/17            gm0w]
 * 
 * Revision 2.3  89/01/28  23:07:20  gm0w
 * 	Generalized bcsauth into authcover.
 * 	[89/01/28  22:58:55  gm0w]
 * 
 * Revision 2.2  89/01/24  23:38:23  mja
 * 	Created.
 * 
 **********************************************************************
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/table.h>
#include <sys/ttyloc.h>
#include <sys/errno.h>
#include <sys/file.h>

#include <libc.h>
#include <c.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <acc.h>
#include <access.h>

/*
 *  We include these directly to avoid putting /usr/andrew along the CPATH with
 *  -I since such directories appear first and would supercede local copies.
 */
typedef bool bool_t;
#include "/usr/andrew/include/afs/cellconfig.h"
#include "/usr/andrew/include/afs/comauth.h"


#ifdef	DEBUG
#include <varargs.h>

/*VARARGS1*/
diag(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list vargs;

    va_start(vargs);
    (void) vfprintf(stderr, fmt, vargs);
    va_end(vargs);
}

/*
 *  Debugging flags for trace output
 */
int debug;			/* set by -debug argument: */
#define	DBG_SAVE	001	/* -trace save file operations */
#define	DBG_AUTH	002	/* -trace authentication decisions */
#define	DBG_TOKEN	004	/* -trace token manipulations */
#define	DBG_UID		010	/* -trace UID manipulations */



#define	dprintf(flags, args) if (debug&(flags)) diag args
#else	DEBUG
/*
 *  Without debugging on, simply expand these to nothing.
 */
#define dprintf(flags, args)
#endif	DEBUG



/*
 *  AFS authentication information record
 */
typedef struct auth {
    SecretToken	sToken;		/* secret token component */
    ClearToken  cToken;		/* clear token component */
    char cellID[MAXCELLCHARS];	/* cell name */
} auth_t;



/*
 *  Other global data
 */
char  *argProgram = "<missing>";/* program name from argv[0] */
char  *envTESTDIR;		/* AUTHCOVER_TESTDIR environment variable */
char  *envAUTHUSER;		/* AUTHCOVER_USER environment variable */
auth_t pagMyToken;		/* my current AFS token */
dev_t  u_ttyd;			/* my controlling terminal device number */
bool   usestdin;		/* use stdin, not /dev/tty */

#define	SAVEFORMAT 0x1


/*
 *  Saved authentication information record (stored in the file system)
 */

typedef struct save {
   int    version;		/* file format version */
   auth_t token;		/* AFS token information */
   uid_t  uid;			/* local user ID */
   gid_t  gid;			/* local group ID */
} save_t;



/*
 *  External forward declarations
 */

extern char *fgetpass();
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern char *rindex();
extern char *index();
extern uid_t geteuid();
extern gid_t getegid();
extern gid_t getgid();
extern uid_t getuid();
extern time_t time();

extern int errno;



/*
 *  Internal forward declarations
 */

int   authAccess();
int   authGetToken();
void  authSetID();
int   authSetToken();
void  authSwapUID();
int   authTestAccess();
int   authVBase();
int   code();
void  memAlloc();
char *okPath();
int   pagGetToken();
int   pagCreate();
int   pagSetToken();
char *saveFileName();
void  saveObtain();
int   saveRead();
int   saveWrite();



/*
 *  main - main program
 *
 *  Save the program name for error messages and parse the debug option if
 *  present.  Swap the effective and real user ID's so that we run by default
 *  with any privileges disabled.  Validate the first argument as an acceptable
 *  program name.  Extract the TESTDIR and AUTHUSER variables, the controlling
 *  device and current AFS tokens from the environment, aborting if they cannot
 *  be found.
 *
 *  Obtain the necessary authentication information (possibly stored by a prior
 *  invocation), enable it, and run the authorized program with any requested
 *  arguments.
 */

/* ARGSUSED */
main(argc, argv)
    int argc;
    char *argv[];
{
    static char TESTDIR[] = "AUTHCOVER_TESTDIR";
    static char AUTHUSER[] = "AUTHCOVER_USER";
    save_t save;
    char *path;
    int error;

#ifdef	DEBUG
    if ((path = getenv("AUTHCOVER_DEBUG")) != NULL)
    {
	debug = atoo(path);
    }
#endif	DEBUG

    authSwapUID();

    if (*argv)
	argProgram = *argv++;

    usestdin = false;

    while (argv[0] != 0)
    {
#ifdef	DEBUG
	if (strcmp(argv[0], "-debug") == 0 && argv[1] != 0)
	{
	    debug = atoo(argv[1]);
	    argv += 2;
	    continue;
	}
#endif	DEBUG
	if (strcmp(argv[0], "-") == 0)
	{
	    usestdin = true;
	    argv++;
	    continue;
	}
	break;
    }

    path = okPath(argv[0]);
    if (path == NULL)
	quit(-1, "%s: %s not authorized\n", argProgram,
	     (argv[0])?argv[0]:"<no program>");

    envTESTDIR = getenv(TESTDIR);
    if (envTESTDIR == NULL)
	quit(-3, "%s: no %s variable set\n", argProgram, TESTDIR);

    if (authAccess(&save) == 0)
    {
	authSwapUID();	/* back to effective ROOT first */
	authSetID((int)getuid(),(int)getuid(),(int)getgid(),(int)getgid());

	execv(path, argv);
	quit(-2, "%s: unable to invoke %s ...: %s\n", argProgram,
	     path, errmsg(-1));
    }

    envAUTHUSER = getenv(AUTHUSER);
    if (envAUTHUSER == NULL)
	quit(-4, "%s: no %s variable set\n", argProgram, AUTHUSER);
    if (table(TBL_U_TTYD, 0, (char *)&u_ttyd, 1, sizeof(u_ttyd)) != 1)
	quit(-5, "%s: no controlling terminal\n", argProgram);
    error = pagGetToken(&pagMyToken);
    if (error)
	quit(-6, "%s: must authenticate yourself (#%d) first\n",
		 argProgram, getuid());

    if (strcmp(argv[0], "unlog") == 0) {
       if (unlink(saveFileName()) < 0 && errno != ENOENT)
	   fprintf(stderr,"[ unable to unlink %s: %s ]\n",
		   saveFileName(),errmsg(errno));
       /* really want to unlog here */
       exit(0);
   }

    saveObtain(&save);

    authSwapUID();	/* back to effective ROOT first */
    authSetID((int)save.uid,(int)save.uid, (int)save.gid,(int)save.gid);

    error = runv(path, argv);

    U_DeleteLocalTokens();

    exit(error);
}

/*
 *  authAccess - check for write access to TESTDIR
 *
 *  sa = authentication information (unused)
 *
 *  Temporarily swap user ID's (since access(2) uses the real user ID and we
 *  normally keep the privilegd ID there) and check the TESTDIR directory for
 *  write access using the current "real" user ID.
 *
 *  Return: 0 if write access is granted, otherwise an approriate error code;
 *
 *  other: as provided by access(2).
 */

int
authAccess(sa)
    save_t *sa;
{
    int error;

#ifdef lint
    if (sa)
	;
#endif

    authSwapUID();
    if (access(envTESTDIR, W_OK) < 0)
	error = errno;
    else
	error = 0;
    authSwapUID();
    dprintf(error?DBG_AUTH:0, ("authAccess=>%s\n", errmsg(error)));
    return(error);
}



/*
 *  authAsUser - invoke a procedure under new user ID
 *
 *  sa   = authentication information
 *  func = procedure to invoke
 *
 *  Temporarily swap user ID's (to obtain privileges), change the real user ID
 *  as specified by the authentication record, swap ID's back to make this the
 *  effective ID, invoke the specified procedure, and restore the original ID's
 *  on return.  The real user ID is changed in order to retain the privileges
 *  provided by the effective ID (usually root) but the convention of keeping
 *  the privileged ID as the real user is maintained within the invoked
 *  procedure.
 *
 *  Return: 0 if the procedure is successfully invoked, otherwise
 *  its error code.
 */

authAsUser(sa, func)
    save_t *sa;
    int (*func)();
{
    uid_t myruid;
    int error;

    myruid = geteuid();

    authSwapUID();
    authSetID((int)sa->uid,-1,-1,-1);
    authSwapUID();
    error = (*func)(sa);
    authSwapUID();
    authSetID((int)myruid,-1,-1,-1);
    authSwapUID();

    dprintf(error?DBG_AUTH:0, ("authAsUser=>%s\n", errmsg(error)));
    return(error);
}




/*
 *  authGetToken - retrieve AFS token for current effective user ID
 *
 *  sa = authentication information
 *
 *  Retrieve the token for the current effective user and PAG and store them in
 *  the supplied buffer.
 *
 *  Return: 0 on success, otherwise an appropriate error code.
 */

authGetToken(sa)
    save_t *sa;
{
    int error;

    error = pagGetToken(&sa->token);
    dprintf(error?DBG_AUTH:0, ("authGetToken=>%s\n", errmsg(error)));
    return(error);
}



/*
 *  authSetID - set real/effective user /group ID's
 *
 *  ruid = desired real user ID
 *  euid = desired effective user ID
 *  rgid = desired real group ID
 *  egid = desired effective group ID
 *
 *  Set the ID's as requested.  A value of -1 indicates that the
 *  current value should remain unchanged.
 */

void
authSetID(ruid, euid, rgid, egid)
    int ruid;
    int euid;
    int egid;
    int rgid;
{
    if (setregid(rgid, egid) < 0)
	fprintf(stderr,"[ could not set E%d,R%d gid: %s ]\n",
		egid, rgid, errmsg(-1));
    if (setreuid(ruid, euid) < 0)
	fprintf(stderr,"[ could not set E%D,R%d uid: %s ]\n",
		euid, ruid, errmsg(-1));
    dprintf(DBG_UID, ("authSetID:E%d,%d;R%d,%d\n",
	     geteuid(),getegid(),getuid(),getgid()));
}



/*
 *  authSetToken - store AFS token for current effective user ID
 *
 *  sa = authentication information
 *
 *  Store the supplied token for the current effective user and PAG.
 *
 *  Return: 0 on success, otherwise an appropriate error code.
 */

authSetToken(sa)
    save_t *sa;
{
    int error;

    error = pagSetToken(&sa->token);
    dprintf(error?DBG_AUTH:0, ("authSetToken=>%s\n", errmsg(error)));
    return(error);
}



/*
 *  authSwapUID - exchange effective and real user ID's
 */

void
authSwapUID()
{
    authSetID((int)geteuid(), (int)getuid(), -1, -1);
}


/*
 *  authTestAccess - test access to TESTDIR for target user
 *
 *  sa      = authentication information
 *  needpag = flag to force creation of new PAG
 *
 *  Create a new PAG if requested, set the specified AFS authentication
 *  information and real user ID, and test that this ID has write access
 *  to the TESTDIR directory.
 *
 *  Return: 0 on success, otherwise an appropriate error code.
 */

authTestAccess(sa, needpag)
    save_t *sa;
    bool needpag;
{
    int error = 0;

    if (needpag)
    {
	error = pagCreate();
	if (error)
	    fprintf(stderr, "[ problem creating new PAG: %s ]\n", errmsg(error));
    }
    if (!error)
    {
	error = authAsUser(sa, authSetToken);
	if (error)
	    fprintf(stderr, "[ failure setting token for user #%d: %s ]\n",
			sa->uid, errmsg(error));
    }
    if (!error)
    {
	error = authAsUser(sa, authAccess);
	if (error)
	    fprintf(stderr, "[ no write access to %s: %s ]\n", envTESTDIR, errmsg(error));
    }
    return(error);
}



/*
 *  authAsk - prompt for authentication from the user
 *
 *  sa = record to receive authentication information
 *
 *  Validate the user specified in the AUTHUSER environment variable via
 *  oklogin() (using code mostly lifted directly from su(1).  If successful,
 *  store its user/group ID along with the token set for it by log(1) into the
 *  authentication record.
 *
 *  Return: 0 on success, otherwise an appropriate error code.
 */

authAsk(sa)
    save_t *sa;
{
    struct passwd *pwd;
    struct group *grp;
    struct account *acc;
    char *pswd;
    int val;
    struct ttyloc tlc;
    char *group, *account;
    int *groups;
    int error = EACCES;
    int code;
    FILE *fp;

    pwd = getpwnam(envAUTHUSER);
    if (pwd == NULL)
	quit(-8, "%s: user %s unknown\n", argProgram, envAUTHUSER);

    authSwapUID();

    if (u_ttyd < 0 || table(TBL_TTYLOC, u_ttyd, (char *)&tlc, 1, sizeof(tlc)) < 0) {
	tlc.tlc_hostid = TLC_UNKHOST;
	tlc.tlc_ttyid = TLC_UNKTTY;
    }

    fprintf(stderr, "[ please authenticate as '%s' ]\n", envAUTHUSER);
    if (usestdin || (fp = fdopen(open("/dev/tty", O_RDWR, 0), "r")) == NULL)
	fp = stdin;
    pswd = fgetpass("Password:", fp);

    code = okaccess(pwd->pw_name, ACCESS_TYPE_SU, getuid(), u_ttyd, tlc);
    code = (code == 1) ? ACCESS_CODE_OK : ACCESS_CODE_DENIED;

    group = NULL;
    account = NULL;
    grp = NULL;
    acc = NULL;
    val = oklogin(pwd->pw_name, group, &account, pswd,
		  &pwd, &grp, &acc, &groups);
    if (val != ACCESS_CODE_OK)
	    code = val;
    switch (code) {
    case ACCESS_CODE_OK:
	    break;
    default:
    case ACCESS_CODE_DENIED:
    case ACCESS_CODE_NOUSER:
    case ACCESS_CODE_BADPASSWORD:
	fprintf(stderr,"Sorry\n");
	break;
    case ACCESS_CODE_ACCEXPIRED:
	fprintf(stderr,"Sorry, your %s account has expired",
		foldup(account, account));
	break;
    case ACCESS_CODE_GRPEXPIRED:
	fprintf(stderr,"Sorry, your %s group account has expired",
		foldup(grp->gr_name, grp->gr_name));
	break;
    case ACCESS_CODE_ACCNOTVALID:
	if (account)
	    fprintf(stderr,"Sorry, %s is not a valid account",
		    foldup(account, account));
	else
	    fprintf(stderr,"Sorry, %s is not a valid account group",
		    foldup(grp->gr_name, grp->gr_name));
	break;
    case ACCESS_CODE_MANYDEFACC:
	fprintf(stderr,"Your accounting file entries are inconsistant.\n");
	fprintf(stderr,"Please report this problem to Gripe");
	break;
    case ACCESS_CODE_NOACCFORGRP:
	fprintf(stderr,"Sorry, your %s group does not have an account",
		foldup(grp->gr_name, grp->gr_name));
	break;
    case ACCESS_CODE_NOGRPFORACC:
	fprintf(stderr,"Sorry, your %s account does not have a group",
		foldup(account, account));
	break;
    case ACCESS_CODE_NOGRPDEFACC:
	fprintf(stderr,"Sorry, your default account does not have a group");
	break;
    case ACCESS_CODE_NOTGRPMEMB:
	fprintf(stderr,"Sorry, you are not a member of the %s group",
		foldup(group, group));
	break;
    case ACCESS_CODE_NOTDEFMEMB:
	fprintf(stderr,"Sorry, you are not a member of your default login\n");
	fprintf(stderr,"group.  You must specify an explicit group to login");
	break;
    }
    logaccess(pwd->pw_name, ACCESS_TYPE_SU, code, getuid(), u_ttyd, tlc);
    authSwapUID();

    if (code == ACCESS_CODE_OK)
    {
	extern afs_log_errcode;

	if (afs_log_errcode == 0)
	{
	    sa->uid = pwd->pw_uid;
	    sa->gid = pwd->pw_gid;
	    error = authAsUser(sa, authGetToken);
	    if (!error)
	    {
		endpwent();
	    }
	}
	else
	    dprintf(DBG_AUTH, ("authAsk:log error=%d\n", afs_log_errcode));
    }

    dprintf(error?DBG_AUTH:0, ("authAsk=>%s\n", errmsg(error)));
    return(error);
}



/*
 *  code = encode/decode buffer
 *
 *  clear     = clear text buffer (obtained from malloc())
 *  clearlenp = clear text buffer length (by ref)
 *  key       = key buffer
 *  keylen    = key length
 *
 *  The clear text buffer is encrypted/decrypted with the specified key as
 *  indicated.  The size of the buffer may be altered during this
 *  process and the caller is responsible for checking the updated length
 *  parameter.  For now, the encryption used is a simple XOR with a checksum
 *  byte appended at the end to help detect decrypt failures and improper
 *  calling usage.
 *  
 *  Return: 0 on success, otherwise an appropriate error code;
 *
 *  ENOTCONN: clear text could not be encrypted/decrypted.
 */

int
code(clear, clearlenp, key, keylen, dodecode)
    char  *clear;
    u_int *clearlenp;
    char  *key;
    u_int keylen;
    bool  dodecode;
{
    register int i,k;
    register u_int l;
    char cksm = 0;
    int error;

    if (dodecode)
	l = --(*clearlenp);
    else
	l = (*clearlenp)++;
    error = 0;

    memAlloc(&clear, l);
    k = 0;
    for (i=0; i<l; i++)
    {
	if (!dodecode)
	    cksm += clear[i];
	clear[i] ^= key[k];
	if (dodecode)
	    cksm += clear[i];
	if (++k >= keylen)
	    k = 0;
    }

    if (dodecode)
    {
	error = ((clear[l] ^ key[k]) != cksm)?ENOTCONN:0;
    }
    else
    {
	error = 0;
	clear[l] = cksm ^ key[k];
    }

    return(error);
}



/*
 *  memAlloc - [re]allocate memory with exit on error
 *
 *  mempp  = memory pointer address
 *  nbytes = number of bytes to allocate
 *
 *  If the supplied memory pointer is NULL, allocate a new block of memory of
 *  the indicated size and record its address in the pointer.  If the supplied
 *  memory pointer was non-NULL, reallocate the existing block of memory to the
 *  indicated size and reord the address of the (possibly) new block in the
 *  pointer.  If sufficient new memory cannot be allocated, quit() with a fatal
 *  error.
 *
 *  Return:  with a potentially updated memory address.
 */

void
memAlloc(mempp, nbytes)
    register char **mempp;
    unsigned nbytes;
{
    *mempp = (*mempp)?realloc(*mempp, nbytes):malloc(nbytes);
    if (*mempp == NULL)
	quit(-9, "%s: malloc failure: %s\n", argProgram, errmsg(-1));
}



/*
 *  okPath - validate program name
 *
 *  Lookup the supplied program name in the validation table.
 *
 *  Return:  the absolute path associated with this program or NULL if the
 *  program is not found in the validation table.
 */

char *
okPath(prog)
   char *prog;
{
    static char buf[BUFSIZ];
    FILE *fp;
    char *ptr, *bp, *pr, *pa;
    bool matchpath;
    char *afsprefix;
    char *proglist;

    if (strcmp(prog, "unlog") == 0)
	return("/bin/true");
#ifdef DEBUG
    if (strcmp(prog, "tokens") == 0)
	return("/usr/vice/bin/tokens");
    if (strcmp(prog, "pj") == 0)
	return("/usr/cs/bin/pj");
    if (strcmp(prog, "oops") == 0)
	return("/usr/cs/bin/oops");
#endif
    afsprefix = pathof("authcover.afsprefix");
    if (afsprefix == NULL)
	afsprefix = "/afs/cs.cmu.edu/@sys/";
    proglist = pathof("authcover.proglist");
    if (proglist == NULL)
	proglist = "/usr/misc/.sdm/lib/authcover.list";
    if (*prog == '/')
    {
	if (strncmp(prog, afsprefix, strlen(afsprefix)) == 0)
	{
	    ptr = prog + strlen(afsprefix);
	    if ((ptr = index(ptr, '/')) == NULL)
		return(NULL);
	}
	else
	    ptr = prog;
	matchpath = true;
    }
    else
	matchpath = false;
    if ((fp = fopen(proglist, "r")) == NULL)
	quit(-10, "%s: %s fopen failure: %s\n",
	     argProgram, proglist, errmsg(-1));
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
	bp = buf;
	pr = nxtarg(&bp, "# \t\n");
	if (*pr == '\0')
	    continue;
	pa = nxtarg(&bp, "# \t\n");
	if (*pa != '/')
	    continue;
	if (strcmp(matchpath ? pa : pr, matchpath ? ptr : prog) != 0)
	    continue;
	return(matchpath ? prog : pa);
    }
    return(NULL);
}


/*
 *  pagCreate - create a new PAG
 *
 *  Create a new PAG, temporarily catching signals in case this system call is
 *  not defined to avoid a code dump (although the program will accomplish much
 *  in this case).
 *
 *  Return: 0 on success, or the appropriate error code.
 */

int
pagCreate()
{
    int (*sig)();
    int error;

    sig = signal(SIGSYS, SIG_IGN);
    errno = 0;
    setpag();
    error = errno;
    (void) signal(SIGSYS, sig);
    dprintf(DBG_TOKEN, ("pagCreate=>%s\n", errmsg(error)));
    return(error);
}



/*
 *  pagGetToken - retrieve the current token
 *
 *  t = token record
 *
 *  Store the token associated with the current effective user ID and PAG in
 *  the primary cell into the supplied record.
 *
 *  Return: 0 on success, or an appropriate error code;
 *
 *  ERANGE: no primary token exists.
 */

int
pagGetToken(t)
   auth_t *t;
{
    int	 isPrimary;
    int	 error;
    int  cellNum;

    for (cellNum=0;;cellNum++)
    {
	errno = 0;
	bzero((char *)t, sizeof(*t));
        error = U_CellGetLocalTokens(1, cellNum, &t->cToken, &t->sToken,
			 t->cellID, &isPrimary);
	if (error)
	{
	    error = errno?errno:ERANGE;
	    break;
	}
	dprintf(DBG_TOKEN, ("pagGetToken:#%d,cell '%s'\n",
	       t->cToken.ViceId, t->cellID));
        if (isPrimary)
	    break;
    }
    dprintf(error?DBG_TOKEN:0, ("pagGetToken=>%s\n",errmsg(error)));
    return(error);
}



/*
 *  pagSetToken - store the current token
 *
 *  t = token record
 *
 *  Set the specified token for the current effective user ID and PAG.
 *
 *  Return: 0 on success, or an appropriate error code;
 *
 *  ERANGE: no primary token exists.
 *  
 */

int
pagSetToken(t)
   auth_t *t;
{
    int error;

    errno = 0;
    error = U_CellSetLocalTokens(0, &t->cToken, &t->sToken, t->cellID, 1);
    dprintf(DBG_TOKEN, ("pagSetToken:#%d,cell '%s'\n",
	   t->cToken.ViceId, t->cellID));
    if (error)
	error = errno?errno:ERANGE;
    dprintf(error?DBG_TOKEN:0,("pagSetToken=>%s\n",errmsg(error)));
    return(error);
}



/*
 *  saveFileName -  construct expected save file name
 *
 *  Return: a pointer to the expected file name constructed (once) from the
 *  current program name, user ID, AUTHUSER environment variable, and
 *  controlling device number.
 */

char *
saveFileName()
{
    static char *name= NULL;

    if (name == NULL)
    {
	char *cp = rindex(argProgram, '/');

	if (cp != 0)
	     cp++;
	else
	     cp = argProgram;
        memAlloc(&name, 5+strlen(cp)+12+strlen(envAUTHUSER)+4+(u_int)1);
	(void) sprintf(name, "/tmp/%s%d%s%04x",
		       cp, geteuid(), envAUTHUSER,u_ttyd);
	dprintf(DBG_SAVE, ("saveFileName:%s\n", name));
    }
    return(name);
}



/*
 *  saveObtain - lookup/prompt for saved authentication information
 *
 *  sa = pointer to authentication record to receive it
 *
 *  Look for the information first in the save file.  If found, enable it and
 *  verify that it still grants access to the base directory.  If not found or
 *  there is no longer access, recompute this information by prompting the user
 *  and continue to reverify until things work.  Remember if new authentication
 *  information was provided to avoid creating a new PAG when oklogin() has
 *  already done this and to force an update of the save file once everything
 *  checks out.
 */

void
saveObtain(sa)
    save_t *sa;
{
    int error;
    bool saved = true;
    bool needpag = true;

    error = saveRead(sa);
    do {
	for (;;)
	{
	    if (!error)
		error = authTestAccess(sa, needpag);
	    if (error)
	    {
		if (unlink(saveFileName()) < 0 && errno != ENOENT)
		    fprintf(stderr,"[ unable to unlink %s: %s ]\n",
	            	    saveFileName(),errmsg(errno));
		saved = false;
		error = authAsk(sa);
		needpag = false;
	    }
	    else
		break;
	}
	if (!saved)
	{
	    error = saveWrite(sa);
	}
    } while (error);
}



/*
 *  saveRead - lookup saved authentication information
 *
 *  sa = pointer to authentication record to receive it
 *
 *  Read the entire contents of the epected file into a temporary buffer.
 *  Decrypt this buffer using the current user's AFS tokens as a minimal
 *  firewall against accidentally divulging this to other users with the same
 *  UID.  Also, verify that the saved format version matches the current format
 *  version.
 *
 *  Return:  0 if the lookup succeeded with the authentication information
 *  filled in to the supplied buffer, otherwise an appropriate error code;
 *
 *  ENOTCONN: the temporary buffer could not be decrypted,
 *  EIO: an unspecified error during the read operation,
 *  ERANGE: the file format is out of date or could not be decrypted,
 *  other: as determined by code(), fopen(), fstat(), or fread().
 */

saveRead(sa)
    save_t *sa;
{
    FILE *f;
    int error;
    struct stat st;

    f = fopen(saveFileName(),"r");
    if (f != NULL && fstat(fileno(f), &st) >= 0)
    {
	union {char *bytes; save_t *auth;} buff;
	u_int size = st.st_size;

	buff.bytes = NULL;
	memAlloc(&buff.bytes, size);
	if (fread(buff.bytes, (int)size, 1, f) == 1)
	{
	    error = code(buff.bytes, &size, (char *)&pagMyToken.sToken,
		         sizeof(pagMyToken.sToken), true);
	    if (!error)
	    {
		*sa = *(buff.auth);
		dprintf(DBG_SAVE, ("saveRead:V%d\n", sa->version));
		if (sa->version == SAVEFORMAT)
		    error = 0;
		else
	        {
		    error = ERANGE;
		    fprintf(stderr, "[ save format obsolete ]\n");
		}
	    }
	    else
	    {
		fprintf(stderr, "[ save decrypt failure ]\n");
	    }
	}
	else
	{
	    error = errno?errno:EIO;
	    fprintf(stderr, "[ save read error: %s ]\n", errmsg(error));
	}
	free(buff.bytes);
	(void) fclose(f);
    }
    else
	error = errno?errno:EIO;
    dprintf(error?DBG_SAVE:0, ("saveRead=>%s\n", errmsg(error)));
    return(error);
}



/*
 *  saveWrite - record saved authentication information
 *
 *  sa = pointer to authentication record to record
 *
 *  Save the current file format version in the authentication record.  Copy
 *  this information to a temporary buffer and encrypt it using the using the
 *  current user's AFS tokens.  Write the entire encrypted contents to the
 *  expected file name.  The file is created with access forbidden to all but
 *  the owner.
 *
 *  Return:  0 if the save succeeded, otherwise an appropriate error code;
 *
 *  ENOTCONN: the temporary buffer could not be encrypted,
 *  EIO: an unspecified error during the write operation,
 *  other: as determined by fopen(), or fwrite().
 */

saveWrite(sa)
    save_t *sa;
{
    FILE *f;
    int error;
    int savemask;
    int fd;

    savemask = umask(077);
    if (unlink(saveFileName()) < 0 && errno != ENOENT)
	fprintf(stderr,"[ unable to unlink %s: %s ]\n",
		saveFileName(),errmsg(errno));
    if ((fd = open(saveFileName(), O_WRONLY|O_CREAT|O_EXCL, 0600)) < 0)
    {
	error = errno?errno:EIO;
	fprintf(stderr, "[ save record failure: %s ]\n", errmsg(error));
    }
    else if ((f = fdopen(fd,"w")) != NULL)
    {
	union {char *bytes; save_t *auth;} buff;
	u_int len = sizeof(*sa);

	sa->version = SAVEFORMAT;

	buff.bytes = NULL;
	memAlloc(&buff.bytes, len);
	*(buff.auth) = *sa;
	dprintf(DBG_SAVE, ("saveWrite:V%d\n", sa->version));
	error = code(buff.bytes, &len, (char *)&pagMyToken.sToken,
				sizeof(pagMyToken.sToken), 0);
	if (!error)
	{
	    if (fwrite(buff.bytes, (int)len, 1, f) == 1)
		error = 0;
	    else
	    {
		error = errno?errno:EIO;
		fprintf(stderr, "[ save write failure: %s ]\n", errmsg(error));
	    }
	}
	else
	{
	    fprintf(stderr,"[ save encrypt failure ]\n");
	}
	if (fclose(f) == EOF && !error)
	{
	    error = errno?errno:EIO;
	    fprintf(stderr, "[ save close failure: %s ]\n",
			   errmsg(error));
	}
    }
    else
    {
	error = errno?errno:EIO;
	fprintf(stderr, "[ save record failure: %s ]\n", errmsg(error));
    }
    (void) umask(savemask);
    dprintf(error?DBG_SAVE:0, ("saveWrite=>%s\n", errmsg(error)));
    return(error);
}
