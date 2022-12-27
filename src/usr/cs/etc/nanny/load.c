#include "nanny.h"
#define MAXTOK 132

/************************
 * HISTORY
 * 18-Aug-87  Doug Philips (dwp) at Carnegie-Mellon University
 *	Changed to use latest of either mtime or ctime to tell if there have
 *	been changes to a file.
 *
 * 06-Nov-86  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Rewrote the parsing to take of advantage of fgetc() which
 *	effectively does that buffer shuffling and fgets that was
 *	happening before. Also re-wrote the processing of groups
 *	and user ids so everything is done before any process is
 *	declared dead.
 *
 */

/**** sallocn:
 * Save the first num characters from the given string. Add a terminating
 * null.
 */
char *sallocn(src, num)
register char *src;
register int num;
{
    register char *ptr;

    if (num <= 0 || src == NULL)
	return(NULL);
    if ((ptr = malloc((unsigned)num + 1)) == NULL)
	quit(1, "Unable to allocate %d bytes\n", num + 1);
    bcopy(src, ptr, num);
    ptr[num] = '\0';
    return(ptr);
}

enum {
RUNFILE,
SCRIPT,
LOGFILE,
UID,
GID,
ATTRIBUTES,
NICE
};

char *label_tab[] = {
"runfile",
"script",
"logfile",
"uid",
"gid",
"attributes",
"nice",
0
};

static FILE *fp;			    /* module file pointer */

/**** get_name:
 *   Pull a server name from the open file and allocate space to hold
 * the name. Server name is followed by a curly bracket, a set of fields
 * and a closing curly bracket.
 */
char *get_name()
{
    static char thename[MAXTOK];
    register int ch;
    register char *ptr;
    char *name;

    /* ignore leading white space */
    while ((ch = fgetc(fp)) != EOF && isspace(ch));
    if (ch == EOF) 
	return (NULL);	/* no more data */

    /* build the name */
    ptr = thename;
    for(;;) {
	if (ptr < &thename[sizeof(thename)-1])
	    *ptr++ = ch;
	*ptr = '\0';
	ch = fgetc(fp);
	if (ch == EOF || ch == '{' || isspace(ch)) 
	    break;
    }

    /* eat up the trailing whitespace */
    while (ch != EOF && ch != '{' && isspace(ch))
	ch = fgetc(fp);

    /* make sure we have a body */
    if (ch != '{') {
	print_err("get_name did not find body for name '%s' got %o\n",thename,ch);
	return(NULL); /* no more input */
    }

    name = sallocn(thename, ptr - thename);

    return(name);
}				/* get_name */

/**** get_label:
 * Pull a label from the open file. Do not allocate anything!!
 * A label exists on one label name, equal sign and parenthesis
 * value.
 */
char *get_label()
{
    static char label[MAXTOK];	/* too big, but so what */
    register int ch;
    register char *ptr;

    for(;;) {
	/* eat leading white space */
	while ((ch = fgetc(fp)) != EOF && isspace(ch));

	/* if we have a curly bracket return null */
	if (ch == '}' || ch == EOF) 
	    return(NULL);

	/* build the label name */
	ptr = label;
	for(;;) {
	    if (ptr < &label[sizeof(label)-1])
		*ptr++ = ch;
	    *ptr = '\0';
	    ch = fgetc(fp);
	    if (ch == EOF || ch == '=' || isspace(ch) && ch != '\n')
		break;
	}

	/* eat up the trailing whitespace */
	while (ch != EOF && ch != '=' && isspace(ch) && ch != '\n')
	    ch = fgetc(fp);

	/* make sure we have a = */
	if (ch != '=') {
	    print_err("get_label did not find body for name '%s' - ignoring line\n",label);
	    while (ch != '\n' && ch != EOF)
		ch = fgetc(fp);
	}
	else
	    return(label);
    }
}

/**** get_field:
 *   Pick out the field between unescaped parenthisis minus the 
 * parenthisis. The parenthisis are manditory. Do not allocate any
 * space for the field!!
 */
char *get_field()
{
    static char field[BUFSIZ];
    register int ch;
    register char *ptr;

    /* init */
    bzero(field,sizeof(field));

    /* eat leading white space */
    while ((ch = fgetc(fp)) != EOF && isspace(ch));

    /* skip over leading '(' */
    if (ch != '(') {
	print_err("get_field did not find ( but found %o - eating line\n",ch);
	while (ch != EOF && ch != '\n') 
	    ch = fgetc(fp);
	return(NULL);	
    }
    
    /* build the field */
    ptr = field;
    for(;;) {
	ch = fgetc(fp);
	if (ch == EOF || ch == '\n')
	    break;
	if (ch == '\\')
	    ch = fgetc(fp);
	else if (ch == ')')
	    break;
	if (ptr < &field[sizeof(field)-1])
	    *ptr++ = ch;
    }

    /* eat up the trailing whitespace */
    while (ch != EOF && ch != '\n' && isspace(ch))
	ch = fgetc(fp);

    /* make sure we have a ) */
    if (ch != ')') {
	print_err("get_field did not end for field '%s' got %o\n",field,ch);
	while (ch != EOF && ch != '\n') 
	    ch = fgetc(fp);
	return(NULL);
    }

    /* eat trailing white space */
    while (ch != EOF && ch != '\n') 
	ch = fgetc(fp);

    return(field);
}

