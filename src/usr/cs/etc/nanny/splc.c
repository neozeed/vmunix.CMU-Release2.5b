#include "nanny.h"

/*************************
 * HISTORY
 *************************/

/**** srv_splc:
 *   We first divide the server lists into those common to both lists,
 * those that are only in the new list and those that are only in the
 * old list. 
 *   Those in common are compaired to each other for differences in their
 * basis. If the differ, the new replaces the old and the old server is
 * killed if we are the Nanny server in charge.
 *   Those that are marked old are all killed if they are ours.
 *   Those that are marked new are appended to the original list and a 
 * pointer to the new servers is returned.
 */
srv_splc(old, new)
SERVER *new, *old;
{
    SERVER         *newp,
                   *oldp;
    SERVER         *rm_srv(), *add_srv();
    SHORTSRV       *basis;
    int            *tmp;

    oldp = old;
    for (oldp = old; oldp; oldp = oldp->next) {
	/* Compare common servers */
	for (newp = new; newp; newp = newp->next) {
	    if (strcmp(newp->basis->name, oldp->basis->name) == 0)
		break;
	}				    /* Found old in new list */
	if (newp) {
	    basis = oldp->basis;	    /* swap new and old data */
	    oldp->basis = newp->basis;
	    newp->basis = basis;
	    oldp->valid = newp->valid;
	    oldp->uid = newp->uid;
	    tmp = oldp->gid;
	    oldp->gid = newp->gid;
	    newp->gid = tmp;
	    /* Kill the server if it has changed */
	    if (srv_cmp(oldp, newp) != 0 && profile == NANNY) {
		kill_srv(oldp);
		oldp->status = DEAD;
	    }
	    new = rm_srv(new, newp);	    /* Free useless profile contained in new */
	    free_srv(newp);
	}
	else { /* Did not find old in new list */
	    print_log("%s no longer needed\n", oldp->basis->name);
	    kill_srv(oldp);
	}
    }
    /*
     * What are left in the new list are the totally new servers that need to be allocated to a
     * nanny server. We will append these to the list of old servers.
     */
    for (oldp = old; oldp && oldp->next; oldp = oldp->next);
    if (oldp)
	oldp->next = new;
    else
	old = new;
    s_first = old;
}				/* srv_splc */
 
/**** get_attr:
 *  We call gethostattr to get the list of attribute and then copy them
 * into a safe place. Thus we have a list of strings. The free_list() 
 * function can safely release the allocated memory. Return NULL on
 * error.
 */
#define ATTRIB_LEN 256

char **get_attr()
{
    static char *vec[ATTRIB_LEN];
    int     len;

    if ((len = gethostattr(vec, ATTRIB_LEN)) < 0) {
	print_err("Unable to gather host attributes len=%d err=%d\n", len, errno);
	return(NULL);
    }

    if (len > ATTRIB_LEN)
	print_err("Too many attributes\n");
    vec[ATTRIB_LEN - 1] = 0;
    return(vec);
}

/**** cmp_attr:
 *   Compair the two attribute list and return if they are "equivilent" or
 * not. 0 or 1 respectively.
 */
cmp_attr(at1, at2)
char **at1, **at2;
{
    char  **d,
          **x;

    for (d = at1; *d; d++) {
	for (x = at2; *x; x++)
	    if (!strcmp(*d, *x))
		break;
	if (!(*x))			    /* did not find d in x */
	    return(1);
    }
    for (d = at2; *d; d++) {
	for (x = at1; *x; x++)
	    if (!strcmp(*d, *x))
		break;
	if (!(*x))			    /* did not find d in x */
	    return(1);
    }
    return(0);
}

/**** ck_cnfg:
 *   Ck_cnfg's first duty is to set the creation time for the specified
 * file if the time has not been set before. If the name of the file changes
 * then it is treated as a modification of the original and replaces it.
 * Now, if the file name has changed or the file has been modified since the
 * last call, this function will do a reload and use that file name from then
 * on. On the first call to this function, we always just return.
 *   Our other function here is to double check the attribute list for this
 * host. If it has changed, we will do a reload.
 */
