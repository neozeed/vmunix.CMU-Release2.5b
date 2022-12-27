/*	machine.h	4.1	81/05/14	*/
/*  HISTORY
 *
 * 11-Mar-88  Mary Thompson (mrt) at Carnegie Mellon
 *	Changed MACH_TT conditional to MACH || MACH_TT
 *  17-Sep-87	Mary Thomson, David Golub @ Carnegie Mellon
 *	Added a #define of KERNEL-FEATURES and changed
 *	MAXSTOR for MACH_TT kernels
 */

#ifndef KERNEL_FEATURES
#define KERNEL_FEATURES
#endif KERNEL_FEATURES
#include <sys/vm.h>

#define	PAGSIZ		(NBPG*CLSIZE)

#define DBNAME "adb\n"
#define LPRMODE "%R"
#define OFFMODE "+%R"
#define	TXTRNDSIZ	PAGSIZ

#define	MAXINT	0x7fffffff
#if	MACH_TT || MACH
#define MAXSTOR	(USRSTACK + sizeof(int [5]))  /* size of sigcode, see machine/vmparam.h */
#else	MACH_TT || MACH
#define	MAXSTOR ((1L<<31) - ctob(UPAGES))
#endif	MACH_TT || MACH
#define	MAXFILE 0xffffffff

/*
 * INSTACK tells whether its argument is a stack address.
 * INUDOT tells whether its argument is in the (extended) u. area.
 * These are used for consistency checking and dont have to be exact.
 *
 * INKERNEL tells whether its argument is a kernel space address.
 * KVTOPH trims a kernel virtal address back to its offset
 * in the kernel address space.
 */
#define	INSTACK(x)	(((x)&0xf0000000) == 0x70000000)
#define	INUDOT(x)	(((x)&0xf0000000) == 0x70000000)
#define	INKERNEL(x)	(((x)&0xf0000000) == 0x80000000)
/* this INUDOT definition is correct for MACH_TT because vm_unix.fake_u lies
   about where the kernel stack was */


#define	KVTOPH(x)	((x)&~ 0x80000000)
#define	KERNOFF		0x80000000
