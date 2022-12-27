/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getttyent.c	5.4 (Berkeley) 5/19/86";
#endif LIBC_SCCS and not lint

/*
 **********************************************************************
 * HISTORY
 * 26-Jan-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added transition code to read /etc/TTYS if available until
 *	we no longer have a 4.2 format /etc/ttys file.
 *
 **********************************************************************
 */
#include <stdio.h>
#include <strings.h>
#include <ttyent.h>

#if	CMUCS
#define TTYFILE		whichttyfile()
#else	CMUCS
static char TTYFILE[] = "/etc/ttys";
#endif	CMUCS
static char zapchar;
static FILE *tf = NULL;
#define LINE 256
static char line[LINE];
static struct ttyent tty;

#if	CMUCS
#include <sys/types.h>
#include <sys/stat.h>

static
char *whichttyfile()
{
	struct stat statb;
	static char *ttyfile = NULL;

	if (ttyfile == NULL) {
		ttyfile = "/etc/TTYS";
		if (stat(ttyfile, &statb) < 0)
			ttyfile = "/etc/ttys";
	}
	return(ttyfile);
}
#endif	CMUCS

setttyent()
{
	if (tf == NULL)
		tf = fopen(TTYFILE, "r");
	else
		rewind(tf);
}

endttyent()
{
	if (tf != NULL) {
		(void) fclose(tf);
		tf = NULL;
	}
}

#define QUOTED	1

/*
 * Skip over the current field, removing quotes,
 * and return a pointer to the next field.
 */
static char *
skip(p)
	register char *p;
{
	register char *t = p;
	register int c;
	register int q = 0;

	for (; (c = *p) != '\0'; p++) {
		if (c == '"') {
			q ^= QUOTED;	/* obscure, but nice */
			continue;
		}
		if (q == QUOTED && *p == '\\' && *(p+1) == '"')
			p++;
		*t++ = *p;
		if (q == QUOTED)
			continue;
		if (c == '#') {
			zapchar = c;
			*p = 0;
			break;
		}
		if (c == '\t' || c == ' ' || c == '\n') {
			zapchar = c;
			*p++ = 0;
			while ((c = *p) == '\t' || c == ' ' || c == '\n')
				p++;
			break;
		}
	}
	*--t = '\0';
	return (p);
}

static char *
value(p)
	register char *p;
{
	if ((p = index(p,'=')) == 0)
		return(NULL);
	p++;			/* get past the = sign */
	return(p);
}

struct ttyent *
getttyent()
{
	register char *p;
	register int c;

	if (tf == NULL) {
		if ((tf = fopen(TTYFILE, "r")) == NULL)
			return (NULL);
	}
	do {
		p = fgets(line, LINE, tf);
		if (p == NULL)
			return (NULL);
		while ((c = *p) == '\t' || c == ' ' || c == '\n')
			p++;
	} while (c == '\0' || c == '#');
	zapchar = 0;
	tty.ty_name = p;
	p = skip(p);
	tty.ty_getty = p;
	p = skip(p);
	tty.ty_type = p;
	p = skip(p);
	tty.ty_status = 0;
	tty.ty_window = NULL;
	for (; *p; p = skip(p)) {
#define space(x) ((c = p[x]) == ' ' || c == '\t' || c == '\n')
		if (strncmp(p, "on", 2) == 0 && space(2))
			tty.ty_status |= TTY_ON;
		else if (strncmp(p, "off", 3) == 0 && space(3))
			tty.ty_status &= ~TTY_ON;
		else if (strncmp(p, "secure", 6) == 0 && space(6))
			tty.ty_status |= TTY_SECURE;
		else if (strncmp(p, "window=", 7) == 0)
			tty.ty_window = value(p);
		else
			break;
	}
	if (zapchar == '#' || *p == '#')
		while ((c = *++p) == ' ' || c == '\t')
			;
	tty.ty_comment = p;
	if (*p == 0)
		tty.ty_comment = 0;
	if (p = index(p, '\n'))
		*p = '\0';
	return(&tty);
}
