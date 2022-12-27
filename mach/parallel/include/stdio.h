/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)stdio.h	5.3 (Berkeley) 3/15/86
 */

/*  HISTORY
 * 28-Aug-87  Michael Jones (mbj) at Carnegie-Mellon University
 *	Converted f_lockbuf to return the lock pointer for later use by
 *	f_unlockbuf, eliminating a redundant call to _f_lock_ptr.
 *	Added typedef for f_buflock.
 *
 * 26-Aug-87  Michael Jones (mbj) at Carnegie-Mellon University
 *	Renamed clearerr to unlocked_clearerr as per getc/putc.
 *
 * 13-Aug-87  Michael Jones (mbj) at Carnegie-Mellon University
 *	Renamed getc & putc macros to unlocked_getc & unlocked_putc so
 *	recompiled code will run safely in a multi-threaded environment.
 *	New getc & putc macros simply call fgetc and fputc.  Revised
 *	locking macros to ignore string and illegal buffers.
 *
 *  8-Jul-87  Michael Jones (mbj) at Carnegie-Mellon University
 *	Added f_lockbuf and f_unlockbuf macros for parallel file
 *	access interlocking.
 *
 *   29-Jun-85	Mary Thompson @ Carnegie Mellon
 *	Added definition of _FMAP. It is used in
 *	#if _FMAP  conditionals and may be turned off
 *	by _D_FMAP=0 on the command line.
 *   23-Jan-85	Mary Thompson @ Carnegie Mellon
 *	Added constants for mapped files 
 */

# ifndef FILE
#ifndef _FMAP
#define _FMAP 1
#endif _FMAP

#define	BUFSIZ	1024
#if _FMAP
#define MAPBUFSIZE (16*1024)
#endif _FMAP
extern	struct	_iobuf {
	int	_cnt;
	char	*_ptr;		/* should be unsigned char */
	char	*_base;		/* ditto */
	int	_bufsiz;
	short	_flag;
	char	_file;		/* should be short */
} _iob[];

#define	_IOREAD	01
#define	_IOWRT	02
#define	_IONBF	04
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	_IOLBF	0200
#define	_IORW	0400
#if _FMAP
#define _IOMAP  01000   /* for mapped files */
#endif _FMAP
#define	NULL	0
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])
#ifndef lint
#define	unlocked_getc(p) (--(p)->_cnt>=0? (int)(*(unsigned char *)(p)->_ptr++):_filbuf(p))
#endif not lint
#define	unlocked_getchar()	unlocked_getc(stdin)
#define getc(p)		fgetc(p)
#define	getchar()	getc(stdin)
#ifndef lint
#define unlocked_putc(x, p)	(--(p)->_cnt >= 0 ?\
	(int)(*(unsigned char *)(p)->_ptr++ = (x)) :\
	(((p)->_flag & _IOLBF) && -(p)->_cnt < (p)->_bufsiz ?\
		((*(p)->_ptr = (x)) != '\n' ?\
			(int)(*(unsigned char *)(p)->_ptr++) :\
			_flsbuf(*(unsigned char *)(p)->_ptr, p)) :\
		_flsbuf((unsigned char)(x), p)))
#endif not lint
#define	unlocked_putchar(x)	unlocked_putc(x,stdout)
#define putc(x, p)	fputc(x, p)
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	((p)->_file)
#define	unlocked_clearerr(p)	((p)->_flag &= ~(_IOERR|_IOEOF))

FILE	*fopen();
FILE	*fdopen();
FILE	*freopen();
FILE	*popen();
long	ftell();
char	*fgets();
char	*gets();
char	*sprintf();
#if _FMAP
char	*fmap();
char    *fremap();
#endif _FMAP

typedef struct rec_mutex *f_buflock;
extern f_buflock f_lockbuf();
#define f_unlockbuf(buflock) { \
	if ((buflock) != (struct rec_mutex *) NULL) \
		rec_mutex_unlock(buflock); \
	}

# endif
