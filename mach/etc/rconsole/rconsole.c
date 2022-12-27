/*
 * rconsole: Utility program for "chatting" to a remote console terminal line,
 *	using a local tty line.
 *
 * Use: rconsole [-eC] [-l logfile] [-k] name
 *	where name is one of the remote machines listed in /etc/rconsoles.
 *	Any of the available aliases listed in /etc/rconsoles is acceptable.
 *	To terminate the program, type ^_ (CTRL-Underscore).  This escape
 *	character (^_) can be changed through use of the -e option.  No space
 *	should separate the -e option flag from the argument character.
 *	If a script of the console session is desired (in addition to the log
 *	produced by the rconsoled daemon) the -l option can be used.  All
 *	console output will be appended to the end of the file whose name is
 *	specified following the -l flag.  If logfile doesn't exist, it will be
 *	created.  Whether or not the -l option is used, a log of the current
 *	console session will be maintained by the rconsoled daemon, along with
 *	all of the other console activity preceding and following this
 *	session.
 *	The -k flag can be used to kill the rconsoled daemon associated with
 *	the specified remote console.  In this case, no chat session will be
 *	take place, and none will be possible until the daemon is restarted.
 * 
 * HISTORY
 *	14-Jan-86   Bill Bolosky (bolosky) at Carnegie-Mellon University.
 *		Changed to use INET sockets instead of Unix.  Fixed to
 *		allow stdin to not be a tty (check for EOF an halt if not).
 *		Added code to handle a possible password request from
 *		the rconsole daemon.
 *	11-Sep-85  James Wendorf (jww) at Carnegie-Mellon University.
 *		Modified termination character sent to rconsoled, to avoid
 *		accidental termination.  Added check that stdin is a tty.
 *	08-Oct-84  James Wendorf (jww) at Carnegie-Mellon University.
 *		Added -l and -k command line options, and fixed timeout to
 *		work more logically and predictably.
 *	4-Oct-84	Robert V Baron (rvb)		CMU
 *		flush the parity in the esc_char test
 *	02-Oct-84  James Wendorf (jww) at Carnegie-Mellon University.
 *		Added -e command line option.
 *	18-Sep-84  James Wendorf (jww) at Carnegie-Mellon University.
 *		Created.
 */

#include "rconsole.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <signal.h>
#include <sgtty.h>
#include <pwd.h>
#include <stdio.h>

char *getpass();

char *name;			/* pgm name */
int k_flag = 0;			/* tell daemon to self destruct */
int debug = 0;			/* local debugging */
int fake_conn = 0;		/* don't use daemon; Programmer is testing
				   client and will type daemon responses */
int on_tty = 0;			/* stdin is tty */
int log_flag = 0;		/* store to log_file */
char *log_file = (char *) 0;	/* name of log file */
char esc_char = ESC_CHAR;	/* rconsole escape character */

char cbuf[LINE_LENGTH];
char oob_buf[2] = { OOB_CHAR, 0};

main(argc, argv)
int argc;
char **argv;
{
int s;				/* rconsoled socket descriptor */
int log;			/* File pointer for log file */

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
				case 'c':
					fake_conn = 1;
					break;
				case 'd':
					debug = 1;
					break;
				case 'e':
					if (!*token) {
						printf("%s: main(); Invalid escape character specification\n",
							name);
						exit(1);
					}
					esc_char = *token++ & 0x7f;
					break;
				case 'k':
					k_flag = 1;
					break;
				case 'l':
					log_file = *++argv;
					if (--argc < 0) goto usage;
					log_flag = 1;
					break;
				default:
					goto usage;
				}
			}
		}
	}
	on_tty = isatty(0);
	if (debug)
		printf("%s: main(); fake_conn %d, debug %d, on_tty %d, k_flag = %d, log %s, esc %c(%o)\n",
			name, fake_conn, debug, on_tty, k_flag,
			(int) log_file? log_file : "NULL",
			esc_char, esc_char &0xff);

	if (argc == 0) {
		printf("%s: main();  Machine to connect to not specified.\n", name);
		exit(1);
	}

	if (!on_tty && fake_conn) {
		printf("%s: main(); Must be interactive to use fake_conn\n", name);
		exit(1);
	}
	if ((int) log_file)
		if ((log = open(log_file, O_RDWR|O_APPEND)) < 0) {
			printf("%s: main(); Unable to open log file %s - ",
				name, log_file);
			fflush(stdout);
			perror("open");
			exit(1);
		}

	s = est_conn(*argv);
	if (!fake_conn)
		est_login(s, s);
	else
		est_login(0, 1);

	if (k_flag) {
		oob_buf[1] = OOB_TERM;
		write(s, oob_buf, 2);
		printf("%s: main(); The daemon has been sent the termination message\n", name);
		exit(0);
	}

	if (on_tty){
		tty_save(0);
		tty_set(0);
		if (esc_char < ' ')
			printf("%s: Type '^%c{q|e}' to exit.\r\n", name, esc_char+'@');
		else
			printf("%s: Type '%c{q|e}' to exit.\r\n", name, esc_char);
	} else
		printf("%s: main(); Copying file into console tty.\n", name);

	if (!fake_conn)
		loop(s, s, log);
	else
		loop(0, 1, log);
