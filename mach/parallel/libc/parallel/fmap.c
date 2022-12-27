/*******************************************************
 *  ABSTRACT  
 *	fmap, fremap and bufsize
 *
 *	fmap maps an entire file into the buffer pointed
 *	to by iop->_base. It returns the pointer to this
 *	buffer to the user, who can then directly read
 *	or write parts of the buffer. If the file is opened
 *	for read or read/write and size is not >= to the file
 *	size, the buffer will be equal to the file size.
 *
 *	fremap is called to change the size of the buffer
 *	that a file is mapped to. If the size is being
 *	increased, a  new buffer may be allocated and returned
 *	to the caller.
 *
 *	If the stream already has a non-mapped buffer assigned
 *	to it, this buffer will be flushed and freed and a
 *	new buffer assigned that is mapped.
 *
 * 	bufsize returns the actual size of the buffer.
 *****************************************************/

/*
 * HISTORY:
 * $Log:	fmap.c,v $
 * Revision 2.1.1.4  89/07/31  15:16:28  mbj
 * 	Include mach/std_types.h.
 * 
 * Revision 2.1.1.3  89/07/31  13:30:42  mbj
 * 	Fall back to non-mapping algorithm when map_fd fails.
 * 	Place all debugging printfs under #if DEBUG.
 * 
 * Revision 2.1.1.2  89/07/20  17:08:45  mbj
 * 	Changed to always map the whole file, and to set
 * 	iop->ptr to be non-zero if an lseek has already 
 * 	been done. e.g. by open for append or a fseek call.
 * 	[88/11/29            mrt]
 * 
 * Revision 2.1.1.1  89/07/20  16:46:05  mbj
 * 	Check parallel libc and file mapping changes into source tree branch.
 * 
 * 28-Aug-87  Michael Jones (mbj) at Carnegie-Mellon University
 *	Converted f_lockbuf to return and f_unlockbuf to use lock pointer.
 *
 *  6-Aug-87  Michael Jones (mbj) at Carnegie-Mellon University
 *	Added lock calls for multi-threaded libc.
 *
 *   12-May-87  Mary Thompson @ Carnegie Mellon
 *	Fixed to not map anything except regular files.
 *
 *    1-Jan-87	Mary Thompson @ Carnegie Mellon
 *	Started
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<mach/std_types.h>
#include	<mach.h>

char *fmap(iop,bufsize)
register FILE *iop;
int	 bufsize;
{
   char 	*buf;
   boolean_t	reading;
   struct stat 	stbuf;
   int		filesize,i;
   kern_return_t retcode;
   int		fileptr;
   register f_buflock buflock;

	buflock = f_lockbuf(iop);
	buf = NULL;   
#if	DEBUG
fprintf(stderr, "fmap: flag is %o\n",iop->_flag); 
#endif	DEBUG
	if ((iop->_flag&_IOMAP) == _IOMAP) {
		f_unlockbuf(buflock);
		return(iop->_base);
	}

	if (iop->_base != NULL)
		fseek(iop,0,0);  /* flush buffer and reset fileptr to 0 */


	reading = (iop->_flag&(_IOREAD|_IORW)) != 0;

	if ((fstat(fileno(iop),&stbuf) == 0) &&
		((stbuf.st_mode & S_IFMT) == S_IFREG))
		filesize = stbuf.st_size;
	else {  /* special or non-existant file */
		goto failure;
	}

	 /* get current fileptr, may be non-zero if file was opened 
	    for append or a fseek call was done */
	fileptr = lseek(fileno(iop),0,1);

	if (reading)   /* set bufsize to size of file */
	{
	    if (filesize > bufsize)
		bufsize = filesize;
	}

	if (bufsize <= 0)  /* for zero length file */
		bufsize = MAPBUFSIZE;

#if	DEBUG
	fprintf(stderr, "calling vm_allocate, size is %d\n",bufsize);
#endif	DEBUG
	retcode = vm_allocate(task_self_,(vm_address_t)&buf,bufsize,TRUE);
	if (retcode != KERN_SUCCESS) {
#if	DEBUG
		fprintf(stderr, "retcode from vm_allocate is %d, _base is %d\n",
			retcode, iop->_base);
#endif	DEBUG
		goto failure;
	}

	if (filesize > 0)
	{
#if	DEBUG
		fprintf(stderr, "calling map_fd, filesize is %d\n",filesize);
#endif	DEBUG
		retcode = map_fd(fileno(iop),(vm_offset_t)0,(vm_offset_t)&buf,FALSE,filesize);
		if (retcode != KERN_SUCCESS) {
#if	DEBUG
			fprintf(stderr, "retcode from map_fd is %d\n",retcode);
#endif	DEBUG
			(void) vm_deallocate(task_self_, buf, bufsize);
			goto failure;
		}
	}

	setbuffer(iop,buf,bufsize);  /* if buf == NULL, setbuffer leaves iop unbuffered */
	iop->_flag |= _IOMAP;
	iop->_ptr = iop->_base+fileptr;
	if (iop->_flag&_IOREAD)
	    iop->_cnt = filesize-fileptr; /* filesize */
	else iop->_cnt = bufsize-fileptr;   /* buffer size */

	f_unlockbuf(buflock);
	return(iop->_base);

