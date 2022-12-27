#include "nanny.h"

/************************
 * HISTORY
 ************************/

/**** strnsav:
 *   Save the first num charactors from the given string. Add a terminating
 * null.
 */
char *strnsav(dest, src, num)
char **dest, *src;
int num;
{
    char *ptr;

    if (num <= 0 || !src)
	return(NULL);
    if ((ptr = *dest =(char *) malloc((unsigned)num + 1)) == NULL)
	quit(1, "Unable to allocate %d bytes\n", num + 1);
    while (num--)
	*ptr++ = *src++;
    *ptr = 0;
    return(*dest);
}/* strnsav */

enum {
RUNFILE,
SCRIPT,
UID,
GID,
ATTRIBUTES,
NICE
};

char *field_tab[] = {
"runfile",
"script",
"uid",
"gid",
"attributes",
"nice",
0
};

char buffer[DEV_BSIZE];			    /* global buffer */
FILE *fp;				    /* Global file pointer */
extern FILE *open_lock();

/**** get_name:
 *   Pull a server name from the open file and allocate space to hold
 * the name. We will use the universal buffer buffer to hold any unused
 * portions of the file.
 */
char *get_name()
{
    char   *ptr,
           *tmp,
           *name;

    if (buffer[0] == 0)			    /* empty buffer? */
	if (!fgets(buffer, DEV_BSIZE, fp))
	    return(NULL);

    for (;;) {
	ptr = buffer;
	while (*ptr && isspace(*ptr))
	    ptr++;
	if (*ptr)
	    break;
	if (!fgets(buffer, DEV_BSIZE, fp))
	    return(NULL);
    }
    tmp = ptr;
    while (*tmp && *tmp != '{' && !isspace(*tmp))
	tmp++;
    (void)strnsav(&name, ptr, tmp - ptr);
    while (*tmp != '{') {		    /* have name, need bracket */
	while (*tmp && *tmp != '{')
	    tmp++;
	if (!*tmp) {
	    if (!fgets(buffer, DEV_BSIZE, fp))
		return(NULL);
	    tmp = buffer;
	}
    }
    tmp++;
    ptr = buffer;
    while (*ptr++ = *tmp++);		    /* shuffle buffer */
    return(name);
}				/* get_name */

/**** get_label:
 *  Pull a label from the open file. Do not allocate anything!!
 */
char *get_label()
{
    static char label[100];	/* too big, but so what */
    char   *ptr,
           *tmp;

    if (buffer[0] == 0)			    /* empty buffer? */
	if (!fgets(buffer, DEV_BSIZE, fp))
	    return(NULL);

    for (;;) {				    /* skip white space */
	ptr = buffer;
	while (*ptr && isspace(*ptr))
	    ptr++;
	if (*ptr)
	    break;
	if (!fgets(buffer, DEV_BSIZE, fp))
	    return(NULL);
    }
    tmp = ptr;
    while (*tmp && *tmp != '=' && !isspace(*tmp))
	tmp++;
    (void)strncpy(label, ptr, tmp - ptr);
    label[tmp - ptr] = 0;
    if (label[0] == '}') {
	ptr = buffer;
	while (*ptr++ = *tmp++);	    /* shuffle buffer */
	return(NULL);
    }
    else {
	while (*tmp != '=') {		    /* look for the = */
	    while (*tmp && *tmp != '=')
		tmp++;
	    if (!*tmp) {
		if (!fgets(buffer, DEV_BSIZE, fp))
		    return(NULL);
		tmp = buffer;
	    }
	}
	tmp++;
    }
    ptr = buffer;
    while (*ptr++ = *tmp++);		    /* shuffle buffer */
    return(label);
}				/* get_label */

/**** get_field:
 *   Pick out the field between unescaped parenthisis minus the 
 * parenthisis. The parenthisis are manditory. Do not allocate any
 * space for the field!!
 */
