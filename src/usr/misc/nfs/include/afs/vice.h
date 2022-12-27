/* $ACIS:vice.h 9.0$ */
/* $Source: /afs/cs.cmu.edu/mach/rcs/usr/misc/.nfs/include/afs/vice.h,v $ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
#endif

/*
 * 5799-WZQ (C) COPYRIGHT IBM CORPORATION  1986
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
 * /usr/include/sys/vice.h
 *
 * Definitions required by user processes needing extended vice facilities
 * of the kernel.
 * 
 * NOTE:  /usr/include/sys/remote.h contains definitions common between
 *	    	Venus and the kernel.
 * 	    /usr/local/include/viceioctl.h contains ioctl definitions common
 *	    	between user processes and Venus.
 */

struct ViceIoctl {
	caddr_t in, out;	/* Data to be transferred in, or out */
	short in_size;		/* Size of input buffer <= 2K */
	short out_size;		/* Maximum size of output buffer, <= 2K */
};

/* The 2K limits above are a consequence of the size of the kernel buffer
   used to buffer requests from the user to venus--2*MAXPATHLEN.
   The buffer pointers may be null, or the counts may be 0 if there
   are no input or output parameters
 */

#define _VICEIOCTL(id)  ((unsigned int ) _IOW(V, id, struct ViceIoctl))
/* Use this macro to define up to 256 vice ioctl's.  These ioctl's
   all potentially have in/out parameters--this depends upon the
   values in the ViceIoctl structure.  This structure is itself passed
   into the kernel by the normal ioctl parameter passing mechanism.
 */

#define _VALIDVICEIOCTL(com) (com >= _VICEIOCTL(0) && com <= _VICEIOCTL(255))
