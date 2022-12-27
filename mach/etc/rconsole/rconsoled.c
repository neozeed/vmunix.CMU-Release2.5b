/*
 * rconsoled: Daemon program for logging the output of a remote console
 *	terminal and providing a chat service to that remote console
 *	terminal line, using a local tty line.
 *
 * Use: rconsoled [-a] name &
 *	where name is one of the remote machines listed in /etc/rconsoles.
 *	Any of the available aliases listed in /etc/rconsoles is acceptable.
 *	The -a flag causes rconsoled to require authorization before
 *	allowing a user to access the console.  Upon connecting to a
 *	client, rconsole sends an 'a' instead of '\n'.  If the next
 *	null-terminated string sent by the client is the password of
 *	the RCONSOLE_ID account, he is ok'ed, otherwise punted.
 *	The only way to terminate rconsoled is by killing it, or sending it
 *	the TERM_CHAR termination message when in chat mode ("rconsole -k
 *	name" is the usual way to send the termination message).
 *
 * History:
 *	14-Jan-86  Bill Bolosky (bolosky) at Carnegie-Mellon University.
 *		Changed to use INET sockets instead of Unix.  Added
 *		authorization (-a) stuff and deleted -f switch.
 *	11-Sep-85  James Wendorf (jww) at Carnegie-Mellon University.
 *		Modified QUIT_CHAR to avoid accidental termination.
 *	08-Oct-84  James Wendorf (jww) at Carnegie-Mellon University.
 *		Modified signal handling to only ignore SIGPIPE, and to clean
 *		up before terminating upon SIGHUP, SIGINT, SIGQUIT, and
 *		SIGTERM.  Fixed log file mode to always be 0664, and added
 *		more information to error messages if opening of log file or
 *		tty fails.  Modified to only send a '\n' upon initial
 *		acceptance of a new connection.  Added recognition of
 *		TERM_CHAR termination message when in chat mode.  Added time
 *		stamps to log file to indicate when rconsoled starts up and
 *		terminates.
 *	19-Sep-84  James Wendorf (jww) at Carnegie-Mellon University.
 *		Created.
 */

#include "rconsole.h"

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <pwd.h>

char *date();
char *ctime();
char *crypt();

char *name;			/* pgm name */
char log_file[100];		/* Name of log file */
char logline[2000];		/* buffer for tty line data */
int log;			/* File descriptor for log file */
long log_length;		/* Length of the log file */
int log_flag = 1;		/* log to log_file */
int tty;

int debug = 0;			/* local debugging */
int fake_conn = 0;		/* don't use daemon; Programmer is testing */
int a_flag = 0;			/* Command line -a flag. */
int baud;			/* Baud rate of tty */
char loginfo[100];		/* log in fo */
char *logname = RCONSOLE_ID;	/* whose passwd to check */

extern int errno;

main(argc, argv)
int argc;
char **argv;
{
char rcon_name[ALIAS_LENGTH];	/* Common name for remote machine */
char rcon_tty[25];		/* Path name of tty connected to rem. mach. */
int s;

	name = *argv;
	{register char *cp =name;
		while (*cp) if (*cp++ == '/') name = cp;
	}

	for ( argv++ ; --argc ; argv++ ) { register char *token = *argv;
		if (*token++ != '-' || !*token)
			break;
		else { register int flag;
			for ( ; flag = *token++ ; ) {
				switch (flag) {
				case 'a':
					a_flag = 1;
					break;
				case 'c':
					fake_conn = 1;
					break;
				case 'd':
					debug = 1;
					break;
				case 'l':
					strcpy(log_file, *++argv);
					if (--argc < 0) goto usage;
					break;
				case 'u':
					strcpy(logname, *++argv);
					if (--argc < 0) goto usage;
					break;
				default:
					goto usage;
				}
			}
		}
	}
	if (debug)
		printf("%s: main(); a_flag %d, fake_conn %d, debug %d, log %s, user %s\n",
			name, a_flag, fake_conn, debug,
			(int) log_file ? log_file : "NULL", logname);

	if (argc == 0) {
		printf("%s: main();  Machine to connect to not specified.\n", name);
		exit(1);
	}

	s = est_conn(*argv, rcon_name, rcon_tty, &baud);

	if (! *log_file) {
		strcpy(log_file, LOG_DIR);
		strcat(log_file, rcon_name);
	}
	if ((log = open(log_file, O_WRONLY | O_APPEND | O_CREAT, LOG_MODE)) < 0 ) {
		printf("%s: main(); Unable to open log file %s - ", name, log_file);
		fflush(stdout);
		perror("open");
		exit(1);
	}
	chmod(log_file,LOG_MODE);
	log_length = lseek(log, 0, L_XTND);
	sprintf(logline, "\n\n*****rconsoled restarted %s", date());
	write(log, logline, strlen(logline));
	if (debug)
		printf("%s: main(); Opened log file %s.\n",
			name, log_file);

	if ((tty = open(rcon_tty, O_RDWR)) < 0) {
		printf("%s: main(); Unable to open tty line %s - ", name, rcon_tty);
		fflush(stdout);
		perror("open");
		exit(1);
	}
	if (debug)
		printf("%s: main(); Opened tty link %s.\n",
			name, rcon_tty);

/*
	signal(SIGHUP, terminate);
	signal(SIGINT, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGPIPE, SIG_IGN);
*/
	tty_save(tty);
	tty_set(tty);

/*	fclose(stdout);	
	fclose(stderr); */

	if (!fake_conn) {
/*		fclose(stdin);
*/		loop(s);
	} else {
		loop(0);
	}

usage:
	printf("%s: rconsoled {-acd -l log} machine\n", name);
	exit(1);
}

