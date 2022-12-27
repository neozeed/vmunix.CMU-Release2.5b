#if	ibmrt
/*
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* ieee.h \c \"This file is also included by a man page, hence this nroff code
.if 0 \{\
*/
/* $Header:ieee.h 9.0$ */
/* $ACIS:ieee.h 9.0$ */
/* $Source: /ibm/acis/usr/src/include/RCS/ieee.h,v $ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidieee = "$Header:ieee.h 9.0$";
#endif
/* 
.\}\" End of nroff code
*/

/***************************************************************************/
/*   types for recommended functions and support routines		   */
/***************************************************************************/

typedef enum { 				/* allows user setting and	   */
	TONEAREST, 			/* clearing of rounding mode	   */
	UPWARD, 
	DOWNWARD, 
	TOWARDZERO 
} ROUNDDIR;

typedef enum {				/* every ieee floating point	   */
	SNAN,				/* value is in one and only one	   */
	QNAN,				/* of these classes		   */
	INFINITE,
	ZERONUM,
	NORMALNUM,
	DENORMALNUM 
} NUMCLASS;

#define FPINVALID	1		/* allows user to set and clear    */
#define FPUNDERFLOW	2		/* floating point exception flags  */
#define FPOVERFLOW	4		/* and traps			   */
#define FPDIVBYZERO	8
#define FPINEXACT	16
#define FPALLEXCEPTIONS 0x1f

typedef short FPEXCEPTION;		/* a sum of particular exceptions  */

/***************************************************************************/
/*   IEEE recommended functions and recommended support routines.	   */
/***************************************************************************/

double copysign(), rint(), scalb(), logb(), drem(), nextdouble(), infinity();
float  nextfloat();
NUMCLASS classfloat(), classdouble();
FPEXCEPTION swapfpflag(), fptestflag();
void fpsetflag(), fpclrflag();
FPEXCEPTION swapfptrap(), fptesttrap();
void fpsettrap(), fpclrtrap();
ROUNDDIR swapround(), fptestround();
void fpsetround();

/***************************************************************************/
/*   Mode specifiers for conversion (ecvt, fcvt) rounding.		   */
/***************************************************************************/

#define	CVT_ROUND	0		/* rounding "to-roundest"	   */
#define	CVT_IEEE	1		/* uses ieee rounding		   */
#else	ibmrt
/* this is supposed to generate an error */
#include <ieee-not-supported.h>
#endif	ibmrt
