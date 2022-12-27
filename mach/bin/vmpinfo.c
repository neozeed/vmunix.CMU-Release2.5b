
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	vmpinfo.c,v $
 * Revision 1.2  89/05/05  18:26:56  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 * 10-Oct-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Made work again after accumlating software rot.
 *
 * 19-Mar-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	vmpinfo.c
 *
 *	contains code to dump virtual memory page table
 */

#include <nlist.h>
#include <stdio.h>
#include <vm/vm_page.h>


#define KMEMFILE	"/dev/kmem"
#define NLISTFILE	"/vmunix"
#define NAMETBLSIZE 	5

char	*names[NAMETBLSIZE] = {
	"_first_page",
#define X_FIRST_PAGE	0
	"_mem_size",
#define X_MEM_SIZE	1
	"_page_size",
#define X_PAGE_SIZE	2
	"_vm_page_array",
#define	X_VM_PAGE_ARRAY	3
	""
};


initnlist(nl, names)
	struct nlist	nl[];
	char		*names[];
{
	int	i;

	for (i = 0; i < NAMETBLSIZE && names[i] != "\0"; i++)
		nl[i].n_name = names[i];
}


#define KMREAD(name, seekaddr, addr) \
\
	(void)lseek(kmem, seekaddr, 0); \
	size = sizeof(*addr); \
	if (read(kmem, (char *)addr, size) != size){ \
		printf("vmpinfo:  read failed on %s!\n", name); \
	exit(2); \
}


main(argc, argv)
	int	argc;
	char	*argv[];
{
	long            first_page;
	long            i;
	int             kmem;
	char           *kmemfile;
	long            mem_size;
	struct nlist    nl[NAMETBLSIZE];
	char           *nlistfile;
	int             numpages;
	int             page_size;
	int             size;
	struct vm_page  vm_page;
	struct vm_page *vm_page_array;

	/* grab kernel memory */

	kmemfile = KMEMFILE;
	kmem = open(kmemfile, 0);
	if (kmem < 0) {
		perror(kmemfile);
		exit(1);
	}
	/* get symbol table info. for symbols given */

	initnlist(nl, names);
	nlistfile = NLISTFILE;
	nlist(nlistfile, nl);

	/* dig around in kernel memory for all the goodies */

	KMREAD("vm_page_array", (long) (nl[X_VM_PAGE_ARRAY].n_value),
	       (&vm_page_array))
	KMREAD("first_page", (long) (nl[X_FIRST_PAGE].n_value),
		       (&first_page))
	KMREAD("mem_size", (long) (nl[X_MEM_SIZE].n_value),
		       (&mem_size))
	KMREAD("page_size", (long) (nl[X_PAGE_SIZE].n_value),
		       (&page_size))
	numpages = (mem_size / page_size) - first_page;

	for (i = 0; i < numpages; i++) {
		KMREAD("vm_page", (long) (vm_page_array), (&vm_page));
		printf("Page #%d\n", i + first_page);
		printf("\tpageq  0x%x 0x%x\n", (vm_page.pageq).next,
		       (vm_page.pageq).prev);
		printf("\thashq  0x%x 0x%x\n", (vm_page.hashq).next,
		       (vm_page.hashq).prev);
		printf("\tlistq  0x%x 0x%x\n", (vm_page.listq).next,
		       (vm_page.listq).prev);
		printf("\tobject 0x%x\n", vm_page.object);
		printf("\toffset 0x%x\n", vm_page.offset);
		printf("\twires  %d\n", vm_page.wire_count);
		if (vm_page.inactive)
			printf("\tinactive\n");
		if (vm_page.active)
			printf("\tactive\n");
		if (vm_page.clean)
			printf("\tclean\n");
		if (vm_page.laundry)
			printf("\tlaundry\n");
		if (vm_page.busy)
			printf("\tbusy\n");
		vm_page_array++;
	}
}
