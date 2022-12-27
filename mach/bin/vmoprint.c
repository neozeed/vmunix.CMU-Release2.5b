
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	vmoprint.c,v $
 * Revision 1.2  89/05/05  18:26:45  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 * 16-Jul-87  Mary Thompson (mrt) at Carnegie Mellon
 *	Removed printing of vm_object.paging_space in vm_object_print
 *	as it is  no longer exported by vm_object.
 *
 * 18-May-87  Mary Thompson (mrt) at Carnegie Mellon
 *	Added definition of first_page and vm_page_array
 *
 * 18-Mar-87  David Black (dlb) at Carnegie-Mellon University
 *	Bug Fix: changed reference to vm_object_list to use queue_first.
 *
 *  7-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Print out new informational field (resident count) and handle
 *	copy objects.
 *
 * 10-Oct-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Make this victim of software rot work again.   Also
 *	reformatted quite a bit to look reasonable.
 *
 * 14-Apr-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	vmoprint.c
 *
 *	utility for printing out virtual memory objects
 */


#include <nlist.h>
#include <stdio.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <sys/queue.h>


#define KMEMFILE	"/dev/kmem"
#define NLISTFILE	"/vmunix"
#define NAMETBLSIZE 	5
#define VM_PAGE_TO_PHYS(entry)	((entry)->phys_addr)

char	*names[NAMETBLSIZE] = {
	"_first_page",
#define X_FIRST_PAGE		0
	"_page_shift",
#define X_PAGE_SHIFT		1
	"_vm_object_list",
#define X_VM_OBJECT_LIST	2
	"_vm_page_array",
#define X_VM_PAGE_ARRAY		3
	""
};

int		indent;		/* how far to indent in iprintf */
int		page_shift;	/* used by VM_PAGE_TO_PHYS */
queue_head_t	vm_object_list;
vm_page_t	vm_page_array;		/* First resident page in table */
long		first_page;		/* first physical page number */
					/* ... represented in vm_page_array */

/*
 *    Initialize list of symbols to lookup
 */
initnlist(nl, names)
	struct nlist	nl[];
	char		*names[];
{
	int	i;

	for (i = 0; names[i] != 0 && strcmp(names[i], "") != 0; i++)
		nl[i].n_name = names[i];
}

/*
 *
 * Abstract:
 *    Get needed kernel variables
 *
 * Parameters:
 *    kmem -  file descriptor for /dev/kmem
 *    nl -  symbol table (name list)
 *
 */
getkvars(kmem, nl)
	struct nlist	nl[];
{
	lseek(kmem, (long) nl[X_FIRST_PAGE].n_value, 0);
	if (read(kmem, (char *) &first_page, sizeof(first_page)) !=
	    sizeof(first_page))
		printf("getkvars:  can't read first_page at 0x%x\n",
		       nl[X_FIRST_PAGE].n_value);

	lseek(kmem, (long) nl[X_PAGE_SHIFT].n_value, 0);
	if (read(kmem, (char *) &page_shift, sizeof(page_shift)) !=
	    sizeof(page_shift))
		printf("getkvars:  can't read page_shift at 0x%x\n",
		       nl[X_PAGE_SHIFT].n_value);

	lseek(kmem, (long) nl[X_VM_OBJECT_LIST].n_value, 0);
	if (read(kmem, (char *) &vm_object_list, sizeof(vm_object_list)) !=
	    sizeof(vm_object_list))
		printf("getkvars:  can't read vm_object_list at 0x%x\n",
		       nl[X_VM_OBJECT_LIST].n_value);

	lseek(kmem, (long) nl[X_VM_PAGE_ARRAY].n_value, 0);
	if (read(kmem, (char *) &vm_page_array, sizeof(vm_page_array)) !=
	    sizeof(vm_page_array))
		printf("getkvars:  can't read vm_page_array at 0x%x\n",
		       nl[X_VM_PAGE_ARRAY].n_value);

	return (0);
}


/*VARARGS*/
iprintf(a, b, c, d, e, f, g, h)
{
	register int i;

	for (i = indent; i > 0;){
		if (i >= 8) {
			putchar('\t');
			i -= 8;
		}
		else {
			putchar(' ');
			i--;
		}
	}

	printf(a, b, c, d, e, f, g, h);
}


/*
 *	Routine:	vm_object_print [debug]
 */
