/****************************************
 *
 * HISTORY
 * $Log:	afs_genpag.c,v $
 * Revision 2.0  89/06/15  15:35:22  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <sys/time.h>

/*
 *  afs_genpag: generate a cryptographically secure 31-bit number.
 *
 *  Actually, we settle for just 'hard to guess'. Maybe someday
 *  I'll dip into the literature and come up with something real.
 */

/*
 *  Random31() is derived from the 3/9/86 version of random.c
 *  in the 4.3BSD C library.
 */

#define	STATELEN	256
#define	RAND_DEG	63
#define	RAND_SEP	1

static unsigned long afs_pagstate[STATELEN];
static unsigned long *afs_rptr	= &afs_pagstate[1];
static unsigned long *afs_fptr	= &afs_pagstate[RAND_SEP + 1];
static unsigned long *end_ptr	= &afs_pagstate[RAND_DEG + 1];

static unsigned long timerand()
{
	unsigned long tr = 0;
	unsigned long timeval[2 * sizeof(struct timeval) / sizeof(tr)];
	int j;

	gettimeofday((struct timeval *)timeval, 0);
	for (j = 0; j < sizeof(struct timeval) / sizeof(tr); j++) {
		tr ^= timeval[j];
	}
	return tr;
}

static unsigned long random31()
{
	unsigned long i;

	*afs_fptr += *afs_rptr;
	i = (*afs_fptr >> 1) & 0x7fffffff;
	afs_fptr++;
	afs_rptr++;
	if (afs_fptr >= end_ptr) {
		afs_fptr = &afs_pagstate[1];
	} else if (afs_rptr >= end_ptr) {
		afs_rptr = &afs_pagstate[1];
	}
	return i;
}

unsigned long afs_genpag()
{
	int j;
	unsigned short n, tr;

	for (j = 0; j < STATELEN; j++) {
		afs_pagstate[j] ^= timerand();
	}
	for (j = 0; j < 1000; j++) {
		random31();
	}
	tr = timerand();
	tr = (tr >> 16) + (tr & 0xffff);
	tr = (tr >> 8) + (tr & 0xff);
	for (n = tr; n > 0; n--) {
		random31();
	}
	return random31() ^ timerand();
}
