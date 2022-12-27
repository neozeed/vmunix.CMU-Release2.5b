/****************************************
 *
 * HISTORY
 * $Log:	service.c,v $
 * Revision 2.1  89/07/31  10:57:06  dimitris
 * 	Added code to deal with "log -pipe". 
 * 	[89/07/31  10:50:51  dimitris]
 * 
 * Revision 2.0  89/06/15  15:35:18  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include "service.h"
#if	SERVER
#include <signal.h>
#include <sys/wait.h>
#else	SERVER
#include "fakevice.h"
#include <pwd.h>
#endif	SERVER

typedef struct {
	unsigned long hostaddr;
	unsigned long uid;
	unsigned long g0;
	unsigned long g1;
} afs_cred_t;

#define DEBUG 0

extern int errno;
static char *getpipepass();

int setpag_function();
int setsysname_function();
int fs_function();
int log_function();
int tokens_function();
int unlog_function();

static (*services[])() = {
	setpag_function,
	setsysname_function,
	fs_function,
	log_function,
	tokens_function,
	unlog_function,
	0
};

int truepipe;

int (*get_service_by_number(number))()
int number;
{
	if (VALID_SERVICE(number)) {
		return services[number];
	}
	return 0;
}

#if	!SERVER
int get_service_number(name)
char *name;
{
	if      (! strcmp(name, "setpag"))	return SERVICE_SETPAG;
	else if (! strcmp(name, "setsysname"))	return SERVICE_SETSYSNAME;
	else if (! strcmp(name, "fs")) 		return SERVICE_FS;
	else if (! strcmp(name, "log")) 	return SERVICE_LOG;
	else if (! strcmp(name, "tokens")) 	return SERVICE_TOKENS;
	else if (! strcmp(name, "unlog")) 	return SERVICE_UNLOG;
	else return INVALID_SERVICE;
}
#endif	!SERVER

/*
 *  These are really transport functions,
 *  but they are conditionalized on SERVER
 *  so we put them here.
 */

#if	SERVER
recv_cred(afs_cred)
afs_cred_t *afs_cred;
{
	afs_cred->hostaddr = client_inet_addr();
	afs_cred->uid = recv_long();
	afs_cred->g0  = recv_long();
	afs_cred->g1  = recv_long();
}
#else	SERVER
send_cred()
{
	int gidset[NGROUPS];

	gidset[0] = 0;
	gidset[1] = 0;
	getgroups(NGROUPS, gidset);
	send_long(getuid());
	send_long(gidset[0]);
	send_long(gidset[1]);
}
#endif	SERVER

#if	SERVER
unsigned long pagCounter = 0;

setpag_function()
{
	send_long(0); /* success */
	send_long(pagCounter++);
}
#else	SERVER
setpag_function(argc, argv)
int argc;
char **argv;
{
	unsigned long pag;
	int status;

	status = recv_long();
	write(1, &status, sizeof(status));
	if (status == 0) {
		pag = recv_long();
		write(1, &pag, sizeof(pag));
	}
}
#endif	SERVER

#if	SERVER
setsysname_function()
{
	char name[64];
	afs_cred_t afs_cred;

	recv_cred(&afs_cred);
	recv_string(name, 64);
	if (pioctl_setclientcontext(&afs_cred) || pioctl_setsysname(name)) {
		send_long(errno);
	} else {
		send_long(0);
	}
}
#else	SERVER
setsysname_function(argc, argv)
int argc;
char **argv;
{
	int status;

	if (argc < 2) {
		status = EINVAL;
		write(1, &status, sizeof(status));
	}
	send_cred();
	send_string(argv[1]);
	status = recv_long();
	write(1, &status, sizeof(status));
}
#endif	SERVER

#if	SERVER
exec_function(path, argc, argv, input)
char *path;
int argc;
char **argv;
char *input;
{
	int p_in[2], p_out[2], p_err[2];
	int pid, waitrc;
	afs_cred_t afs_cred;
	union wait status;

	recv_cred(&afs_cred);
	if (pioctl_setclientcontext(&afs_cred)) {
		fatal("pioctl_setclientcontext");
	}

#if 1
	{ FILE *logfile;
	  char **targv;

		logfile = fopen("/tmp/pag2.log", "a");
		if (logfile == NULL) {
			return;
		}
		targv=argv;
		while (*targv)
		   fprintf(logfile,"*argv %s\n",*(targv++));
		fprintf(logfile,"done \n");
		fclose(logfile);
	}
#endif 1

 	if (pipe(p_out) < 0 || pipe(p_err) < 0) {
		fatal("pipe");
	}
	if (input && pipe(p_in) < 0) {
		fatal("pipe");
	}
	pid = fork();
	if (pid < 0) {
		fatal("fork");
	} else if (pid == 0) {
		close_comm_socket();
		if (input) {
			close(0); dup2(p_in[0], 0);
			close(p_in[0]); close(p_in[1]);
		}
		close(1); dup2(p_out[1], 1); close(p_out[1]);
		close(2); dup2(p_err[1], 2); close(p_err[1]);
		execv(path, argv);
		fatal("execv");
	} else {
		if (input) {
			signal(SIGPIPE, SIG_IGN);
			if (write(p_in[1], input, strlen(input)) < 0) {
				fatal("write of input");
			}
			close(p_in[0]);
			close(p_in[1]);
		}
		close(p_out[1]);
		close(p_err[1]);
		send_stream(p_out[0], p_err[0]);
		while ((waitrc = wait(&status)) != pid) {
			if (waitrc == -1) {
				exit(1);
			}
		}
	}
}
#else	SERVER
exec_function()
{
	send_cred();
	if (truepipe == 0)
		recv_stream(1, 2);
}
#endif	SERVER

