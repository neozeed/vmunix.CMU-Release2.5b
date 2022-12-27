#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)gcvt.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * gcvt  - Floating output conversion to
 * minimal length string
 **********************************************************************
 * HISTORY
 * 26-Jan-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to emulate the functionality of the RT acis version.
 *
 **********************************************************************
 */

char	*ecvt();

char *
gcvt(number, ndigit, buf)
double number;
char *buf;
{
	int sign, decpt;
#ifdef	ibmrt
	int ondigit;	/* for determining f(loat) or e(xp) print format */
#endif
	register char *p1, *p2;
	register i;

	p1 = ecvt(number, ndigit, &decpt, &sign);
	p2 = buf;
	if (sign)
		*p2++ = '-';
#ifdef	ibmrt
	if (*p1 < '0' || *p1 > '9') {	/* INF, NAN() */
		while (*p2++ = *p1++);
		return(buf);
	}
	if ((i = strlen(p1)) < ndigit)	/* < full precision from ecvt */
		ndigit = i;
	ndigit = ondigit = ((ndigit <= 16) ? ndigit - 1 : 16) + 1;
#endif
	for (i=ndigit-1; i>0 && p1[i]=='0'; i--)
		ndigit--;
#ifdef	ibmrt
	if (decpt >= 0 && decpt-ondigit > 0 
#else
	if (decpt >= 0 && decpt-ndigit > 4
#endif
	 || decpt < 0 && decpt < -3) { /* use E-style */
		decpt--;
		*p2++ = *p1++;
#ifdef	ibmrt
		if (ndigit < 1)
#endif
		*p2++ = '.';
		for (i=1; i<ndigit; i++)
			*p2++ = *p1++;
		*p2++ = 'e';
		if (decpt<0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
#ifdef	ibmrt
		if (decpt>99) {
			*p2++ = decpt/100 + '0';
			decpt = decpt%100;
		}
#endif
		*p2++ = decpt/10 + '0';
		*p2++ = decpt%10 + '0';
	} else {
		if (decpt<=0) {
			if (*p1!='0')
#ifdef	ibmrt
				*p2++ = '0';
			if (*p1!='0')
#endif
				*p2++ = '.';
			while (decpt<0) {
				decpt++;
				*p2++ = '0';
			}
		}
		for (i=1; i<=ndigit; i++) {
			*p2++ = *p1++;
			if (i==decpt)
				*p2++ = '.';
		}
		if (ndigit<decpt) {
			while (ndigit++<decpt)
				*p2++ = '0';
			*p2++ = '.';
		}
	}
	if (p2[-1]=='.')
		p2--;
	*p2 = '\0';
	return(buf);
}
