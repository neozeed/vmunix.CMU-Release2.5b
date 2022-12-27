/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	cmds.c,v $
 * Revision 1.2  89/05/05  18:25:57  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  4-Jun-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Removed wholine support, slightly reformatted.
 *
 */
#define MACH

#include "parse.h"
#include <stdio.h>
#include <vaxuba/qvioctl.h>

extern char		**cmd_help();
extern int		qvssfd;
extern struct qv_info	*qv_scn;
int			debug;

/*
 * NEVER FORGET to add your command to the "cmds" table at the end of
 * this file or it can not be invoked.
 */
 
/*
 * Debugged ioctl's
 */

unsigned short cursor[] = {
	0xaa, 0x55, 0xaa, 0x55,
	0xaa, 0x55, 0xaa, 0x55,
	0xaa, 0x55, 0xaa, 0x55,
	0xaa, 0x55, 0xaa, 0x55
};


char **cmd_setup(self, argv, argvend)
	struct cmd_entry	*self;
	char			**argv, **argvend;
{
	register int	status;
	int		num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	++argv;
	if (num != 100 && num != 260) {
		fprintf(stderr, "qvss: only models 100 and 260 are supported\n");
		return argv;
	}
	++num;
	status = ioctl(qvssfd, QIOCSETUP, &num);
	if (status < 0) {
		printf("QIOCSETUP: status = %d.\n", status);
		perror("QIOCSETUP");
	}
	return(argv);
}

char **cmd_reset(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	status = ioctl(qvssfd, QIOCCURSORICON, cursor);
	if (status < 0) {
		printf("QIOCCURSORICON: status = %d.\n", status);
		perror("QIOCCURSORICON");
	}
	num = 15 * 60;
	status = ioctl(qvssfd, QIOCFADE, &num);
	if (status < 0) {
		printf("QIOCFADE: status = %d.\n", status);
		perror("QIOCFADE");
	}
	num = 30;
	status = ioctl(qvssfd, QIOCCURSOR, &num);
	if (status < 0) {
		printf("QIOCCURSOR: status = %d.\n", status);
		perror("QIOCCURSOR");
	}
	num = LED_OFF;
	status = ioctl(qvssfd, QIOCLEDPATTERN, &num);
	if (status < 0) {
		printf("QIOCLEDPATTERN: status = %d.\n", status);
		perror("QIOCLEDPATTERN");
	}
	printf("Fade 15 minutes blink 30 lights off\n");
	return argv;
}

char **cmd_fade(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	++argv;
	if (!strncmp(*argv, "hour", 4)) {
		num *= 3600;
		++argv;
	} else if (!strncmp(*argv, "min", 3)) {
		num *= 60;
		++argv;
	}
	status = ioctl(qvssfd, QIOCFADE, &num);
	if (status < 0) {
		printf("QIOCFADE: status = %d.\n", status);
		perror("QIOCFADE");
	}
	return argv;
}

char **cmd_cursor_same(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	status = ioctl(qvssfd, QIOCCURSOR, &num);
	if (status < 0) {
		printf("QIOCCURSOR: status = %d.\n", status);
		perror("QIOCCURSOR");
	}
	return(++argv);
}

char **cmd_cursor_both(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	status = ioctl(qvssfd, QIOCCURSORON, &num);
	if (status < 0) {
		printf("QIOCBOTH: status = %d.\n", status);
		perror("QIOCBOTH");
	}
	argv++;
	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	status = ioctl(qvssfd, QIOCCURSOROFF, &num);
	if (status < 0) {
		printf("QIOCBOTH: status = %d.\n", status);
		perror("QIOCBOTH");
	}
	return(++argv);
}

char **cmd_cursor_on(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	status = ioctl(qvssfd, QIOCCURSORON, &num);
	if (status < 0) {
		printf("QIOCCURSORON: status = %d.\n", status);
		perror("QIOCCURSORON");
	}
	return(++argv);
}

char **cmd_cursor_off(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return ++argv;
	}
	status = ioctl(qvssfd, QIOCCURSOROFF, &num);
	if (status < 0) {
		printf("QIOCCURSOROFF: status = %d.\n", status);
		perror("QIOCCURSOROFF");
	}
	return(++argv);
}

static
short mask[16] = { 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00};

char **cmd_cursor_icon(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	register int i;

	for (i = 0; i < 16; i++, argv++) {
		if (getnum(argv < argvend ? *argv : "Too Few Args", &mask[i])
					< 0) {
			fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
				self->cmd,
				argv < argvend ? *argv : "Too Few Args");
			return(argv);
		}
	}

	status = ioctl(qvssfd, QIOCCURSORICON, mask);
	if (status < 0) {
		printf("QIOCCURSORICON: status = %d.\n", status);
		perror("QIOCCURSORICON");
	}
	return(argv);
}

struct entry {
	char	*name;
	int	mode;
	int	def_interval;
};

static
struct entry modes[] = {
	{ "off",	LED_OFF,	0 },
	{ "count",	LED_COUNT,	5 },
	{ "rotate",	LED_ROTATE,	6 },
	{ "cylon",	LED_CYLON,	6 },
	{ 0,		0,		0 }
};

char **cmd_LED_finish();

