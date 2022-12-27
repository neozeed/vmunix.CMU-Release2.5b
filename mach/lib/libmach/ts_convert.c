/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	ts_convert.c,v $
 * Revision 1.2  89/05/05  18:47:31  mrt
 * 	Cleanup for Mach 2.5
 * 
 */
/*
 *	ts_convert.c -- convert machine dependent timestamps to
 *		machine independent timevals.
 *
 */

#include <sys/time.h>
#include <mach/time_stamp.h>

convert_ts_to_tv(ts_format,tsp,tvp)
int	ts_format;
struct tsval *tsp;
struct timeval *tvp;
{
	switch(ts_format) {
		case TS_FORMAT_DEFAULT:
			/*
			 *	High value is tick count at 100 Hz
			 */
			tvp->tv_sec = tsp->high_val/100;
			tvp->tv_usec = (tsp->high_val % 100) * 10000;
			break;
		case TS_FORMAT_MMAX:
			/*
			 *	Low value is usec.
			 */
			tvp->tv_sec = tsp->low_val/1000000;
			tvp->tv_usec = tsp->low_val % 1000000;
			break;
		default:
			/*
			 *	Zero output timeval to indicate that
			 *	we can't decode this timestamp.
			 */
			tvp->tv_sec = 0;
			tvp->tv_usec = 0;
			break;
	 }
}
