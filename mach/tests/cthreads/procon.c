/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	procon.c,v $
 * Revision 1.3  89/05/05  19:03:57  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 * Revision 1.2  88/10/25  17:02:39  mrt
 * Changed string_t to char *
 */
/* procon.c
 *	This is a test program for the cthreads library package
 *	Usage is procon <file>
 *	One thread reads the file and the other thread
 *	prints it out
 */

#include <stdio.h>
#include <cthreads.h>

#define DEFAULT_SIZE 1

typedef struct buffer {
	struct mutex lock;
	char *chars;	/* chars[0..size-1] */
	int size;
	int px, cx;	/* producer and consumer indices into chars[] */
	int count;	/* number of unconsumed chars in buffer */
	int eof;
	struct condition non_empty, non_full;
} *buffer_t;

buffer_t
buffer_alloc(size)
	int size;
{
	buffer_t b;
	extern char *malloc();

	b = (buffer_t) malloc(sizeof(struct buffer));
	mutex_init(&b->lock);
	b->size = size;
	b->chars = malloc((unsigned) size);
	b->px = b->cx = b->count = 0;
	b->eof = FALSE;
	condition_init(&b->non_empty);
	condition_init(&b->non_full);
	return b;
}

producer(b)
	buffer_t b;
{
	int ch;

	mutex_lock(&b->lock);
	while ((ch = getchar()) != EOF) {
		while (b->count == b->size)
			condition_wait(&b->non_full, &b->lock);
		ASSERT(0 <= b->count && b->count < b->size);
		b->chars[b->px] = ch;
		b->px = (b->px + 1) % b->size;
		b->count += 1;
		condition_signal(&b->non_empty);
	}
	b->eof = TRUE;
	/* wake up consumer */
	condition_signal(&b->non_empty);
	mutex_unlock(&b->lock);
}

consumer(b)
	buffer_t b;
{
	int ch;

	mutex_lock(&b->lock);
	for (;;) {
		while (b->count == 0) {
			if (b->eof) {
				mutex_unlock(&b->lock);
				return;
			}
			condition_wait(&b->non_empty, &b->lock);
		}
		ASSERT(0 < b->count && b->count <= b->size);
		ch = b->chars[b->cx];
		if (!cthread_debug)
			printf("%c", ch);
		b->cx = (b->cx + 1) % b->size;
		b->count -= 1;
		condition_signal(&b->non_full);
	}
}

cthread_t pro, con;

watcher()
{
	(void) cthread_join(pro);
	(void) cthread_join(con);
}

main(argc, argv)
	int argc;
	char **argv;
{
	char *prog;
	int size = 0;
	buffer_t b;

	cthread_init();
	prog = *argv++;
	argc -= 1;
	if (argc > 0 && strcmp(argv[0], "-d") == 0) {
#ifdef	DEBUG
		cthread_debug = TRUE;
#endif	DEBUG
		argc -= 1;
		argv += 1;
	}
	if (argc > 0 && (size = atoi(argv[0])) != 0) {
		argc -= 1;
		argv += 1;
	}
	if (argc > 1) {
		fprintf(stderr, "Usage: %s [-d size file]\n", prog);
		exit(1);
	}
	if (argc > 0 && freopen(argv[0], "r", stdin) == NULL) {
		perror(argv[0]);
		exit(1);
	}
	b = buffer_alloc(size > 0 ? size : DEFAULT_SIZE);
	pro = cthread_fork(producer, (any_t) b);
	con = cthread_fork(consumer, (any_t) b);
	cthread_detach(cthread_fork(watcher, (any_t) 0));
	cthread_exit(0);
}