void vm_object_print(kmem, objectptr, verbose)
	int			kmem;
	vm_object_t		objectptr;
	short			verbose;
{
	struct vm_object	object;
	register vm_page_t	p;
	struct vm_page		vmpage;

#if	DEBUG_LONGFORM
	if (objectptr)
		iprintf("Backing store object 0x%x.\n", objectptr);
	else {
		iprintf("No backing store object.\n");
		return;
	}

	lseek(kmem, (long)objectptr, 0);
	if (read(kmem, (char *)&object, sizeof(object)) != 
	    sizeof(object)){
		printf("vm_object_print: can't read object at 0x%x\n",
			objectptr);
		return;
	}

	indent += 2;
	iprintf("Size = 0x%x(%d).\n", object.size, object.size);
	iprintf("Ref count = %d.\n", object.ref_count);
	iprintf("Pager = 0x%x, pager_info = 0x%x.\n", object.pager,
		object.pager_info);
	iprintf("Memq = (next = 0x%x, prev = 0x%x).\n", object.memq.next,
			object.memq.prev);
	if (verbose){

		iprintf("Resident memory:\n");
		p = (vm_page_t) object.memq.next;
		while (!queue_end(&objectptr->memq, (queue_entry_t) p) {
			indent += 2;
			iprintf("Mem_entry 0x%x (phys = 0x%x).\n", p,
					VM_PAGE_TO_PHYS(p));

			lseek(kmem, (long)p, 0);
			if (read(kmem, (char *)&vmpage, sizeof(vmpage)) != 
			    sizeof(vmpage)){
				printf("vm_object_print: can't read page struct at 0x%x\n", p);
				break;
			}
			p = (vm_page_t) queue_next(&vmpage.listq);

			indent -= 2;
		}

	}
	indent -= 2;
#else	DEBUG_LONGFORM

	register int count;

	if (objectptr == VM_OBJECT_NULL)
		return;

	lseek(kmem, (long)objectptr, 0);
	if (read(kmem, (char *)&object, sizeof(object)) != 
	    sizeof(object)){
		printf("vm_object_print: can't read object at 0x%x\n",
			objectptr);
		return;
	}

	indent += 2;

	iprintf("Obj 0x%x: size=0x%x, in=%d, ref=%d, pager=(%d,0x%x), ",
		(int) objectptr, (int) object.size, object.resident_page_count,
		object.ref_count,
		(int) object.pager,
		(int) object.paging_offset);
	if (object.copy)
		printf("copy=0x%x\n", (int) object.copy);
	else
		printf("shadow=(0x%x)+0x%x\n", (int) object.shadow,
			(int) object.shadow_offset);


	if (verbose){

		indent += 2;
		count = 0;
		p = (vm_page_t) queue_first(&object.memq);
		while (!queue_end(&objectptr->memq, (queue_entry_t) p)) {
			if (count == 0) iprintf("memory:=");
			else if (count == 6) {printf("\n"); iprintf(" ..."); count = 0;}
			else printf(",");
			count++;

			lseek(kmem, (long)p, 0);
			if (read(kmem, (char *)&vmpage, sizeof(vmpage)) != 
			    sizeof(vmpage)){
				printf("vm_object_print: can't read page struct at 0x%x\n", p);
				break;
			}

			printf("(off=0x%x,page=0x%x)",
				 vmpage.offset, VM_PAGE_TO_PHYS(p));
			p = (vm_page_t) queue_next(&vmpage.listq);
		}
		if (count != 0)
			printf("\n");
		indent -= 2;

		vm_object_print(kmem, object.shadow, verbose);

	}
	indent -= 2;
#endif	DEBUG_LONGFORM
	fflush(stdout);
}


main(argc, argv)
	int	argc;
	char	*argv[];
{
	int             kmem;
	char           *kmemfile;
	struct nlist    nl[NAMETBLSIZE];
	char           *nlistfile;
	char           *s;
	struct vm_object object;
	vm_object_t     objectptr;
	short           verbose;

	kmemfile = KMEMFILE;
	nlistfile = NLISTFILE;

	kmem = open(kmemfile, 0);	/* grab kernel memory */
	if (kmem < 0) {
		perror(kmemfile);
		exit(1);
	}
	initnlist(nl, names);	/* lookup kernel variables */
	nlist(nlistfile, nl);
	if (getkvars(kmem, nl) != 0)
		exit(2);

	indent = -2;		/* starts by += 2 */
	verbose = 0;
	if (argc == 1) {
		objectptr = (vm_object_t) queue_first(&vm_object_list);
		if (objectptr)
			while (!queue_end((vm_object_t) (nl[X_VM_OBJECT_LIST].n_value),
					  objectptr)) {
				lseek(kmem, (long) objectptr, 0);
				if (read(kmem, (char *) &object, sizeof(object)) !=
				    sizeof(object))
					printf("vmoprint:  can't read object at 0x%x\n", objectptr);
				vm_object_print(kmem, objectptr, verbose);
				objectptr = (vm_object_t) queue_next(&object.object_list);
			};
	}
	while (--argc > 0) {
		objectptr = 0;
		/* parse command line */

		if ((*++argv)[0] == '-') {
			s = argv[0] + 1;
			switch (*s) {
			case 'a':
				if (*++s)
					sscanf(s, "%x", &objectptr);
				else if (--argc > 0)
					sscanf(*++argv, "%x", &objectptr);
				break;
			case 'v':
				verbose = 1;
				continue;
			default:
				printf("vmoprint:  illegal option %c\n", *s);
				exit(3);
			}
		} else if (((*argv)[0] >= '0' && (*argv)[0] <= '9')) {
			sscanf(*argv, "%x", &objectptr);
			printf("Assuming object address specified as 0x%x\n", objectptr);
		}
		if (objectptr)
			vm_object_print(kmem, objectptr, verbose);
		else {
			objectptr = (vm_object_t)queue_first(&vm_object_list);
			if (objectptr)
				while (!queue_end((vm_object_t) (nl[X_VM_OBJECT_LIST].n_value),
						  objectptr)) {
					lseek(kmem, (long) objectptr, 0);
					if (read(kmem, (char *) &object, sizeof(object)) !=
					    sizeof(object))
						printf("vmoprint:  can't read object at 0x%x\n", objectptr);
					vm_object_print(kmem, objectptr, verbose);
					objectptr = (vm_object_t) queue_next(&object.object_list);
				};
		}
	}
}
