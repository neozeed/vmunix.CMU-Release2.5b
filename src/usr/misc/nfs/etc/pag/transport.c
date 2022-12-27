/****************************************
 *
 * HISTORY
 * $Log:	transport.c,v $
 * Revision 2.0  89/06/15  15:35:44  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

char *service;
int comm_socket;

set_comm_socket(s)
{
	comm_socket = s;
}

close_comm_socket()
{
	close(comm_socket);
}


unsigned long client_inet_addr()
{
	struct sockaddr_in sa;
	int sa_size = sizeof(sa);

	if (getpeername(comm_socket, &sa, &sa_size) < 0) {
		return 0;
	}
	return sa.sin_addr.s_addr;
}

netread(buf, len)
char *buf;
int len;
{
	int len_read, resid;

	for (resid = len; resid > 0; resid -= len_read, buf += len_read) {
		len_read = read(comm_socket, buf, resid);
		if (len_read < 0) {
			fatal("read from network");
		}
		if (len_read == 0) {
			fprintf(stderr, "%s: eof on network input\n",service);
			exit(1);
		}
	}
}

/*
 *  Buffering netwrite would be a big win. However, we would have to
 *  add a netflush function.
 */

netwrite(buf, len)
char *buf;
int len;
{
	int len_written, resid;

	for (resid = len; resid > 0; resid -= len_written, buf += len_written){
		len_written = write(comm_socket, buf, resid);
		if (len_written < 0) {
			fatal("write to network");
		}
		
	}
}

send_long(l)
long l;
{
	int netlong;

	netlong = htonl(l);
	netwrite((char *)&netlong, sizeof(netlong));
}

long recv_long()
{
	int netlong;

	netread((char *)&netlong, sizeof(netlong));
	return ntohl(netlong);
}

send_string(s)
char *s;
{
	long len;

	len = strlen(s) + 1;
	send_long(len);
	netwrite(s, len);
}

recv_string(s, n)
char *s;
int n;
{
	long len;

	len = recv_long();
	if (len > n) {
		fprintf(stderr, "%s: recv_string: string too long (%d)\n",
			service, len);
		exit(1);
	}
	netread(s, len);
}

fatal(s)
char *s;
{
	char perror_string[64];
	extern char *sys_errlist[];
	extern int errno, sys_nerr;

	if (errno > 0 && errno < sys_nerr) {
		sprintf(perror_string, ": %s", sys_errlist[errno]);
	} else {
		perror_string[0] = 0;
	}
	fprintf(stderr, "[pagd] %s: %s%s\n", service, s, perror_string);
	exit(1);
}

send_args(argc, argv)
int argc;
char **argv;
{
	int j, len, stringspace;

	send_long(argc);
	stringspace = 0;
	for (j = 0; j < argc; j++) {
		stringspace += (strlen(argv[j]) + 1);
	}
	send_long(stringspace);
	for (j = 0; j < argc; j++) {
		len = strlen(argv[j]) + 1;
		send_long(len);
		netwrite(argv[j], len);
	}
}

recv_args(argcp, argvp)
int *argcp;
char ***argvp;
{
	int j, len, stringspace, argc;
	char *argj, **argv;

	argc = recv_long();
	stringspace = recv_long();
	argj = (char *) malloc(stringspace);
	argv = (char **) malloc((argc + 1) * sizeof(char *));
	for (j = 0; j < argc; j++) {
		len = recv_long();
		netread(argj, len);
		argv[j] = argj;
		argj += len;
	}
	argv[argc] = 0;
	*argcp = argc;
	*argvp = argv;
}

#define	STREAM_TYPE_OUT		0
#define	STREAM_TYPE_ERR		1
#define	STREAM_TYPE_EOF		2

#define	STREAM_MAX_PACKET_SIZE	1024

/*
 *  ========================= begin 4.2 compat junk =========================
 */
#ifndef	FD_SET
#define	NBBY	8

/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#endif	FD_SET

/*
 *  ========================= end 4.2 compat junk =========================
 */

send_stream(out, err)
int out, err;
{
	int fd;
	int numfds;
	int len_read;
	int nfds;
	fd_set fdset;
	char buf[STREAM_MAX_PACKET_SIZE];

	if (out > err) {
		nfds = out + 1;
	} else {
		nfds = err + 1;
	}
	FD_ZERO(&fdset);
	numfds = 2;
	while (numfds > 0) {
		if (numfds > 1) {
			FD_SET(out, &fdset);
			FD_SET(err, &fdset);
			if (select(nfds, &fdset, 0, 0, 0) < 0) {
				fatal("select");
			}
			fd = (FD_ISSET(err, &fdset) ? err : out);
		}
		len_read = read(fd, buf, STREAM_MAX_PACKET_SIZE);
		if (len_read < 0) {
			fatal("send_stream: read");
		}
		if (len_read == 0) {
			numfds--;
			fd = (fd == out ? err : out);
		} else {
			send_long(fd == out ? STREAM_TYPE_OUT:STREAM_TYPE_ERR);
			send_long(len_read);
			netwrite(buf, len_read);
		}
	}
	send_long(STREAM_TYPE_EOF);
}

recv_stream(out, err)
int out, err;
{
	int len, stream_type;
	char buf[STREAM_MAX_PACKET_SIZE];

	for (;;) {
		stream_type = recv_long();
		if (stream_type == STREAM_TYPE_EOF) {
			return;
		}
		len = recv_long();
		netread(buf, len);
		if (stream_type == STREAM_TYPE_OUT) {
			write(out, buf, len);
		} else {
			write(err, buf, len);
		}
	}
}
