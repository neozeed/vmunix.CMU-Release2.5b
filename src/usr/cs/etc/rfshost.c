/*
 **********************************************************************
 * HISTORY
 * $Log:	rfshost.c,v $
 * Revision 1.4  88/11/08  20:28:40  gm0w
 * 	Changed to use correct include files.
 * 	[88/11/04            gm0w]
 * 
 * 03-Jan-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed to take all info as arguments instead of using host
 *	lookups.  Tricky code is now all located in mkremhosts.
 *
 * 16-Jan-86  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed to return non-garbage error status.
 *
 * 21-Nov-85  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed to work better if the remote link file already exists.
 *
 **********************************************************************
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/table.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libc.h>

main(argc, argv)
int argc;
char **argv;
{
    register char *name;
    register fd = -1;
    char hname[MAXHOSTNAMELEN];
    struct in_addr addr;
    struct stat st;
    int modes;
    int error = 0;

    if (argc <= 2)
	quit(-1, "Usage: rfshost addr hostname [ aliases ]\n");

    argc -= 2;
    addr.s_addr = inet_addr(*++argv);
    if (addr.s_addr == -1)
	quit(-1, "Unable to parse address %s\n", *argv);

    hname[0] = '.';
    (void) strcpy(&hname[1], *++argv);

    /*
     *  Suppress symbolic/remote link following so we can treat
     *  the files without special quoting below.
     */
    modes = getmodes();
    setmodes(modes|UMODE_NOFOLLOW);

    /*
     * Create link, if necessary.
     */
    if (lstat(hname, &st) < 0) {
	printf("creating %s address %s\n", hname, inet_ntoa(addr));
	fd = open(hname, O_WRONLY|O_CREAT|O_TRUNC, 0744);
	if (fd < 0) {
	    perror(hname);
	    exit(-1);
	}
	(void) write(fd, (char *)&addr.s_addr, sizeof(addr.s_addr));
	(void) close(fd);
    }

    /*
     * Create links for aliases.
     */
    while (--argc > 0) {
	name = *++argv;
	if (fd >= 0)
	    (void) unlink(name);
        else if (stat(name, &st) >= 0)
	    continue;
	printf("creating link %s to %s\n", name, hname);
    	if (link(hname, name) < 0) {
	    perror(name);
	    error = -1;
	}
    }

    /*
     * Set correct file mode and ownership.
     */
    if (fd >= 0) {
	if (chmod(hname, 0744) < 0) {
	    perror("chmod");
	    error = -1;
	}
	if (chown(hname, 0, 64) < 0) {
	    perror("chown");
	    error = -1;
	}
    }

    /*
     * Restore normal link handling.
     */
    setmodes(modes);

    /*
     * Exit with meaningful status.
     */
    exit(error);
}
