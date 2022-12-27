/* @(#)mntent.c	1.2 86/11/14 NFSSRC */
/* 
 * HISTORY
 * $Log:	mntent.c,v $
 * Revision 2.3  89/08/09  13:53:11  dimitris
 * 	Fixed a messup with the log messages.
 * 
 * 
 * Revision 2.1  89/07/19  09:12:34  dimitris
 * 	Changed to accept Ultrix type format for /etc/fstab.
 * 	[89/07/19  09:08:35  dimitris]
 * 
 */

#ifndef lint
static	char sccsid[] = "@(#)mntent.c 1.1 86/09/24 SMI";
#endif

#include <stdio.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/file.h>
#include <sys/param.h>

static	struct mntent mnt;
static	char line[BUFSIZ+1];

static char *
mntstr(p)
	register char **p;
{
	char *cp = *p;
	char *retstr;

#if CMUCS
	while (*cp && iscolon(*cp))
#else CMUCS
	while (*cp && isspace(*cp))
#endif CMUCS
		cp++;
	retstr = cp;
#if CMUCS
        while (*cp && !iscolon(*cp))
#else CMUCS
        while (*cp && !isspace(*cp))
#endif CMUCS
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (retstr);
}

static int
mntdigit(p)
	register char **p;
{
	register int value = 0;
	char *cp = *p;
	char *retstr;

#if CMUCS
        while (*cp && iscolon(*cp))
#else CMUCS
        while (*cp && isspace(*cp))
#endif CMUCS
		cp++;
	for (; *cp && isdigit(*cp); cp++) {
		value *= 10;
		value += *cp - '0';
	}
#if CMUCS
        while (*cp && !iscolon(*cp))
#else CMUCS
        while (*cp && !isspace(*cp))
#endif CMUCS
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (value);
}

static
mnttabscan(mnttabp, mnt)
	FILE *mnttabp;
	struct mntent *mnt;
{
	char *cp;
#if CMUCS
	char *p,*opts2;
#endif CMUCS

	do {
		cp = fgets(line, 256, mnttabp);
		if (cp == NULL) {
			return (EOF);
		}
	} while (*cp == '#');
	mnt->mnt_fsname = mntstr(&cp);
	if (*cp == '\0')
		return (1);
	mnt->mnt_dir = mntstr(&cp);
	if (*cp == '\0')
		return (2);
#if CMUCS

	/* parse Ultrix fstab format i.e. :
	     spec:file:type(ro...):freq:passno:name(nfs...):options(soft...)
         */
	mnt->mnt_opts = mntstr(&cp);
	if (*cp == '\0')
		return (3);
	mnt->mnt_freq = mntdigit(&cp);
	if (*cp == '\0')
		return (4);
	mnt->mnt_passno = mntdigit(&cp);
	if (*cp == '\0')
		return (5);
	mnt->mnt_type = mntstr(&cp);
	if (*cp == '\0')
		return (6);
	/* Now get the options at the end and add them to the opts field */

        opts2 = mntstr(&cp);
	if (isalpha(*opts2)) {
 	  p = (char *) malloc (strlen(mnt->mnt_opts) + strlen(opts2) + 2) ;
	  strcpy(p,mnt->mnt_opts);
	  strcat(p,",");
	  strcat(p,opts2);
	  mnt->mnt_opts = p;
	}

		return(7);
#else CMUCS
	mnt->mnt_type = mntstr(&cp);
	if (*cp == '\0')
		return (3);
	mnt->mnt_opts = mntstr(&cp);
	if (*cp == '\0')
		return (4);
	mnt->mnt_freq = mntdigit(&cp);
	if (*cp == '\0')
		return (5);
	mnt->mnt_passno = mntdigit(&cp);
	return (6);
#endif CMUCS
}
	