failure:
	f_unlockbuf(buflock);
	return NULL;
}

char *fremap(iop,bufsize)
register FILE *iop;
int	 bufsize;
{
    vm_size_t		vmsize,newvmsize;
    kern_return_t	retcode;
    vm_address_t	bufptr;
    int			filesize;
    struct stat 	stbuf;
    char		*buftmp;
    register f_buflock buflock;

	buflock = f_lockbuf(iop);
	if ((iop->_flag&_IOMAP) != _IOMAP) {
		buftmp = fmap(iop,bufsize);
		f_unlockbuf(buflock);
		return buftmp;
	}

	retcode = KERN_SUCCESS;
	if ((fstat(fileno(iop),&stbuf) == 0) &&
		((stbuf.st_mode & S_IFMT) == S_IFREG))
		filesize = stbuf.st_size;
	else { /* special or non-existant file */
		goto failure;
	}

	vmsize=((iop->_bufsiz+vm_page_size-1)/vm_page_size)*vm_page_size;
	newvmsize=((bufsize+vm_page_size-1)/vm_page_size)*vm_page_size;
	if (newvmsize<vmsize)
	{
	    retcode = vm_deallocate(task_self_,
			(vm_address_t)(iop->_base+newvmsize),
			vmsize-newvmsize);
	    if (retcode != KERN_SUCCESS) {
#if	DEBUG
		fprintf(stderr, "fremap: error from vm_deallocate: %d\n",retcode);
#endif	DEBUG
	    }
#if	DEBUG
	    else fprintf(stderr, "fremap returns success from vm_deallocate\n");
#endif	DEBUG
	}
	else if (newvmsize > vmsize)
	{
	    /*
	     * Try to extend allocation in place
	     */
	    bufptr = (vm_address_t)(iop->_base+vmsize); 
	    retcode = vm_allocate(task_self_, &bufptr,
			  newvmsize-vmsize,FALSE);
	    if (retcode != KERN_SUCCESS)
	    {
		/*
		 * Reallocate new full sized region
		 */
		bufptr = NULL;
	 	retcode = vm_allocate(task_self_, &bufptr, newvmsize, TRUE);
		if (retcode == KERN_SUCCESS)
		{   retcode = vm_copy(task_self_, (vm_address_t)iop->_base,
					vmsize,bufptr);
		    (void) vm_deallocate(task_self_,
					(vm_address_t)iop->_base,
					vmsize);
		    iop->_base = (char *)bufptr;

		    if (retcode == KERN_SUCCESS) {
			retcode = map_fd(fileno(iop),0,(vm_offset_t)&bufptr,FALSE,filesize);
#if	DEBUG
			if (retcode != KERN_SUCCESS)
			    fprintf(stderr, "retcode from map_fd is %d\n",retcode);
#endif	DEBUG
		    }
#if	DEBUG
		fprintf(stderr, "fremap called vm_copy and vm_deallocate\n");
#endif	DEBUG
		}
	    }
#if	DEBUG
	    else fprintf(stderr, "fremap returns success from vm_allocate\n");
#endif	DEBUG
	}
	if (retcode == KERN_SUCCESS) {
	    iop->_ptr = iop->_base;
	    iop->_cnt = iop->_bufsiz = bufsize;
	    f_unlockbuf(buflock);
	    return(iop->_base);
	}

failure:
	iop->_base = iop->_ptr = NULL;
	iop->_cnt = iop->_bufsiz = 0;
	iop->_flag &= ~_IOMAP;
	iop->_flag |= _IONBF;
	f_unlockbuf(buflock);
	return NULL;
}



int bufsize(iop)
register FILE *iop;
{
	register int size;
	register f_buflock buflock;

	buflock = f_lockbuf(iop);
	if (iop->_flag&(_IONBF|_IOMYBUF) == _IOMYBUF
	    || iop->_flag&_IOMAP )
	    size = (iop->_bufsiz);
	else size = 0;
	f_unlockbuf(buflock);
	return size;
}