ck_cnfg()
{
    static long lastmod = NULL;
    static char *oldcnfg = NULL;
    long    mod,
            last_mod();
    char  **attr;
    int     reload;

    mod = last_mod(config);
    reload = 0;

    if (oldcnfg == NULL) {
	oldcnfg = salloc(config);
	lastmod = mod;
	return(0);
    }
    if (strcmp(oldcnfg, config) != 0) {
	print_log("Configuration file has been changed to %s\n", config);
	free(oldcnfg);
	oldcnfg = salloc(config);
	lastmod = mod;
	reload++;
    }
    if (mod > lastmod) {
	print_log("Configuration file has been altered\n");
	lastmod = mod;
	reload++;
    }
 /* check host attributes */
    if (cmp_attr(attr = get_attr(), attrib)) {
	free_list(attrib);
	attrib = attr;
	print_log("Host attributes have changed\n");
	reload++;
    }
    if (reload) {
	print_log("Doing reload\n");
	if (recnfg(config) < 0)
	    print_err("Unable to reload from %s!!\n", config);
	return(1);
    }
    return(0);
}				/* ck_cnfg */

/**** recnfg:
 *  The given configuration file will be loaded and the internal structures 
 * updated to correspond to the new file. 
 */
recnfg(cnfg)
char *cnfg;
{
    SERVER *srv1, *srv2, *new_load();

    srv1 = s_first;
    if ((srv2 = new_load(cnfg)) == NULL) {
	print_err("reconfig FAILED!!\n");
	return(-1);
    }
    srv_splc(srv1, srv2);	/* Splice srv2 and srv1 */
    spawn();
    return(0);
} /* recnfg */

/**** rm_srv:
 *   Remove the secound argument from the pointed to list and return a 
 * pointer to the first element of the adjusted list. We are assuming that
 * the secound argument does exist in the first arguments list. The item is
 * not purged from memory. That is up to some other function.
 */
SERVER *rm_srv(head, item)
SERVER *head, *item;
{
    if (item->next)
	item->next->prev = item->prev;
    if (item->prev)
	item->prev->next = item->next;
    else 
	head = item->next;
    return(head);
} /* rm_srv */

/**** srv_cmp:
 *   Compair the two given servers and return 0 if they are the same, 1 if
 * they differ.
 */
srv_cmp(srv1, srv2)
SERVER *srv1, *srv2;
{
    if (strcmp(srv1->basis->name,srv2->basis->name) != 0) {
	print_err("Names do not match for servers!! Internal ERROR\n");
	return(1);
    }
    if (list_cmp(srv1->basis->script, srv2->basis->script) != 0
	|| list_cmp(srv1->basis->runfile, srv2->basis->runfile) != 0
	|| list_cmp(srv1->basis->uid, srv2->basis->uid) != 0
	|| list_cmp(srv1->basis->gid, srv2->basis->gid) != 0
	|| list_cmp(srv1->basis->attributes, srv2->basis->attributes) != 0
	|| srv1->valid != srv2->valid
	|| srv1->basis->nice != srv2->basis->nice
       )
    {
	print_log("%s's profile has changed \n", srv2->basis->name);
	return(1);
    }
    return(0);
} /* srv_cmp */

/**** list_cmp:
 *  Compair the two lists. If they differ, in any way, return 1, else 0.
 */
list_cmp(one, two)
char **one, **two;    
{
    char **tone, **ttwo;

    for (tone = one; *tone; tone++) {
	for (ttwo = two; *ttwo; ttwo++) {
	    if (strcmp(*tone, *ttwo) == 0)
		break;
	}
	if (!*ttwo)
	    return(1);
    }
    for (ttwo = two; *ttwo; ttwo++) {
	for (tone = one; *tone; tone++) {
	    if (strcmp(*ttwo, *tone) == 0)
		break;
	}
	if (!*one)
	    return(1);
    }
    return(0);
}