FILE *
setmntent(fname, flag)
	char *fname;
	char *flag;
{
	FILE *mnttabp;
	int lock;

	if ((mnttabp = fopen(fname, flag)) == NULL) {
		return (NULL);
	}
	lock = LOCK_SH;
	while (*flag) {
		if (*flag == 'w' || *flag == 'a' || *flag == '+') {
			lock = LOCK_EX;
		}
		flag++;
	}
	if (flock(fileno(mnttabp), lock) < 0) {
		fclose(mnttabp);
		return (NULL);
	}
	return (mnttabp);
}

int
endmntent(mnttabp)
	FILE *mnttabp;
{

	if (mnttabp) {
		fclose(mnttabp);
	}
	return (1);
}

struct mntent *
getmntent(mnttabp)
	FILE *mnttabp;
{
	int nfields;

	if (mnttabp == 0)
		return ((struct mntent *)0);
	nfields = mnttabscan(mnttabp, &mnt);
#if CMUCS
	if (nfields == EOF || (nfields != 7 && nfields !=5))
#else CMUCS
	if (nfields == EOF || nfields != 6)
#endif CMUCS
		return ((struct mntent *)0);
	return (&mnt);
}

addmntent(mnttabp, mnt)
	FILE *mnttabp;
	register struct mntent *mnt;
{
	if (fseek(mnttabp, 0, 2) < 0) {
		return (1);
	}
	mntprtent(mnttabp, mnt);
	return (0);
}

static char tmpopts[256];

static char *
mntopt(p)
	char **p;
{
	char *cp = *p;
	char *retstr;

#if CMUCS
        while (*cp && iscolon(*cp))
#else CMUCS
        while (*cp && isspace(*cp))
#endif CMUCS
		cp++;
	retstr = cp;
	while (*cp && *cp != ',')
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (retstr);
}

char *
hasmntopt(mnt, opt)
	register struct mntent *mnt;
	register char *opt;
{
	char *f, *opts;
 
	strcpy(tmpopts, mnt->mnt_opts);
	opts = tmpopts;
	f = mntopt(&opts);
	for (; *f; f = mntopt(&opts)) {
		if (strncmp(opt, f, strlen(opt)) == 0)
			return (f - tmpopts + mnt->mnt_opts);
	} 
	return (NULL);
}

static
mntprtent(mnttabp, mnt)
	FILE *mnttabp;
	register struct mntent *mnt;
{
#if CMUCS
	fprintf(mnttabp, "%s:%s:%s:%d:%d:%s:\n",
	    mnt->mnt_fsname,
	    mnt->mnt_dir,
	    mnt->mnt_opts,
	    mnt->mnt_freq,
	    mnt->mnt_passno,
	    mnt->mnt_type);
#else CMUCS
	fprintf(mnttabp, "%s %s %s %s %d %d\n",
	    mnt->mnt_fsname,
	    mnt->mnt_dir,
	    mnt->mnt_type,
	    mnt->mnt_opts,
	    mnt->mnt_freq,
	    mnt->mnt_passno);
#endif CMUCS
	return(0);
}

#if CMUCS

static int iscolon(c)

	char c;
{
  if (c==':') 
	return(1);
   return(0);
}

/*
 *      routine to convert notation "/@honeydew" to the notation
 *      honeydew:/ and vice versa (honeydew:/ to /@honeydew)
 *      This lets you put /@honeydew in /etc/fstab without changing
 *      fstab.c and it lets you use EITHER notation on the command line!
 */

char *convert(s,bad,good)
register char *s;
char *bad, *good;
{
	char	*index();
        register char *t, *p;
        register int len1,len2,i;
        char *ptr;

        if ((p = index(s,*bad)) == NULL) {
                return(s);
	}
        ptr = t = (char *) calloc(MAXPATHLEN,1);
        len1 = p - s;
        len2 = strlen(s) - len1 - 1;
        p++;
        for(i=0;i<len2;i++) *t++ = p[i];
        *t++ = *good;
        for(i=0;i<len1;i++) *t++ = s[i];
        return(ptr);
}

#endif CMUCS
	