usage:
	printf("%s: rconsole {-dbke# -l log} machine\n", name);
	exit(1);
}

est_conn(rcname)
char *rcname;
{
int s;				/* rconsoled socket descriptor */
struct sockaddr_in sin;		/* Internet socket address. */
struct hostent *hp;		/* Internet host entry. */
char rcon_name[20];		/* Common name for remote machine */
char rcon_tty[25];		/* Path name of tty connected to rem. mach. */
u_short serv_port;		/* INET port number for server. */

	if (lookup_machine(rcname, rcon_name, rcon_tty, &serv_port, &hp) < 0) {
		printf("%s: est_conn(); %s does not have a rconsole line.\n", name, rcname);
		exit(1);
	}

	if (debug)
		printf("%s: est_conn(); name %s, real name %s, tty %s, port %d\n",
			name, rcname, rcon_name, rcon_tty, ntohs(serv_port));

	if (hp == NULL) {
		printf("%s: est_conn(); server host not in network database.\n", name);
		exit(1);
	}
	if (debug)
		printf("%s: est_conn(); daemon %s, addr type %d, addr len %d, addr %x\n",
			name, hp->h_name, hp->h_addrtype, hp->h_length,
			(int *)&hp->h_addr);


	printf("Attempting to connect to %s (via %s) ... ",
		rcon_name, rcon_tty);

	if (fake_conn) {
		printf(" Self ... \n");
		return 1;
	}

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = hp->h_addrtype;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = serv_port;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	   printf("%s: est_conn(); ", name);
	   fflush(stdout);
	   perror("socket");
	   exit(1);
	}
	if (connect(s, (char *)&sin, sizeof(sin)) < 0) {
	   printf("%s: est_conn(); ", name);
	   fflush(stdout);
	   perror("connect");
	   exit(1);
	}
	if (debug) {struct sockaddr_in sname; int len = sizeof (struct sockaddr);
		if (getpeername(s, (struct sockaddr *) &sname, &len) < 0) {
			printf("%s: est_conn(); ", name);
			fflush(stdout);
			perror("getpeername");
		}
		printf("\n%s: est_conn(); connected to %x socket %d",
			name, ntohl(sname.sin_addr.s_addr), ntohs(sname.sin_port));
	}
	return s;
}

