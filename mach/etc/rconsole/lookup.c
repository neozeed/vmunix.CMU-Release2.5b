/*
 * lookup_machine: Function used by both rconsole and rconsoled for looking
 *	up a given machine name in the file /etc/rconsoles.  It returns the
 *	common name used to refer to the machine, the path name of the
 *	tty connected to that machine, and the INET port number used to
 *      access the server for that machine.  The value returned by the 
 *	function is the baud rate code for the tty line (see <sgtty.h>) if 
 *	the lookup was successful, and -1 if the name couldn't be found.  
 *
 * HISTORY:
 * 22-Sep-86  William Bolosky (bolosky) at Carnegie-Mellon University
 *	Changed to use fopenp to search down LPATH to find the rconsoles
 *	file, and to use CONSOLE_FILE only if fopenp fails.
 *
 * 13-Feb-86  Bill Bolosky (bolosky) at Carnegie-Mellon University
 *	Added include of <netinet/in.h> so that it will get the htons
 *	macro and work on byte-swapped machines (like the Sailboat).
 *
 *	18-Sep-84  James Wendorf (jww) at Carnegie-Mellon University.
 *		Created.
 *
 * 	13-Jan-86  Bill Bolosky (bolosky) at Carnegie-Mellon University.
 *		Modified to return an INET port number associated
 *		with a server, and the host on which is runs.
 */


#include "rconsole.h"

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

char *getenv();
FILE *fopenp();
extern int debug;

static char LPATH[] = ":/usr/mach/lib:/usr/cs/lib:/lib:/usr/lib";

lookup_machine(name, common_name, tty_path, port_num, hp)
char *name;		/* Name of machine to be looked up */
char *common_name;	/* Returns common name for machine */
char *tty_path;		/* Returns path name of tty connected to machine */
u_short *port_num;	/* Returns INET port number for server. */
struct hostent **hp;	/* Returns the hostent for the server's host. */
{
	FILE *f;				/* File pointer for CONSOLE_FILE */
	char tty[20];				/* Name of tty connected to remote machine */
	int baud;				/* Baud rate code for tty (see <sgtty.h>) */
	char line[LINE_LENGTH];			/* Buffer to hold line from CONSOLE_FILE */
	char alias[NUM_ALIASES][ALIAS_LENGTH];  /* List of aliases */
	char *lpath, buffer[200] /*XXX*/ ;
	register int n;				/* Number of aliases for one machine */
	char	host_name[200];			/* Server host name. */
	int	port;

	if ((lpath = getenv("LPATH")) == NULL)
		lpath = LPATH;
	if (debug)
		printf("lpath = \"%s\"\n", lpath);

	if ((f = fopenp(lpath, "rconsoles", buffer, "r")) != NULL) {
		if (debug)
			printf("buffer = \"%s\"\n", buffer);
	} else if ((f = fopen(CONSOLE_FILE, "r")) == NULL)  return(-1);

	while (fscanf(f, "%s %d %s %d", tty, &baud, host_name, &port) 
								   != EOF) {
		if (debug)
			printf("entry: tty %s, baud %d, host_name %s, port_num %d\n",
				tty, baud, host_name, port);
		*port_num = port;
		fgets(line, LINE_LENGTH, f);
		n = sscanf(line, "%s %s %s %s %s %s %s %s %s %s",
		alias[0], alias[1], alias[2], alias[3], alias[4],
		alias[5], alias[6], alias[7], alias[8], alias[9]);
		for (n--;  n >= 0;  n--) {
			if (strcmp(name, alias[n]) == 0) {
				strcpy(common_name, alias[0]);
				strcpy(tty_path, "/dev/");
				strcat(tty_path, tty);
				*port_num = htons(*port_num);
				*hp = gethostbyname(host_name);
				fclose(f);
				return(baud);
			}
		}
	}

	fclose(f);
	return(-1);
}
