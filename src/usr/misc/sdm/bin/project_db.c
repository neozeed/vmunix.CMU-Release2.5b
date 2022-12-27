/*
 * routines to parse project database into description structure
 *
 **********************************************************************
 * HISTORY
 * $Log:	project_db.c,v $
 * Revision 2.3  89/07/04  17:42:39  gm0w
 * 	Changed HASH() and HASHSIZE to PROJ_HASH() and PROJ_HASHSIZE
 * 	to avoid possible naming conflicts.
 * 	[89/07/04            gm0w]
 * 
 * Revision 2.2  89/01/28  23:04:19  gm0w
 * 	Created.
 * 	[88/11/27            gm0w]
 * 
 **********************************************************************
 */
#include <sys/param.h>
#include <sys/time.h>
#include <stdio.h>
#include <libc.h>
extern char *concat();
#include <c.h>
#include "project.h"

#ifndef STATIC
#define STATIC static
#endif

STATIC
find_field(proj_p, field, field_pp, insertok)
struct project *proj_p;
char *field;
struct field **field_pp;
int insertok;
{
    int i;
    char *p;
    struct hashent **hpp;

    i = 0;
    for (p = field; *p != '\0'; p++)
	i += *p;
    hpp = &proj_p->hashtable[PROJ_HASH(i)];
    while (*hpp != (struct hashent *)NULL) {
	if (strcmp((*hpp)->field->name, field) != 0) {
	    hpp = &((*hpp)->next);
	    continue;
	}
	*field_pp = (*hpp)->field;
	return(0);
    }
    if (!insertok)
	return(1);
    if ((field = salloc(field)) == NULL)
	return(1);
    *field_pp = (struct field *) calloc(sizeof(char), sizeof(struct field));
    if (*field_pp == (struct field *)NULL)
	return(1);
    (*field_pp)->name = field;
    *hpp = (struct hashent *) calloc(sizeof(char), sizeof(struct hashent));
    if (*hpp == (struct hashent *)NULL)
	return(1);
    (*hpp)->field = *field_pp;
    if (proj_p->last == (struct field *)NULL)
	proj_p->list = *field_pp;
    else
	proj_p->last->next = *field_pp;
    proj_p->last = *field_pp;
    return(0);
}

STATIC
free_field(field_p)
struct field *field_p;
{
    struct arg_list *args_p;

    while ((args_p = field_p->args) != NULL) {
	field_p->args = args_p->next;
	free_args(args_p);
    }
    free(field_p->name);
    free((char *)field_p);
}

STATIC
free_args(args_p)
struct arg_list *args_p;
{
    while (--args_p->ntokens >= 0)
	free(args_p->tokens[args_p->ntokens]);
    free((char *)args_p->tokens);
    free((char *)args_p);
}

STATIC
create_arglist(field_p, args_pp)
struct field *field_p;
struct arg_list **args_pp;
{
    struct arg_list **app;

    app = &field_p->args;
    while (*app != (struct arg_list *)NULL)
	    app = &((*app)->next);
    *app = (struct arg_list *) calloc(sizeof(char), sizeof(struct arg_list));
    if (*app == (struct arg_list *)NULL)
	return(1);
    *args_pp = *app;
    return(0);
}

STATIC
append_arg(arg, args_p)
char *arg;
struct arg_list *args_p;
{
    if (args_p->ntokens == args_p->maxtokens) {
	if (args_p->maxtokens == 0) {
	    args_p->maxtokens = 8;
	    args_p->tokens = (char **) malloc((unsigned) args_p->maxtokens *
					      sizeof(char *));
	} else {
	    args_p->maxtokens <<= 1;
	    args_p->tokens = (char **) realloc((char *) args_p->tokens,
					(unsigned) args_p->maxtokens *
					       sizeof(char *));
	}
	if (args_p->tokens == NULL)
	    return(1);
    }
    if ((arg = salloc(arg)) == NULL)
	return(1);
    args_p->tokens[args_p->ntokens++] = arg;
    return(0);
}

STATIC
parse_project_file(projfile, proj_p)
FILE *projfile;
struct project *proj_p;
{
    char buf[BUFSIZ];
    char *p, *field, *arg;
    FILE *ifile;
    struct field *field_p;
    struct arg_list *args_p;

    while (fgets(buf, sizeof(buf)-1, projfile) != NULL) {
	if (index("#\n", *buf) != NULL)
	    continue;
	if ((p = rindex(buf, '\n')) != NULL)
	    *p = '\0';
	p = buf;
	field = nxtarg(&p, " \t");
	if (*field == '\0')
	    continue;
	if (strcmp(field, "include") == 0) {
	    arg = nxtarg(&p, " \t");
	    if (*arg == '\0')
		return(1);
	    if ((ifile = fopen(arg, "r")) == NULL)
		return(1);
	    if (parse_project_file(ifile, proj_p) != 0) {
		(void) fclose(ifile);
		return(1);
	    }
	    (void) fclose(ifile);
	}
	if (find_field(proj_p, field, &field_p, TRUE))
	    return(1);
	if (create_arglist(field_p, &args_p))
	    return(1);
	for (;;) {
	    arg = nxtarg(&p, " \t");
	    if (*arg == '\0')
		break;
	    if (append_arg(arg, args_p))
		return(1);
	}
    }
    return(0);
}

parse_project(name, proj_p)
char *name;
struct project *proj_p;
{
    char *path;
    char projpath[MAXPATHLEN];
    char relpath[MAXPATHLEN];
    FILE *projfile;

    if (name == NULL && (name = getenv("PROJECT")) == NULL)
	name = "default";
    if ((path = getenv("PROJECTPATH")) != NULL)
	(void) strcpy(relpath, name);
    else {
	if ((path = getenv("LPATH")) == NULL)
	    path = "/usr/cs/lib";
	(void) concat(relpath, sizeof(relpath), "project/", name, NULL);
    }
    if ((projfile = fopenp(path, relpath, projpath, "r")) == NULL) {
	fprintf(stderr, "unable to find description of %s project\n", name);
	return(1);
    }
    if (parse_project_file(projfile, proj_p) != 0)
	return(1);
    return(0);
}

project_field(proj_p, field, field_pp)
struct project *proj_p;
char *field;
struct field **field_pp;
{
    return(find_field(proj_p, field, field_pp, FALSE));
}

free_project(proj_p)
struct project *proj_p;
{
    int i;
    struct hashent *hash_p;

    for (i = 0; i < PROJ_HASHSIZE; i++) {
        while ((hash_p = proj_p->hashtable[i]) != NULL) {
            proj_p->hashtable[i] = hash_p->next;
            free_field(hash_p->field);
	    free((char *)hash_p);
        }
    }
}
