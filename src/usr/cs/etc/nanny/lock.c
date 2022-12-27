#include "nanny.h"
#include <sys/file.h>

/**** write_lock:
 *   This function will write lock the file associated with the 
 * given descriptor. Return 0 if the file is locked, -1 otherwise.
 */
write_lock(file)
int file;
{
    int     i;
    char    tmp[DEV_BSIZE];

    for (i = 0; i < 60; i++) {
	if (flock (file, (LOCK_EX|LOCK_NB)) == 0)
	    return(0);
	if (errno != EWOULDBLOCK) {
	    (void)sprintf(tmp, "nanny%d", nanny_num);
	    perror(tmp);
	    return(-1);
	}
	Sleep((time_t)2);
    }
    fprintf(stderr, "nanny: file busy too long.\n");
    return(-1);
}				/* write_lock */

/**** open_lock:
 *  open and lock the file with the specified path name. Return a pointer to 
 * the opened file. The file must always be opened for updating for lock to
 * work.
 */
FILE *open_lock(name, mode)
char *name, *mode;
{
    long tim,
	now;
    FILE *fp;

    n_time(tim);
    now = tim;
    while ((fp = fopen(name, mode)) == NULL && tim + LOCKTIMO > now) {
	if (errno != EBUSY) {
	    fprintf(stderr, "nanny: Unable to open \"%s\" err=%d\n", name, errno);
	    return(NULL);
	}
	Sleep((time_t)2);
	n_time(now);
    }
    if (fp) {
	if (write_lock(fileno(fp)) == -1)
	    fprintf(stderr, "Lock failed for %s\n", name);
    }
    else {
	fprintf(stderr, "%s busy too long. This should not happen\n", name);
	return(NULL);
    }
    if (*mode == 'a')			    /* Make sure we are at the end */
	(void)fseek(fp, 0L, 2);
    return(fp);
} /* open_lock */

/**** unlock:
 *  Unlock the given file
 */
unlock(file)
{
    (void)flock(file, 0);
}
