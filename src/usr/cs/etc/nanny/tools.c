#include "nanny.h" 
/************************ 
 * HISTORY
 ************************/

/**** kill_srv:
 *   Kill off the named server by sending a kill -9 to its PID. Inform the
 * mother nanny of what we did.
 */
kill_srv(srv)
SERVER *srv;
{
    if (srv->status != RUNNING)
	return;
    print_log("Killing server %s\n", srv->basis->name);
    if (srv->pid < 3) {
	print_err("Process ID was %d. Signal not sent\n", srv->pid);
	return;
    }
    srv->status = KILLED;
    (void)kill(srv->pid, SIGKILL);
    death(srv->pid);
}				/* kill_srv */

/**** restart_srv:
 *   Attempt to restart the given server. We will not restart servers that 
 * should not run on this host.
 */
restart_srv(s)
SERVER *s;
{
    if (!s->valid)
	return;
    if (s->status == RUNNING)
	kill_srv(s);
    s->status = DEAD;
    spawn();
}

/**** cont_srv:
 *  This will only work on those servers that are already stopped.
 */
cont_srv(s)
SERVER *s;
{
    if (s->status != STOPPED)
	return;
    (void)kill(s->pid, SIGCONT);
}

/**** stop_srv:
 *  We only stop running servers.
 */
stop_srv(s)
SERVER *s;
{
    if (s->status != RUNNING)
	return;
    (void)kill(s->pid, SIGSTOP);
}

/**** add_srv:
 *  Add the given server to the given list at the tail and return a pointer 
 * to the new head if one was allocated. Only add the server if it does not
 * exist already. No duplications permitted.
 */
SERVER *add_srv(head, item)
SERVER *head, *item;
{
    SERVER *srv;

    if (head && item) {
	for (srv = head; srv->next; srv = srv->next) {
	    if (strcmp(srv->basis->name, item->basis->name) == 0)
		return(head);
	}
					    /* No duplicate names allowed!! */
	if (strcmp(srv->basis->name, item->basis->name) == 0)
	    return(head);
	srv->next = item;
	item->prev = srv;
	item->next = NULL;
	return(head);
    }
    if (item) {
        item->next = NULL;
	item->prev = NULL;
	return(item);
    }
    return(head);
} /* add_srv */

/**** mk_srv:
 *   The needed memory for a server structure is allocated and a
 * pointer to that structure is returned. 
 */
SERVER *mk_srv()
{
    SERVER * srv;

    if ((srv = (SERVER *) calloc(1, sizeof(SERVER))) == NULL) {
	fprintf(stderr, "nanny: Unable to allocate %d bytes of memory\nAborting\n\r", sizeof(SERVER));
	exit(1);
    }
    srv->status = DEAD;
    srv->pid = -1;
    srv->restarts = -1;
    return(srv);
}				/* mk_srv */

/**** mk_basis:
 *   Allocate memory for the basis of the server structure. Return a
 * pointer to the new structure. The structure should already be 
 * initialized
 */
SHORTSRV *mk_basis()
{
    SHORTSRV *basis;

    if ((basis =(SHORTSRV *) calloc(1, sizeof(SHORTSRV))) == NULL) {
	quit(1, "nanny: Unable to allocate %d bytes of memory\nAborting\n", sizeof(SHORTSRV));
    }
    return(basis);
} /* mk_basis */

/**** mk_path:
 *  Scan down the given path creating those directories that do not 
 * already exist. 
 */
mk_path(path)
char *path;
{
    char    head[128],
           *tail,
           *rindex();
    int     len;

    if (access(path, F_OK) == -1) {
    /* Insure that the head of the path exists */
	if ((tail = rindex(path, '/')) != NULL) {
	    len = tail - path;
	    (void)strncpy(head, path, len);
	    head[len] = 0;
	    mk_path(head);
	}
	if (mkdir(path, 0777))
	    print_err("Unable to create dir %s err=%d\n", path, errno);
    }
}			       /* mk_path */

/**** last_mod:
 *   Return the last mod time for the given file. Full path names are
 * recomended.
 */
long last_mod(file)
char *file;
{
    struct stat buf;

    errno = 0;
    if (stat(file, &buf) < 0) {
	print_err("Unable to get status of file %s err=%d\n", file, errno);
	return(0);
    }
    return(buf.st_mtime);
}

/**** free_srv:
 *   The server pointed to by the argument is removed from memory. Note that 
 * NOTHING is done with the links. These must be rearranged before the call 
 * to this function.
 */
free_srv(srv)
SERVER *srv;
{
    if (srv == NULL)
        return;

    if (srv->basis) 
        free_basis(srv->basis);
    if (srv->gid)
	free((char *)srv->gid);
    free((char *)srv);
} /* free_srv */

/**** free_basis:
 *   free the components of the basis, then the basis itself.
 */
free_basis(base)
SHORTSRV *base;
{
    if (base->name)
	free(base->name);
    if (base->runfile)
	free_list(base->runfile);
    if (base->script)
	free_list(base->script);
    if (base->uid)
	free_list(base->uid);
    if (base->gid)
	free_list(base->gid);
    if (base->attributes)
	free_list(base->attributes);
    free((char *)base);
}

/**** free_list:
 *   Free the space allocated in the list of string pointers.
 */
free_list(list)
char **list;
{
    char **tmp = list;

    if (!list)
	return;
    while (*tmp)
	free(*tmp++);
    free((char *)list);
}

/**** usage:
 *   print a quick message to the user on how to use nanny.
 */
usage()
{
    print_err(" Usage: nanny [-wd <path>] [-logdir <path>] [-init <path>]\n\t[-start <server>] [-list] [-status <server>]\n\t[-continue <server>] [-stop <server>] [-kill <server>]\n\t[-restart <server>] [-generate <path>] [-reconfigure <path>]\n\t[-Suicide] [-cd]\n");
}

/**** fetch_srv:
 *   Look up the server with the given name, null if not found.
 */
SERVER *fetch_srv(name)
char *name;
{
    SERVER * srv;

    if (!name)
	return(NULL);
    for (srv = s_first; srv; srv = srv->next)
	if (strcmp(srv->basis->name, name) == 0)
	    break;
    return(srv);
}

/**** ingrp:
 *  Check to see if the current process is already working under the
 * given set of groups. If we lack anything, we return 0, 1 otherwise.
 */
ingrp(list)
int *list;
{
    int     us[NGROUPS],
            i,
            num;

    if ((num = getgroups(NGROUPS, us)) == -1)
	quit(2, "Unable to get current group list!! err=%d\n", errno);
    while (*list != -1) {
	for (i = num; i && *list != us[i]; i--);
	if (!i)
	    return(0);
	list++;
    }
    return(1);
}

char *Malloc(siz)
unsigned siz;
{
    char   *tmp;

    if (tmp = (char *) malloc(siz))
	return(tmp);
    exit(ENOMEM);
    return(NULL);
}