est_login(is, os)
{
int readfds;
int cnt;
struct timeval timeout;		/* Time limit for use with select */

	if (fake_conn) {
		printf("%s: provide a daemon char 1 : ... ", name);
		fflush(stdout);
	}
	timeout.tv_sec = TIME_LIMIT;
	timeout.tv_usec = 0;
	readfds = 1 << is;
	if (select(is+1, &readfds, 0, 0, &timeout) <= 0) {
		printf("Failed\n");
		printf("%s: est_login(); This console line is currently in use\n", name);
		exit(1);
	}
	printf("Connected\n");


	if ((cnt = read(is, cbuf, sizeof (cbuf))) <= 0) {
		printf("%s: est_login(); read %d chars from daemon.\n", name, cnt);
		fflush(stdout);
		perror("read");
		exit(2);
	}
	if (debug)
		printf("%s: est_login(); daemon '%s'\n", name, cbuf);

	switch(*cbuf) {
	case 'a':
		{ /* We need to send the password. */
			char *passwd;
			passwd = getpass("Password: ");
			write(os, passwd, strlen(passwd) + 1);  /* +1 sends \0 */
	check_passwd:
			if (fake_conn) {
				printf("%s: provide a daemon char 2: ... ", name);
				fflush(stdout);
			}
			if ((cnt = read(is, cbuf, 1)) != 1) {
				printf("%s: est_login(); error reading daemon %d\n", name, cnt);
				fflush(stdout);
				perror("read");
				exit(2);
			}
	refuse:
			if (*cbuf == 'n') {
				printf("%s: est_login(); Invalid password.  Connection refused.\n",
					name);
				exit(1);
			}
		}
		break;
	case 'b':
		{ /* We need to send the password. */
			char *passwd;
			char hostname[100];		/* for gethostname() */
			char loginfo[100];
			int uid = getuid();
			struct passwd *pdp = getpwuid(uid);

			passwd = getpass("Password: ");
			if (gethostname(hostname, sizeof hostname) < 0) {
				printf("%s: est_login(); gethostname \"%s\" failure, ", name, hostname);
				fflush(stdout);
				perror("gethostbyname");
				exit(1);
			}
			if (pdp == NULL) {
				printf("%s: est_login(): getpwuid \"%d\" failure, ", name, uid);
				exit(1);
			}
			if (debug)
				printf("%s: est_login(); hostname %s, user %s\n",
					name, hostname, pdp->pw_name);
			sprintf(loginfo, "%s %s@%s",
				passwd, pdp->pw_name, hostname);

			write(os, loginfo, strlen(loginfo) + 1);  /* +1 sends \0 */
			goto check_passwd;
		}
		break;
	case 'n':
		if (debug)
			printf("%s: est_login(); Daemon already in use.\n", name);

		printf("%s", cbuf+1);
		exit(1);

	case '\n':
		if (debug)
			printf("%s: est_login(); No password needed.\n", name);
		break;
	default:
		printf("%s: est_login(); rconsole DAEMON returned %c. Abort.\n", name, *cbuf);
		exit(1);
	}

}

loop(is, os, log)
register int is, os;
register int log;
{
register int READFDS = (1 << 0) | (1 << is);
int readfds;

	while(1) {
		readfds = READFDS;
		if (select(is+1, &readfds, 0, 0, 0) < 0) {
			printf("%s: loop(); ", name);
			fflush(stdout);
			perror("select");
			exit(1);
		}

			/* read from socket to daemon */
		if (!fake_conn)
			if (readfds & (1 << is))
				read_daemon(is, 1, log);

			/* read from tty */
		if (readfds & (1 << 0))
			read_tty(0, os);
	}
}

read_daemon(s, ofd, log)
{
register int cnt;
register int c;

	if (fake_conn)
		printf("%s: read_daemon(); supply input: ", name);

	if ((cnt = read(s, cbuf, sizeof cbuf)) <= 0) {
		printf("%s: read_daemon(); error reading daemon %d\n", name, cnt);
		fflush(stdout);
		perror("read");
		exit(2);
	}
	if (debug) {
		cbuf[cnt]=0;
		printf("\n%s: read_daemon(); %s cnt = %d\r\n", name, cbuf, cnt);
		fflush(stdout);
	}
	if (write(ofd, cbuf, cnt) != cnt) {
		printf("%s: read_daemon(); error writing tty %d\n", name, cnt);
		fflush(stdout);
		perror("write");
	}

		/* and possibly log */
	if ((int)log_file) {register char *cp=cbuf, *sp=cbuf, *ep = &cbuf[cnt];
		while (cp < ep)
			if ((c = *cp++) == '\r') {
				if (sp < cp)
					if (log_flag) write(log, sp, cp - sp - 1);
				sp = cp;
			}
		if (sp < cp)
			if (log_flag) write(log, sp, cp - sp);
	}
}

/* We are essentially reinventing stdio below.  We can't afford to do a getc()
 * because we are called from a loop doing a select so that when
 * our initial load of characters is read we must return.
 */
int esc_flag = 0;

