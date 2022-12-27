/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
_sccsid:.asciz	"@(#)htonl.c	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* netorder = htonl(hostorder) */

/*
 **********************************************************************
 * HISTORY
 * 06-Nov-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Standardize upon one ENTRY macro format.
 *
 **********************************************************************
 */
#include "DEFS.h"

#if	CMUCS
ENTRY(htonl, 0)
#else	CMUCS
ENTRY(htonl)
#endif	CMUCS
	rotl	$-8,4(ap),r0
	insv	r0,$16,$8,r0
	movb	7(ap),r0
	ret