est_conn(rcname, rcon_name, rcon_tty, baud)
char *rcname, *rcon_name, *rcon_tty;
int *baud;
{
int s;				/* Socket descriptor for chat connections */
struct sockaddr_in sin;		/* INet socket address */
struct hostent *hp;
int serv_port;

	if ((*baud = lookup_machine(rcname, rcon_name, rcon_tty,
				  		&serv_port, &hp)) < 0) {
		printf("%s: est_conn(); Machine name not recognized %s\n", name, rcname);
		exit(1);
	}

	if (debug)
		printf("%s: est_conn(); name %s, real name %s, tty %s, port %d, baud %d\n",
			name, rcname, rcon_name, rcon_tty, ntohs(serv_port), *baud);
	if (hp == NULL) {
		printf("%s: est_conn(); server host not in network database.\n", name);
		exit(1);
	}
	if (debug)
		printf("%s: est_conn(); daemon %s, addr type %d, addr len %d, addr %x\n",
			name, hp->h_name, hp->h_addrtype, hp->h_length,
			(int *)&hp->h_addr);

	if (fake_conn)
		return 0;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	   printf("%s: est_conn(); ", name);
	   fflush(stdout);
	   perror("socket");
	   exit(1);
	}

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = serv_port;
	if (bind(s, (caddr_t)&sin, sizeof (sin)) < 0) {
		printf("%s: est_conn(); ", name);
		fflush(stdout);
		perror("bind");
		exit(1);
	}

	if (listen(s, 1) < 0) {
		printf("%s: est_conn(); Can't get socket to listen %d - ",
			name, sin.sin_port);
		fflush(stdout);
		perror("listen");
		close(s);
		exit(1);
	}
	if (debug)
		printf("%s: est_conn(); Listening on port %d\n", name, s);
	return s;
}

static int log_state = 0;