#define flush if (sp < cp - (esc_flag?1:0)) write(s, sp, cp - sp - (esc_flag?1:0));\
		 if(debug)printf("%dN\r\n",cp - sp - (esc_flag?1:0)),fflush(stdout)
read_tty(ifd, s)
{
register int cnt;
register char *cp, *sp, *ep;
register int c;
register char *macro;

	if (fake_conn)
		printf("%s: read_tty(); supply input: ", name);

	if ((cnt = read(ifd, cbuf, sizeof cbuf)) <= 0) {
		printf("%s: read_tty(); error reading tty %d\n", name, cnt);
		fflush(stdout);
		perror("read");
		goto out;
	}
	if (debug) {
		cbuf[cnt]=0;
		printf("%s: read_tty(); %s cnt = %d\n", name, cbuf, cnt);
		fflush(stdout);
	}


	for(cp=cbuf, sp=cbuf, ep = &cbuf[cnt]; cp < ep; cp++) {
		if ((c = (*cp &= 0177)) == esc_char) {
			if (++esc_flag > 1) {
				esc_flag = 0;	/* two esc_chars go through as one. */
				/* flush;	/* first esc flushed */
				/* sp = cp + 1;	/* leave pointer so we output esc char */
			}
		} else if (esc_flag) {	/* we'd need to read to get next char */
			flush;
			sp = cp+1;	/* step over esc char */
			esc_flag = 0;
			switch(c) {
			case 'b':
				printf("esc b (send break)\n", name);
				oob_buf[1] = OOB_BRK;
				write(s, oob_buf, 2);
				break;
			case 'd':
				debug = ! debug;
				printf("esc d (debug client), debug = %d\n",
						name, debug);
				break;
			case 'D':
				printf("esc s (debug daemon)\n", name);
			     	oob_buf[1] = OOB_DEBUG;
				write(s, oob_buf, 2);
				break;
			case 'Q':
			case 'E':
				printf("esc k (kill daemon)\n", name);
			     	oob_buf[1] = OOB_TERM;
				write(s, oob_buf, 2);
				break;
			case 'q':
			case 'e':
				printf("esc \"quit\" (exit client)\r\n", name);
				goto out;
				break;
			case 'l':
				log_flag = ! log_flag;
				printf("esc l (log local), log_flag = %d\n",
						name, log_flag);
				break;
			case 'L':
				printf("esc L (log daemon)\n", name);
			     	oob_buf[1] = OOB_LOG;
				write(s, oob_buf, 2);
				break;
			case 't':		/* would you believe a macro */
				printf("esc t (send wt*=80a0)\n", name);
				macro = "wt0=80a0wt1=80a0wpra";
				write(s, macro, strlen(macro));
				break;
			case 'z':
				printf("esc z (suspend client)\n", name);
				tty_restore(ifd);
				kill(0, SIGTSTP);
				tty_set(ifd);
				break;
			default:
				printf("%s: read_tty(); ignoring random char read after esc = %c\n",
					name, c);
				break;
			}
		}
	}
	flush;
	return;
out:
	if (on_tty)
		tty_restore(ifd);
	exit(0);
}

static struct sgttyb tty_params;	/* tty basic parameter block */
static short tty_flags;			/* Flag field from tty parameter block */

tty_save(fd)
{
	if (!on_tty) return;

	if (ioctl(fd, TIOCGETP, &tty_params) < 0) {
		printf("%s: tty_save(); ", name);
		fflush(stdout);
		perror("ioctl(0, TIOCGETP, ...");
	}
	tty_flags = tty_params.sg_flags;
}

tty_set(fd)
{
	if (!on_tty) return;

	tty_params.sg_flags |= RAW;
	tty_params.sg_flags &= ~ECHO;
	ioctl(fd, TIOCSETP, &tty_params);
	if (ioctl(fd, TIOCSETP, &tty_params) < 0) {
		printf("%s: tty_set(); ", name);
		fflush(stdout);
		perror("ioctl(0, TIOCSETP, ...");
	}
}

tty_restore(fd)
{
	if (!on_tty) return;

	tty_params.sg_flags = tty_flags;
	if (ioctl(fd, TIOCSETP, &tty_params) < 0) {
		printf("%s: tty_set(); ", name);
		fflush(stdout);
		perror("ioctl(0, TIOCSETP, ...");
	}
}
