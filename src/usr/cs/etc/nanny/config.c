/*
 * config - configuration routines
 **********************************************************************
 * HISTORY
 * 11-Nov-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed to always have at least a "unix" attribute.
 *
 **********************************************************************
 */
#include "nanny.h"

/**** last_mod:
 *   Return the last mod time for the given file. Full path names are
 * recomended.  Note:  This is not always JUST the mtime!
 */
long last_mod(file)
char *file;
{
    struct stat buf;

    if (stat(file, &buf) < 0) {
	print_err("Unable to get status of file %s: %s\n", file, errmsg(-1));
	return(0);
    }
    if (buf.st_ctime > buf.st_mtime)
	return(buf.st_ctime);
    else
	return(buf.st_mtime);
}

Check_cnfg(ptr)
char *ptr;
{
    static long lastmod = 0L;
    long    mod;
    int doload;
    char  **attr;
    SERVER *NewList, *newsrv, *oldsrv;

    if (verbose)
	print_log("Checking configuration file %s\n",
		  ptr == NULL ? "" : ptr);

    /* assume we don't have to load */
    doload = 0;

    if (config == NULL) {
	if (ptr == NULL) {
	    print_err("No configuration file specified\n");
	    return;
	}
	/* first configuration file specified */
	print_log("Initial load from configuration file %s\n", ptr);
	config = salloc(ptr);
	mod = last_mod(ptr);
	doload++;
    }
    else if (ptr == NULL) {
	/* just check the configuration file */
	mod = last_mod(config);
	if (mod != 0 && mod != lastmod) {
	    /* it has changed */
	    print_log("Configuration file has been altered\n");
	    doload++;
	}
    }
    else if (strcmp(ptr, config) != 0) {
	print_log("Configuration file has been changed from %s to %s\n", 
		  config, ptr);
	free(config);
	config = salloc(ptr);
	mod = last_mod(ptr);
	doload++;
    }

    /* check host attributes */

    attr = get_attr();
    if (attrib == NULL) {
	/* never set attributes */
	print_log("Initial attribute load\n");
	attrib = attr;
	doload++;
    }
    else if (listcmp(attr, attrib)) {
	free_list(attrib);
	attrib = attr;
	print_log("Host attributes have changed\n");
	doload++;
    }
    else {
	/* no changes in the attributes */
	free_list(attr);
    }

    /* if we don't need a load then don't */
    if (!doload) return;

    /* do the load or reload */
    NewList = load(config,&lastmod);
    if (NewList == NULL) return;

    /* invalidate all old servers that no longer exist */
    if ((oldsrv = SrvHead) != NULL)
	do {
	    if (!oldsrv->valid)
		continue;
	    oldsrv->valid = 0;
	    if ((newsrv = NewList) != NULL)
		do {
		    if (strcmp(oldsrv->basis->name, newsrv->basis->name) == 0) {
			oldsrv->valid = 1;
			break;
		    }
		} while ((newsrv = newsrv->Next) != NewList);
	} while ((oldsrv = oldsrv->Next) != SrvHead);

    /* add the current server list */
    while (NewList != NULL) {
	newsrv = (SERVER *) DeQHead(&NewList);
	oldsrv = fetch_srv(newsrv->basis->name);
	if (oldsrv != NULL) {
	    if (servercmp(newsrv,oldsrv)) {
		kill_srv(oldsrv);
		oldsrv->valid = 0;
		QueueTail(&SrvHead,newsrv);
		print_log("server %s is an update\n",
			  newsrv->basis->name);
	    }
	    else {
		/* same version of server */
		FreeSrv(newsrv);
		if (verbose)
		    print_log("server %s is a duplicate\n",
			      oldsrv->basis->name);
	    }
	}
	else {
	    /* totally new server -- just add it*/
	    QueueTail(&SrvHead,newsrv);
	    print_log("server %s is new\n",newsrv->basis->name);
	}
    }
}

servercmp(srv1,srv2)
SERVER *srv1,*srv2;
{
    if (strcmp(srv1->basis->name,srv2->basis->name)) 
	return(1);
    if (listcmp(srv1->basis->runfile,srv2->basis->runfile))
	return(1);
    if (listcmp(srv1->basis->script,srv2->basis->script))
	return(1);
    if (listcmp(srv1->basis->uid,srv2->basis->uid))
	return(1);
    if (listcmp(srv1->basis->gid,srv2->basis->gid))
	return(1);
    if (listcmp(srv1->basis->attributes,srv2->basis->attributes))
	return(1);
    if (srv1->basis->nice != srv2->basis->nice)
	return(1);
    if (srv1->valid != srv2->valid)
	return(1);
    return(0);
}

/*
 * compare two lists of strings -- we assume they are sorted
 */
listcmp(lst1,lst2)
char **lst1,**lst2;
{
    while (*lst1 != NULL && *lst2 != NULL)
	if (strcmp(*lst1++,*lst2++) != 0)
	    return 1;
    if (*lst1 != NULL || *lst2 != NULL)
	return 1;
    return 0;
}


/**** FreeSrv:
 *   The server pointed to by the argument is removed from memory. Note that 
 * NOTHING is done with the links. These must be rearranged before the call 
 * to this function.
 */
FreeSrv(srv)
SERVER *srv;
{
    if (srv->basis) 
        FreeBasis(srv->basis);
    srv->basis = NULL;
    if (srv->gid)
	free((char *)srv->gid);
    srv->gid = NULL;
    free((char *)srv);
}

/**** FreeBasis:
 *   free the components of the basis, then the basis itself.
 */
FreeBasis(base)
SHORTSRV *base;
{
    if (base->name)
	free(base->name);
    base->name = NULL;
    if (base->runfile)
	free_list(base->runfile);
    base->runfile = NULL;
    if (base->script)
	free_list(base->script);
    base->script = NULL;
    if (base->uid)
	free_list(base->uid);
    base->uid = NULL;
    if (base->gid)
	free_list(base->gid);
    base->gid = NULL;
    if (base->attributes)
	free_list(base->attributes);
    base->attributes = NULL;
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
 
/*
 * set up to return an attribute list that can be freed by
 * free_list
 */
#define ATTRIB_LEN 512

char **get_attr()
{
    char *vec[ATTRIB_LEN];
    int len,i;
    char **ptr;

    bzero((char *)vec,sizeof(vec));

    if ((len = gethostattr(vec, ATTRIB_LEN-1)) < 0) {
	print_err("Unable to gather host attributes len=%d err=%d\n", len, errno);
	vec[0] = "unix";
	len = 1;
    } else if (len >= ATTRIB_LEN) {
	print_err("Too many attributes\n");
	vec[0] = "unix";
	len = 1;
    }

    /* copy it so we can do a free_list later */
    ptr = (char **) malloc((unsigned)sizeof(char *) * (len+1));
    if (ptr == NULL) {
	print_err("Can not allocate memory for attribute array\n");
	return(NULL);
    }

    /* copy the individual attributes too */
    for(i=0;i<len;i++)
	ptr[i] = salloc(vec[i]);
    ptr[len] = 0;

    /* all done */
    return(ptr);
}

/**** fetch_srv:
 *   Look up the server with the given name, null if not found.
 */
SERVER *fetch_srv(name)
char *name;
{
    SERVER *srv;

    if ((srv = SrvHead) == NULL) 
	return NULL;

    do {
	if (strcmp(srv->basis->name, name) == 0)
	    return(srv);
    } while ((srv = srv->Next) != SrvHead);

    return(NULL);
}