char *get_field()
{
    static char field[DEV_BSIZE];/* Again too big and too bad */
    char   *ptr,
           *tmp;

    if (buffer[0] == 0)			    /* empty buffer? */
	if (!fgets(buffer, DEV_BSIZE, fp))
	    return(NULL);

    for (;;) {
	ptr = buffer;
	while (*ptr && *ptr != '(' && *ptr != '}')
	    ptr++;
	if (*ptr == '}')
	    return(NULL);
	if (*ptr)
	    break;
	if (!fgets(buffer, DEV_BSIZE, fp))
	    return(NULL);
    }
    ptr++;
    tmp = buffer;
    while (*tmp++ = *ptr++);		    /* Shuffle */
    tmp = ptr = buffer;
    while (*tmp) {
	while (*tmp && *tmp != ')')
	    tmp++;
	if (*tmp == ')')
	    if (*(tmp - 1) != '\\')
		break;
	    else
		tmp++;
	if (!*tmp) {			    /* Need more !! */
	    if (!fgets(tmp, DEV_BSIZE -(tmp - ptr), fp))
		return(NULL);
	}
    }
    *tmp++ = 0;
    (void)strcpy(field, buffer);	    /* Copy to safe place */
    while (*ptr++ = *tmp++);		    /* shuffle buffer */
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
    char          **list = NULL,
                  **mark;
    char           *ptr,
                   *front,
                   *back;
    unsigned        num = 0;

    if (!(ptr = str))
	return (NULL);

    while (*ptr) {			    /* Count the number of fields */
	if (*ptr++ == *str && *ptr && *ptr != *str)
	    num++;
    }

    mark = list = (char **) Malloc(sizeof(char *) * (num + 1));

    front = str;
    while (*front) {
	while (*front && *front == *str)
	    front++;			    /* skip extra seperators */
	back = front;
	while (*back && *back != *str)
	    back++;
	(void)strnsav(mark, front, back - front);
	mark++;
	front = back;
    }
    *mark = NULL;
    return (list);
}

/***** new_load:
 *   The new configuration file format is human-readable. The functions in
 * this file are intended to parse the new format. The format is as follows:
 * 
 *         ; A semi colon as the first charactor on a line denotes a comment.
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
SERVER *new_load(file)
char *file;
{
    SERVER * first = NULL,
	*srv,
	*mk_srv(),
	*add_srv();
    SHORTSRV * mk_basis();
    char   *tmp,
          **list,
	  **get_attr();
    int     num;

    if ((fp = open_lock(file, "r+")) == NULL)
	return(NULL);

    attrib = get_attr();
    buffer[0] = 0;
    while (tmp = get_name()) {
	srv = mk_srv();
	srv->basis = mk_basis();
	srv->basis->name = tmp;
	while (tmp = get_label()) {
	    folddown(tmp, tmp);
	    num = stablk(tmp, field_tab, 1);
	    list = field_prs(get_field());
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
	proc(srv);
	first = add_srv(first, srv);
    }
    unlock(fileno(fp));
    (void)fclose(fp);
    return(first);
}
 
/**** proc:
 *   Process the given srv basis and fill in the other fields.
 */
proc(srv)
SERVER *srv;
{
    struct passwd  *pwnam,
                   *getpwnam();
    struct group   *grnam,
                   *getgrnam();
    char  **ctmp;
    int    *itmp;
    unsigned    cnt;

    if (!srv)
	return;
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
 /* UID */
    pwnam = getpwnam(*srv->basis->uid);
    endpwent();
    if (pwnam == NULL) {
	print_log("Invalidating %s. Bad UID %s\n", srv->basis->name, *srv->basis->uid);

	srv->uid = -1;
	srv->valid = 0;
	return;
    }
    else
	srv->uid = pwnam->pw_uid;
 /* GIDs */
    for (cnt = 1, ctmp = srv->basis->gid; *ctmp; cnt++, ctmp++);
    itmp = srv->gid = (int *) Malloc(cnt * sizeof(int));
    for (ctmp = srv->basis->gid, *itmp = -1; *ctmp; ctmp++) {
	grnam = getgrnam(*ctmp);
	(void)endgrent();
	if (grnam == NULL) {
	    print_log("Invalidating %s. Bad group %s\n", srv->basis->name, *ctmp);
	    srv->valid = 0;
	    return;
	}
	*itmp++ = grnam->gr_gid;
	*itmp = -1;
    }

    (void)time(&srv->tim);

    srv->valid = valid_srv(srv->basis->attributes);
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
