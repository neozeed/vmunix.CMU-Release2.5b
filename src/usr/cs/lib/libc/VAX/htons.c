/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
_sccsid:.asciz	"@(#)htons.c	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* hostorder = htons(netorder) */

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
ENTRY(htons, 0)
#else	CMUCS
ENTRY(htons)
#endif	CMUCS
	rotl	$8,4(ap),r0
	movb	5(ap),r0
	movzwl	r0,r0
	ret
