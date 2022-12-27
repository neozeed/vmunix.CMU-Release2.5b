/*
 * routines to manage project description databases
 *
 **********************************************************************
 * HISTORY
 * $Log:	project.c,v $
 * Revision 2.3  89/02/19  11:18:09  gm0w
 * 	Use "project" field of project description for project name.
 * 	[89/02/19            gm0w]
 * 
 * Revision 2.2  89/01/28  23:04:36  gm0w
 * 	Created.
 * 	[88/11/27            gm0w]
 * 
 **********************************************************************
 */

#include <sys/param.h>
#include <stdio.h>
#include <libc.h>
#include <c.h>
#include "project.h"

char *prog;

struct project proj;
char *project;

char **args;
int nargs;

main(argc, argv)
int argc;
char **argv;
{
    /*
     * parse command line
     */
    parse_command_line(argc, argv);

    /*
     * read project description
     */
    if (parse_project(project, &proj) != 0)
	quit(1, "%s: unable to parse %s project description\n", prog, project);

    /*
     * print project description
     */
    print_project();

    /*
     * return with good status
     */
    exit(0);
}

parse_command_line(argc, argv)
int argc;
char **argv;
{
    if (argc > 0) {
	if ((prog = rindex(argv[0], '/')) != NULL)
	    prog++;
	else
	    prog = argv[0];
	argc--; argv++;
    } else
	prog = "project";

    while (argc > 0) {
	if (argv[0][0] != '-')
	    break;
	switch (argv[0][1]) {
	case 'p':
	    if (strcmp(argv[0], "-project") == 0) {
		if (argc == 1)
		    quit(1, "%s: missing argument to %s\n", prog, argv[0]);
		argc--; argv++;
		project = argv[0];
	    } else
		usage(argv[0]);
	    break;
	default:
	    usage(argv[0]);
	}
	argc--; argv++;
    }

    args = argv;
    nargs = argc;
}

usage(opt)
char *opt;
{
    if (opt != NULL)
	fprintf(stderr, "%s: unknown switch %s\n", prog, opt);
    quit(1, "usage: %s [ -project <project> ] [ fields ... ]\n", prog);
}

print_project()
{
    char *indent;
    struct field *field_p;
    struct arg_list *args_p;
    int all = (nargs == 0);
    char *spaces = "            ";
    int i;

    indent = spaces + strlen(spaces);
    if (all) {
	if (project == NULL && (project = getenv("PROJECT")) == NULL)
	    project = "default";
	if (project_field(&proj, "project", &field_p) != 0)
	    printf("%s:\n", project);
	else {
	    args_p = field_p->args;
	    if (args_p->ntokens != 1)
		quit(1, "%s: improper project\n", prog);
	    if (strcmp(project, args_p->tokens[0]) == 0)
		printf("%s:\n", project);
	    else
		printf("%s: (%s)\n", args_p->tokens[0], project);
	}
	indent -= 4;
    }
    for (field_p = proj.list; field_p != NULL; field_p = field_p->next) {
	if (all) {
	    print_field(field_p, indent, all);
	    continue;
	}
	for (i = 0; i < nargs; i++)
	    if (strcmp(args[i], field_p->name) == 0)
		break;
	if (i == nargs)
	    continue;
	print_field(field_p, indent, all);
    }
}

print_field(field_p, indent, all)
struct field *field_p;
char *indent;
int all;
{
    struct arg_list *args_p;
    int i;

    if ((args_p = field_p->args)->next == NULL) {
	if (args_p->ntokens == 0) {
	    printf("%s%s\n", indent, field_p->name);
	    return;
	}
	printf("%s%s%s%s", indent,
	       all ? field_p->name : "", all ? ": " : "",
	       args_p->tokens[0]);
	for (i = 1; i < args_p->ntokens; i++)
	    printf(" %s", args_p->tokens[i]);
	(void) putchar('\n');
	return;
    }
    if (all) {
	printf("%s%s:\n", indent, field_p->name);
	indent -= 4;
    }
    do {
	if (args_p->ntokens == 0) {
	    printf("%s%s\n", indent, field_p->name);
	    continue;
	}
	printf("%s%s", indent, args_p->tokens[0]);
	for (i = 1; i < args_p->ntokens; i++)
	    printf(" %s", args_p->tokens[i]);
	(void) putchar('\n');
	args_p = args_p->next;
    } while (args_p != NULL);
}
