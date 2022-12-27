#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ttyname.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 * ttyname(f): return "/dev/ttyXX" which the the name of the
 * tty belonging to file f.
 *  NULL if it is not a tty
 **********************************************************************
 * HISTORY
 * 03-Jun-88  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Merged cmuttyname() into this module.
 *
 * 11-Dec-85  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added call to cmuttyname routine to make pty lookups faster.
 *
 **********************************************************************
 */

#define	NULL	0
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>

static	char	dev[]	= "/dev/";
char	*strcpy();
char	*strcat();

#if	CMUCS
#include <libc.h>
#include <c.h>

#define	TMR_HEX		16
#define	TMR_ALPHA	26

static struct ttymap {
	int tm_rdev;
	char tm_base;
	char tm_radix;
	char tm_count;
} ttymap[] = {
	-1,	'v',	TMR_HEX,	1,
	-1,	'p',	TMR_HEX,	5,
	-1,	'P',	TMR_HEX,	5,
	-1,	'f',	TMR_ALPHA,	1
};

static
char *cmuttyname(rdev)
dev_t rdev;
{
	struct stat stb;
	register int i, n, rmin, rmaj = major(rdev);
	register struct ttymap *tm;
	static char ttytemp[32] = "/dev/tty??";

	for (i = 0; i < sizeofA(ttymap); i++) {
		tm = &ttymap[i];
		if (tm->tm_base == '\0')
			continue;
		if (tm->tm_rdev < 0) {
			ttytemp[8] = tm->tm_base;
			ttytemp[9] = (tm->tm_radix == TMR_HEX) ? '0' : 'a';
			if (stat(ttytemp, &stb) < 0) {
				tm->tm_base = '\0';
				continue;
			}
			tm->tm_rdev = stb.st_rdev;
			n = tm->tm_count;
			tm->tm_count = 0;
			do {
				tm->tm_count += tm->tm_radix;
				if (--n == 0)
					break;
				ttytemp[8]++;
			} while (stat(ttytemp, &stb) >= 0);
		}
		if (major(tm->tm_rdev) == rmaj) {
			rmin = minor(rdev);
			if (rmin < minor(tm->tm_rdev) ||
			    rmin >= minor(tm->tm_rdev) + tm->tm_count)
				continue;
			rmin -= minor(tm->tm_rdev);
			switch (tm->tm_radix) {
			case TMR_HEX:
				ttytemp[8] = tm->tm_base+((rmin>>4)&0xf);
				ttytemp[9] = (rmin&0xf) < 10 ?
					     (rmin&0xf) + '0' :
					     (rmin&0xf) - 10 + 'a';
				break;
			case TMR_ALPHA:
				ttytemp[8] = tm->tm_base;
				ttytemp[9] = rmin + 'a';
				break;
			}
			return(ttytemp);
		}
	}
	return(NULL);
}
#endif	/* CMUCS */

char *
ttyname(f)
{
	struct stat fsb;
	struct stat tsb;
	register struct direct *db;
	register DIR *df;
	static char rbuf[32];

	if (isatty(f)==0)
		return(NULL);
	if (fstat(f, &fsb) < 0)
		return(NULL);
	if ((fsb.st_mode&S_IFMT) != S_IFCHR)
		return(NULL);
#if	CMUCS
	{
		char *p;

		if ((p = cmuttyname(fsb.st_rdev)) != NULL)
			return(p);
	}
#endif	/* CMUCS */
	if ((df = opendir(dev)) == NULL)
		return(NULL);
	while ((db = readdir(df)) != NULL) {
		if (db->d_ino != fsb.st_ino)
			continue;
		strcpy(rbuf, dev);
		strcat(rbuf, db->d_name);
		if (stat(rbuf, &tsb) < 0)
			continue;
		if (tsb.st_dev == fsb.st_dev && tsb.st_ino == fsb.st_ino) {
			closedir(df);
			return(rbuf);
		}
	}
	closedir(df);
	return(NULL);
}