/**** field_prs:
 *  The given string is taken and parsed as follows:
 *        
 *       - The string is assumed to be NULL terminated.
 *       - All fields MUST be seperated by the field seperator. They
 *         will be taken literally as seperated. 
 *       - The first charactor is taken as the field seperator. 
 *         Though, the last charactor need not be the seperator.
 *         Thus, ":zip:and:zap" is acceptable as is ":zip:and:zap:"
 *       - Empty fields will be ignored. That is, if the seperator 
 *         is "/", then the string "/hi//there/big (fat)////guy/" 
 *         will be broken into "hi" "there" "big (fat)" "guy". 
 *       - Once a seperator is selected, it can not appear in the rest 
 *         of the script except as a seperator. There is no escape 
 *         mechanism.
 *
 *   Contiginous memory will be allocated to hold the parsed fields. Thus,
 * freeing the space for the first field will free the entire group.
 *   A null terminated list of pointers will be allocated and returned. 
 * The original string will remain untouched.
 */
char **field_prs(str)
char *str;
{
    int cnt;
    char delim, *ptr, **list, *begptr;

    /* count the fields first */
    ptr = str;
    delim = *ptr++;
    cnt = 0;
    for(;;) {
	/* ignore leading delim */
	while (*ptr && delim == *ptr) ptr++;
	/* count it */
	if (*ptr == '\0') break;
	cnt++;
	while (*ptr && delim != *ptr) ptr++;
    }
    
    /* build the list */
    list = (char **) malloc((unsigned)sizeof(char *) * (cnt + 1));
    if (list == NULL) {
	print_err("memory allocation failed for field_prs - %s\n",
		  errmsg(errno));
	return(NULL);
    }

    /* add the fields */
    ptr = str;
    cnt = 0;
    for(;;) {
	/* ignore leading delim */
	while (*ptr && delim == *ptr) ptr++;
	/* find end of part */
	if (*ptr == '\0') break;
	begptr = ptr;
	while (*ptr && delim != *ptr) ptr++;
	list[cnt] = sallocn(begptr, ptr-begptr);
	cnt++;
    }
    list[cnt] = NULL;

    return (list);
}

/***** load:
 *   The configuration file format is human-readable. The functions in
 * this file are intended to parse the new format. The format is as follows:
 *
 *         server-name {
 *           RUNFILE=( file_name)
 *           SCRIPT=( script)
 *           UID=( user_id)
 *           GID=( group_id)
 *           ATTRIBUTES=( attribute_list)
 *           NICE=( nice)
 *         }
 *
 *   Leading and trailing white space is ignored except as a seperator.
 * A semi-colon ";" as the first charactor on a line denotes a comment. 
 * Fields within parenthisis are taken literally. If parenthisis are 
 * used within the field, they must be escaped via a preceding back-
 * slash "\". The back-slash will be removed from the field only if 
 * followed by a "(" or ")". To get "\(" or "\)" in a field use "\\(" 
 * or "\\)" respectively.
 *   Any parsing oddities for each field are covered in the parsing 
 * routine used for that field. For now, they are all using the same parsing 
 * routine.
 */
SERVER *load(file,tim)
char *file;
long *tim;
{
    SERVER *ListHead, *srv;
    char *tmp, **list, *fptr;
    int num;

    ListHead = NULL;

    /* open the configuration file */
    if ((fp = fopen(file, "r")) == NULL)
	return(NULL);

    /* get the modification time of the file */
    /* This used to do a fstat, but rather than duplicate the code
       that is in last_mod, we'll just call it from here...
     */
    *tim = last_mod(file);

    /* parse the file....name { ... } first */
    while (tmp = get_name()) {
	srv = NewSrv();
	srv->basis = NewBasis();
	srv->basis->name = tmp;
	/* parse a label = field */
	while (tmp = get_label()) {
	    folddown(tmp, tmp);
	    num = stablk(tmp, label_tab, 1);
	    fptr = get_field();
	    if (fptr == NULL) {
		print_err("no field for label %s\n",tmp);
		continue;
	    }
	    list = field_prs(fptr);
	    if (list == NULL) 
		continue;
	    switch(num) {
		case RUNFILE: 
		    srv->basis->runfile = list;
		    break;
		case SCRIPT: 
		    srv->basis->script = list;
		    break;
		case UID: 
		    srv->basis->uid = list;
		    break;
		case GID: 
		    srv->basis->gid = list;
		    break;
		case ATTRIBUTES: 
		    srv->basis->attributes = list;
		    break;
		case NICE: 
		    srv->basis->nice = atoi(*list);
		    free_list(list);
		    break;
		default: 
		    print_err("Unknown label %s in config file, ignoring\n", tmp);
		    free_list(list);
		    break;
	    }
	    free(tmp);
	}
	/* convert strings to numbers if neccessary and other things */
	ProcessNew(srv);
	QueueTail(&ListHead, srv);
    }

    /* all done */
    (void)fclose(fp);

    return(ListHead);
}
 