loop(s)
{
register int READFDS = (1<<tty) | (1<<s);
register int MAX;
register int cnt;
int readfds;
int sa = 0;
int sl = 0;
int tmp;

	if (debug)
		printf("%s: loop(); Entered s %d, tty %d, READFDS %x\n", name, s, tty, READFDS);
	if (fake_conn) {
		READFDS = 1<<s;
		printf("%s: est_login(); Prime the pump.  ", name);
		if (!a_flag)
			printf("Type something.\n");
		else
			printf("Type a suitable passwd 3 tuple.\n");
	}

	MAX = max(tty,s)+1;

	while(1) {
		readfds = READFDS;
		if (select(MAX, &readfds, 0, 0, 0) < 0) {
			printf("%s: loop(); ", name);
			fflush(stdout);
			perror("select");
			exit(1);
		}

			/* loging in */
		if ((readfds & (1<<s)) || (readfds & (1<<sl))) {
			switch (est_login(s, &sl, readfds & (1<<sl))) {
			case 0:	/* no check.  sa = s */ 
				sa = sl;
				sl = 0;
				MAX=max(max(tty,s),sa)+1;
				READFDS |= (1<<sa);
				if (fake_conn) sl = s = 10;
				if (debug)
					printf("%s: loop(); sa %d, READFDS %x\n", name, sa, READFDS);
				break;
			case 1: /* check. save sl = s */
				MAX=max(max(tty,s),sl)+1;
				READFDS |= (1<<sl);
				if (fake_conn) sa = s = 10;
				break;
			case 2: /* ok. sa = sl */
				sa = sl;
				sl = 0;
				if (fake_conn) s = sl = 10;
				break;
			case -1: /* fail. undo sl */
				close(sl);
				sl = 0;
				READFDS &= ~(1<<sl);
				MAX = max(tty,s)+1;
				if (fake_conn) s = 0;
				break;
			case -2: /* ignore */
				break;
			}
		}
			/* client */
		if (readfds & (1<<sa)) {
				register char *cp = logline;
				register char *np = cp;
				register char *lp;
				register char *cpe;
				int c;
			if ((cnt = read(sa, logline, sizeof logline)) <= 0) {
				printf("%s: loop(); error reading client %d\n", name, cnt);
				fflush(stdout);
				perror("read");
				close (sa);
				READFDS &= ~(1<<sa);
				MAX = max(tty,s)+1;
				sa = 0;
				log_state = 0;
			}
			if (debug) {
				logline[cnt]=0;
				printf("\n%s: loop(); Client %s cnt = %d\r\n", name, logline, cnt);
				fflush(stdout);
			}
			for (cpe = &logline[cnt], lp = logline; cp < cpe;) {
			    		register int c;
				if ((c = *cp++ &0xff) == OOB_CHAR) {
					cnt = cp - lp - 1;
					write(tty, lp, cnt);
					oob_char(*cp++);
					lp = cp;
				} else {
					*np++ = c;
				}
			}
			cnt = cp - lp;
			write(tty, lp, cnt);
		}
			/* tty line */
		if (readfds & (1<<tty)) {
				register char *cp = logline;
				register char *np = cp;
				register char *cpe;
				int cnt;
			if ((cnt = read(tty, logline, sizeof (logline))) <= 0) {
				perror("rconsoled: loop(); read console failed");
				sprintf(logline, "****rconsoled: error reading console file: %d, cnt= %d.\n",errno,cnt);
				write(log, logline, strlen(logline));
				exit(1);
			}
			if (debug) {
				logline[cnt]=0;
				printf("\n%s: loop(); tty %s cnt = %d\r\n", name, logline, cnt);
				fflush(stdout);
			}
			if (sa)
				write(sa, logline, cnt);
			for (cpe = &logline[cnt]; cp < cpe;) {
				if ((*np++ = (*cp++ & 0x7f)) == '\r'){
					np--; cnt--;
				}
			}
			if ( (log_length += cnt) > LOG_LIMIT)
				new_log();
			if (log_flag)
				write(log, logline, cnt);
		}
	}
}

static char inpass[100];

est_login(s, sl, active)
int *sl;
{
int c;
register int cnt;
int rej;
	if (debug)
		printf("%s: est_login(); Entered s %d, sl %d\n", name, s, *sl);
	switch (log_state) {
	case 0:
		if (debug)
			printf("%s: est_login(); Case 0, a_flag %d\n", name, a_flag);
		if (!fake_conn)
			*sl = accept(s, 0, 0);
		else
			*sl = s;
		if (a_flag) {
			c = 'b';
			write(*sl, &c, 1);
			log_state++;
			return 1;
		} else {
			c = '\n';
			write(*sl, &c, 1);
			log_state = 2;
			return 0;
		}
		break;
	case 1:
		if (debug)
			printf("%s: est_login(); Case 1\n", name);

		if ((cnt = read(*sl, inpass, sizeof inpass)) <= 0) {
			printf("%s: est_login(); error reading daemon %d\n", name, cnt);
			fflush(stdout);
			perror("read");
			exit (2);
		}

		if (check_passwd(inpass, cnt, *sl)) {
			log_state = 2;
			return 2;
		} else {
			log_state = 0;
			return -1;
		}
		break;
	case 2:
		if (debug)
			printf("%s: est_login(); Case 2 refusing login\n", name);

		if (!active)
			printf("%s: est_login(); Case 2 and not active??\n", name);

		sprintf(inpass, "nSorry %s is using the daemon.  Go bug him/her.\n",
			loginfo);
		cnt = strlen(inpass) + 1;
/* check values everywhere ... */
		rej = accept(s, 0, 0);
		if (write(rej, inpass, cnt) != cnt) {
			printf("%s: est_login(); write client failed  ", name);
			perror("socket write");
		}
		close(rej);
		return -2;
		break;
	}
}

