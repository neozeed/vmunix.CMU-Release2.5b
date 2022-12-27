/*
 * data structures used to hold project description
 *
 **********************************************************************
 * HISTORY
 * $Log:	project.h,v $
 * Revision 2.3  89/07/04  17:42:35  gm0w
 * 	Change HASH* to PROJ_HASH* to avoid potential naming conflicts.
 * 	[89/07/04            gm0w]
 * 
 * Revision 2.2  89/01/28  23:04:28  gm0w
 * 	Created.
 * 	[89/01/28            gm0w]
 * 
 **********************************************************************
 */

struct arg_list {
    struct arg_list *next;
    char **tokens;
    int ntokens;
    int maxtokens;
};

struct field {
    struct field *next;		/* next field */
    char *name;			/* name of this field */
    struct arg_list *args;	/* args for this field */
};

#define PROJ_HASHBITS 6		/* bits used in hash function */
#define PROJ_HASHSIZE (1<<PROJ_HASHBITS) /* range of hash function */
#define PROJ_HASHMASK (PROJ_HASHSIZE-1)	/* mask for hash function */
#define PROJ_HASH(i) ((i)&PROJ_HASHMASK)

struct hashent {
    struct hashent *next;
    struct field *field;
};

struct project {
    struct hashent *hashtable[PROJ_HASHSIZE];
    struct field *list;
    struct field *last;
};