char **cmd_LED(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register struct entry *m = modes;

	if (argv >= argvend) {
		fprintf(stderr,
			"qvss: lights [off|count|rotate|cylon] [interval]\n");
		return(argv);
	}
	while (m->name) {
		if (!strcmp(*argv, m->name)) {
			++argv;
			return(cmd_LED_finish(m, self, argv, argvend));
		}
		m++;
	}
	return(cmd_LED_finish((struct entry *) 0, self, argv, argvend));
}

char **cmd_LED_count(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	return(cmd_LED_finish(&modes[1], self, argv, argvend));
}

char **cmd_LED_rotate(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	return(cmd_LED_finish(&modes[2], self, argv, argvend));
}

char **cmd_LED_cylon(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	return(cmd_LED_finish(&modes[3], self, argv, argvend));
}

char **cmd_LED_finish(m, self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
	register struct entry *m;
{
	int interval = -1, mode = -1;
	register int status;

	if ((int) (m))
		mode = m->mode;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &interval) > 0)
		++argv;

	if (mode == -1 && interval == -1) {
			/*
			 * this will cause the token "lights" to be
			 */
		return(argv);
	}

	if (mode != -1 && interval == -1)	interval = m->def_interval;
	else if (interval < 1)			interval = 1;

	if (mode != -1) {
		status = ioctl(qvssfd, QIOCLEDPATTERN, &mode);
		if (status < 0) {
			printf("QIOCLEDPATTERN: status = %d.\n", status);
			perror("QIOCLEDPATTERN");
		}
	}

	if (interval != -1) {
		status = ioctl(qvssfd, QIOCLEDINTERVAL, &interval);
		if (status < 0) {
			printf("QIOCLEDINTERVAL: status = %d.\n", status);
			perror("QIOCLEDINTERVAL");
		}
	}
	return(argv);
}

/*
 * Random ioctl's
 */

char **cmd_lights(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return(++argv);
	}
	status = ioctl(qvssfd, QIOCLIGHT, &num);
	if (status < 0) {
		printf("QIOCLIGHT: status = %d.\n", status);
		perror("QIOCLIGHT");
	}
	if (debug)
		fprintf(stderr, "qvss: \"%s\":  arg \"%s\" == 0x%x == 0t%d returned %d.\n",
			self->cmd, *argv, num, num, status);

	return(++argv);
}

char **cmd_reverse_video(self, argv, argvend)
	struct cmd_entry *self;
	char **argv, **argvend;
{
	register int status;
	int num;

	if (getnum(argv < argvend ? *argv : "Too Few Args", &num) < 0) {
		fprintf(stderr, "qvss: \"%s\": bad argument \"%s\".\n",
			self->cmd, argv < argvend ? *argv : "Too Few Args");
		return(++argv);
	}
	status = ioctl(qvssfd, QIOCREVERSE, &num);
	if (status < 0) {
		printf("QIOCREVERSE: status = %d.\n", status);
		perror("QIOCREVERSE");
	}
	return(++argv);
}

/*
 * Flags
 */

parse_flags(str)
	char *str;
{
	register int flag;

	str++;
	while (flag = *str++) {
		switch (flag) {
		case 'd':
			debug = 1;
			break;
		}
	}
}

/*
 * 			The command table
 *
 * Note: The keys have to be lower case.  The typed command is converted to
 * lower case before the match is attempted.
 */

struct cmd_entry cmds[] = {

			/* tell all */
	{"help",	cmd_help,		0,
"\twhat you see"},

	{"model",	cmd_setup,		0,
"\tdefine model as 100 or 260"},

	{"reset",	cmd_reset,		0,
"\treset all qvss ioctl's to defaults"},

	{"fade",	cmd_fade,		0,
"\tfade screen after specified interval"},

	{"blink",	cmd_cursor_same,	0,
"\tboth the cursor \"on\" and cursor \"off\" interval are set the same"},

	{"both",	cmd_cursor_both,	0,
"\tboth the cursor \"on\" interval is set and cursor \"off\" interval is set"},

	{"cursor_on",	cmd_cursor_on,		0,
"set cursor \"on\" interval"},

	{"on",		cmd_cursor_on,		0,
"\tset cursor \"on\" interval"},

	{"cursor_off",	cmd_cursor_off,		0,
"set cursor \"on\" interval"},

	{"off",		cmd_cursor_off,		0,
"\tset cursor \"off\" interval"},

	{"icon",	cmd_cursor_icon,	0,
"\tset the cursor icon"},

	{"lights",	cmd_LED,		0,
"\tlights [off|count|rotate|cylon] [interval]\teither field is optional"},

	{"leds",	cmd_LED,		0,
"\tlights [off|count|rotate|cylon] [interval]\teither field is optional"},

	{"count",	cmd_LED_count,		0,
"\tLEDs == counter, incremented every [interval]"},

	{"rotate",	cmd_LED_rotate,		0,
"\tLEDs == shift register, rotated every [interval]"},

	{"cylon",	cmd_LED_cylon,		0,
"\tLEDs in cylon pattern every [interval]"},

	{"", 		0,		0,	""},

	{"lights",	cmd_lights,	0,
"\tsee avie"},

	{"reverse_video",cmd_reverse_video,0,
"\tenable reverse video"},


};