/**** ProcessNew:
 *   Process the given srv basis and fill in the other fields.
 */
ProcessNew(srv)
SERVER *srv;
{
    char  **ctmp;
    int    cnt;
    int	glist[NGROUPS+1];
    struct passwd *pwnam;
    struct group *grnam;

    srv->tim = time((long *)NULL);

    print_log("name= %s\n", srv->basis->name);
    for (ctmp = srv->basis->runfile; *ctmp; ctmp++)
	print_log("\trunfile= %s\n", *ctmp);
    for (ctmp = srv->basis->script; *ctmp; ctmp++)
	print_log("\tscript= %s\n", *ctmp);
    for (ctmp = srv->basis->uid; *ctmp; ctmp++)
	print_log("\tuid= %s\n", *ctmp);
    for (ctmp = srv->basis->gid; *ctmp; ctmp++)
	print_log("\tgid= %s\n", *ctmp);
    for (ctmp = srv->basis->attributes; *ctmp; ctmp++)
	print_log("\tattributes= %s\n", *ctmp);
    srv->valid = valid_srv(srv->basis->attributes);
    print_log("\tattributes indicate server is %s\n",
	      srv->valid ? "valid" : "invalid");
 /* UID */
    pwnam = getpwnam(*srv->basis->uid);
    if (pwnam == NULL) {
	print_log("Invalidating %s. Bad UID %s\n", srv->basis->name, *srv->basis->uid);
	srv->uid = -1;
	srv->valid = 0;
    }
    else
	srv->uid = pwnam->pw_uid;
 /* GIDs */
    cnt = 0;
    for (ctmp = srv->basis->gid; *ctmp; ctmp++) {
	grnam = getgrnam(*ctmp);
	if (grnam == NULL) {
	    print_log("Bad group %s for %s - ignoring\n", *ctmp, srv->basis->name);
	}
	else if (cnt < NGROUPS)
	    glist[cnt++] = grnam->gr_gid;
    }
    if (cnt == 0){
	print_log("Invalidating %s. No valid groups.\n", srv->basis->name);
	srv->valid = 0;
    }
    glist[cnt++] = -1;
    srv->gid = (int *) malloc(sizeof(glist));
    if (srv->gid == NULL) {
	print_log("memory allocation failed for proc - %s\n",
		  errmsg(errno));
    }

    bcopy((char *)glist,(char *)srv->gid,sizeof(glist));

}

/**** valid_srv:
 *   See if the current host has all of the required attributes. Return
 * 1 if yes, 0 otherwise.
 */
valid_srv(list)
char **list;
{
    char  **attr;

    if (!attrib)
	print_err("No attribute list for this host!!\n");
    if (!list)
	return(1);

    for (; *list; list++) {
	for (attr = attrib; *attr; attr++) {
	    if (strcmp(*attr, *list) == 0) {
		break;
	    }
	}
	if (!*attr) {
	    return(0);
	}
    }
    return(1);
}

SHORTSRV *NewBasis()
{
    SHORTSRV *basis;

    if ((basis =(SHORTSRV *) malloc(sizeof(SHORTSRV))) == NULL)
	quit(1, "nanny: Unable to allocate %d bytes of memory - Aborting\n", sizeof(SHORTSRV));
    bzero((char *)basis,sizeof(SHORTSRV));
    return(basis);
}

/**** NewSrv:
 *   The needed memory for a server structure is allocated and a
 * pointer to that structure is returned. 
 */
SERVER *NewSrv()
{
    SERVER * srv;

    srv = (SERVER *) malloc(sizeof(SERVER));
    if (srv == NULL) {
	print_err("memory allocation failed for mk_srv - %s\n",
		  errmsg(errno));
	return(NULL);
    }
    bzero((char *)srv,sizeof(SERVER));
    srv->status = DEAD;
    srv->pid = -1;
    srv->restarts = -1;
    srv->uid = -1;
    srv->gid = NULL;
    srv->basis = NewBasis();
    return(srv);
}