check_passwd(str, cnt, sl)
char *str;
{
register struct passwd *pwd = getpwnam(logname);
register char *namep;
register char *cp;
int c;

	str[cnt] = 0;
	cp = str;
	while ( (c = *cp++) && c != ' ') ;
	*--cp = 0;				/* null terminate passwd */
	for (c = *cp++; c == ' '; c = *cp++);
	strcpy(loginfo, cp);
 
	if (debug) {
		printf("%s: check_passwd(); loginfo = %s, name = %s pwd = %s\n",
			name, loginfo, pwd->pw_name, pwd->pw_passwd);
		fflush(stdout);
	}

	namep = crypt(str, pwd->pw_passwd);

	if (!strcmp(namep, pwd->pw_passwd)) {
		c = 'y';
		write(sl,&c,1);
		return 1;
	} else {
		c = 'n';
		write(sl,&c,1);
		return 0;
	}
}

oob_char(c) {
	switch(c&0xff) {
	case OOB_TERM:
		if (debug)
			printf("%s: oob_char(); OOB_TERM\n", name);
		sigsetmask(0);
		sprintf(logline,"\n\n*****rconsoled terminated %s\n", date());
		write(log,logline,strlen(logline));
		tty_restore(tty);
		exit(0);
		break;

	case OOB_BRK:
		if (debug)
			printf("%s: oob_char(); OOB_BRK\n", name);
		tty_brk(tty);
		break;
	case OOB_DEBUG:
		debug = ! debug;
		if (debug)
			printf("%s: oob_char(); OOB_DEBUG, debug = %d\n",
				name, debug);
		break;
	case OOB_LOG:
		log_flag = ! log_flag;
		if (debug)
			printf("%s: oob_char(); OOB_LOG, log_flag = %d\n",
				name, log_flag);
		break;
	default:
		if (debug)
			printf("%s: oob_char(); ignoring random char read after esc = %c\n",
				name, c);
		break;
	}
}

static struct sgttyb tty_params;	/* tty basic parameter block */
static struct sgttyb save_tty_params;	/* tty basic parameter block */

tty_save(fd)
{

	if (ioctl(fd, TIOCGETP, &tty_params) < 0) {
		printf("%s: tty_save(); ", name);
		fflush(stdout);
		perror("ioctl(tty, TIOCGETP, ...");
	}
	save_tty_params = tty_params;
}

tty_set(fd)
{
	tty_params.sg_flags |= RAW;
	tty_params.sg_flags &= ~ECHO;
	tty_params.sg_ispeed = baud;
	tty_params.sg_ospeed = baud;
	ioctl(fd, TIOCSETP, &tty_params);
	if (ioctl(fd, TIOCSETP, &tty_params) < 0) {
		printf("%s: tty_set(); ", name);
		fflush(stdout);
		perror("ioctl(tty, TIOCSETP, ...");
	}
}

tty_restore(fd)
{

	tty_params = save_tty_params;
	if (ioctl(fd, TIOCSETP, &tty_params) < 0) {
		printf("%s: tty_set(); ", name);
		fflush(stdout);
		perror("ioctl(tty, TIOCSETP, ...");
	}
}

tty_brk(fd)
{
	if (ioctl(fd, TIOCSBRK, &tty_params) < 0) {
		printf("%s: tty_brk(); ", name);
		fflush(stdout);
		perror("ioctl(tty, TIOCSBRK, ...");
	}
sleep(2);
	if (ioctl(fd, TIOCCBRK, &tty_params) < 0) {
		printf("%s: tty_brk(); ", name);
		fflush(stdout);
		perror("ioctl(tty, TIOCCBRK, ...");
	}
}

char *date() {
	struct timeval now;		/* Current time, in cannonical form */

	gettimeofday(&now, 0);
	return(ctime(&now.tv_sec));
}

new_log() {
	char olog_file[100];	/* Name of old log file */

	sprintf(logline, "\n*****logging terminated %s", date());
	write(log, logline, strlen(logline));
	close(log);

	strcpy(olog_file, log_file);
	strcat(olog_file, ".old");
	rename(log_file, olog_file);

	if ((log = open(log_file, O_WRONLY | O_APPEND | O_CREAT, LOG_MODE)) < 0 ) {
		exit(1);
	}
	chmod(log_file, LOG_MODE);
	sprintf(logline, "*****logging started %s", date());
	write(log, logline, strlen(logline));
	log_length = lseek(log, 0, 2);
}