#if	SERVER
fs_function()
{
	int argc;
	char **argv;

	recv_args(&argc, &argv);
	exec_function("/usr/vice/bin/fs", argc, argv, 0);
}
#else	SERVER
fs_function(argc, argv)
int argc;
char **argv;
{
	if (argc > 2) {
		argv[2] = locate_in_afs(argv[2]);
	}
	send_args(argc, argv);
	exec_function();
}
#endif	SERVER

#if	SERVER
log_function()
{
	int argc, j;
	char **_argv;
	char *argv[64];
	char passwd[128];

	recv_string(passwd, 128);
	recv_args(&argc, &_argv);
	truepipe = 0;
	for (j = 0; j < 60 && _argv[j]; j++) {
		argv[j] = _argv[j];
		if (strcmp(argv[j],"-pipe") == 0)
			truepipe =1;
	}
	if (truepipe == 0) {
		argv[j]   = "-pipe";
		argv[j+1] = 0;
		argc++; 
	}
	exec_function("/usr/vice/bin/log", argc, argv, passwd);
}
#else	SERVER
log_function(argc, argv)
int argc;
char **argv;
{
	char *passwd, *getpass() ;

        if (argc < 2) {
        
	  char *targv[3];
          struct passwd *tpw;

	  tpw = getpwuid(getuid());
          if (tpw == 0) {
                fprintf(stderr, "Can't figure out your name from your uid \n");
                fprintf(stderr, "Try providing the user name.\n");
                fprintf(stderr, "Usage: log [[-x] user [password] [-c cellname]]\n\n");
	        exit (1);
	  }
	  targv[0] = *argv;
	  targv[1] = (char *) malloc (9);
	  strcpy(targv[1],tpw->pw_name);
	  argv = targv ;
	  argc++;
	}
	{	int i;
	truepipe = 0;
        for (i=0;i<argc;i++)
           if (strcmp("-pipe",argv[i]) == 0 )
		truepipe = 1;
	}
	  if (truepipe)
		passwd = getpipepass();
	  else
		passwd = getpass("Password: ");

		
	send_string(passwd);
#if DEBUG
	{	int i;
        for (i=0;i<argc;i++)
           printf("argv[%d] = %s \n",i,argv[i]);
	}
#endif DEBUG
	send_args(argc, argv);
	exec_function();
}
#endif	SERVER

#if	SERVER
tokens_function()
{
	int argc;
	char **argv;

	recv_args(&argc, &argv);
	exec_function("/usr/vice/bin/tokens", argc, argv, 0);
}
#else	SERVER
tokens_function(argc, argv)
{
	send_args(argc, argv);
	exec_function();
}
#endif	SERVER

#if	SERVER
unlog_function()
{
	int argc;
	char **argv;

	recv_args(&argc, &argv);
	exec_function("/usr/vice/bin/unlog", argc, argv, 0);
}
#else	SERVER
unlog_function(argc, argv)
{
	send_args(argc, argv);
	exec_function();
}
#endif	SERVER

#ifdef	SERVER
log_usage(addr)
long addr;
{
	FILE *logfile;
	char *timestring, *ctime();
	long t;

	logfile = fopen("/usr/tmp/pagd.log", "a");
	if (logfile == NULL) {
		return;
	}
	time(&t);
	timestring = ctime(&t);
	timestring[24]=0;
	fprintf(logfile, "%s    %u.%u.%u.%u\n",
		timestring,
		(addr >> 24) & 0xff,
		(addr >> 16) & 0xff,
		(addr >>  8) & 0xff,
		(addr      ) & 0xff);
	fclose(logfile);
}
#endif	SERVER

static char gpbuf[9];
static char *getpipepass() {
    /* read a password from stdin, stop on \n or eof */
    register int i, tc;
    bzero(gpbuf, sizeof(gpbuf));
    for(i=0;i<8;i++) {
        tc = fgetc(stdin);
        if (tc == '\n' || tc == EOF) break;
        gpbuf[i] = tc;
    }
    return gpbuf;
}
