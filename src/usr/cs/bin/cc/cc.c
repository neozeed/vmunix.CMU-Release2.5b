static	char sccsid[] = "@(#)cc.c 4.13 9/18/85";
/*
 * cc - front end for C compiler
 **********************************************************************
 * HISTORY
 * $Log:	cc.c,v $
 * Revision 1.14  89/07/31  11:08:24  moore
 * 	Removed default -Dhc for Metaware HighC compiler.
 * 	Fixed problems with variables named hc.
 * 	Still uses -D__HC__.
 * 	[89/07/31  11:07:58  moore]
 * 
 * Revision 1.13  89/07/28  12:17:22  moore
 * 	Corrected check on rt for pcc in argv[0].
 * 	[89/07/28  12:17:01  moore]
 * 
 * Revision 1.12  89/01/25  13:24:42  gm0w
 * 	To find hccom along the final lpath before parsing arguments
 * 	means that ALL lpath processing must be done first thing.
 * 	[89/01/25            gm0w]
 * 
 * Revision 1.11  89/01/25  11:56:27  gm0w
 * 	Added code to dynamically resize lpath.
 * 	[89/01/25            gm0w]
 * 
 * Revision 1.10  88/11/12  20:44:29  gm0w
 * 	Added -lfp_p on IBMRT's when profiling.  Added -L commands to
 * 	LPATH used by "cc" to search for programs and libraries to load.
 * 	[88/11/12            gm0w]
 * 
 * Revision 1.9  88/10/12  15:29:22  gm0w
 * 	Added code to use pcc if hccom is not found along LPATH.
 * 	[88/10/12            gm0w]
 * 
 * Revision 1.8  88/09/27  14:53:51  gm0w
 * 	Added code to support hc compiler as default and created pcc
 * 	link to invoke pcc.  This code is only applicable on the RT.
 * 	[88/09/25            gm0w]
 * 	
 * 	Removed /usr/cs/lib and . (current directory) from LPATH default
 * 	when LPATH is not in the environment.
 * 	[88/09/23            gm0w]
 * 
 * 27-Nov-87  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Added changes for inline for Sun.
 *
 * 30-Apr-87  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Fixed up f option for Sun.
 *
 * 07-Apr-87  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added code to pass -R switch to ccom on an RT.
 *
 * 04-Feb-87  Doug Philips (dwp) at Carnegie-Mellon University
 * 	Added proper optimizer switch if we are on an RT.
 *
 * 13-Dec-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Changed -N switch to -MD since -N is a switch to ld.  Changed
 *	suffix from .n to .d (.md might be more appropriate, but would
 *	require more significant changes).
 *
 * 21-Nov-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Merged in code for Sun.
 *
 * 06-May-86  Glenn Marcy (gm0w) at Carnegie-Mellon University
 *	Added -ES switch to CC to force .s files through cpp.
 *
 * 09-Apr-86  Robert V Baron (rvb) at Carnegie-Mellon University
 *	setsuf now sets the whole suffix -- not the last character.
 *
 * 04-Apr-86  Robert V Baron (rvb) at Carnegie-Mellon University
 *	Added N option -- this is generates a dependency file.
 *	N is something line M, but generates the file as a side effect
 *	and does not effect compilation
 *
 * 22-Feb-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Carried CS changes to 4.3 version.
 *
 * 30-Apr-85  Steven Shafer (sas) at Carnegie-Mellon University
 *	Adapted for CS collection.  Changed to exec all programs
 *	along LPATH search path.
 *
 **********************************************************************
 */
#include <sys/param.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/dir.h>
#if	CMUCS
#include <sys/file.h>
#endif	CMUCS

#if	CMUCS
char	*cpp = "cpp";
char	*ccom = "ccom";
char	*sccom = "sccom";
#if	ibmrt
char	*hccom = "hccom";
#endif	ibmrt
char	*c2 = "c2";
char	*as = "as";
char	*ld = "ld";
char	*crt0 = "crt0.o";
#else	CMUCS
char	*cpp = "/lib/cpp";
char	*ccom = "/lib/ccom";
char	*sccom = "/lib/sccom";
char	*c2 = "/lib/c2";
char	*as = "/bin/as";
char	*ld = "/bin/ld";
char	*crt0 = "/lib/crt0.o";
#endif	CMUCS

#if	CMUCS
char	tmp0[30];		/* big enough for /tmp/cc.%05.5d */
#else	CMUCS
char	tmp0[30];		/* big enough for /tmp/ctm%05.5d */
#endif	CMUCS
char	*tmp1, *tmp2, *tmp3, *tmp4, *tmp5;
char	*outfile;
char	*savestr(), *strspl(), *setsuf();
int	idexit();
char	**av, **clist, **llist, **plist;
int	cflag, eflag, oflag, pflag, sflag, wflag, Rflag, exflag, proflag;
int	fflag, gflag, Gflag, Mflag, debug;
char	*dflag;
int	exfail;
char	*chpass;
char	*npassname;
#if	CMUCS
char	*lpath;
int	lpath_len;
#if	ibmrt
char	**comlist;
int	ncom;
char	ON[] = "-on";
char	OFF[] = "-off";
char	ipath[1024] = "";
int	ansi_flag, cpp_flag, nocpp_flag, make_flag, verbose;
char	*index(), *rindex();
#endif	ibmrt
int	ESflag, MDflag;
char	*findlp(), *getenv(), *malloc(), *realloc();
#endif	CMUCS

int	nc, nl, np, nxo, na;

#define	cunlink(s)	if (s) unlink(s)

#if	sun
#include "SUN/cc.include"	/* Sun code */
#endif	sun

main(argc, argv)
	char **argv;
{
	char *t;
	char *assource;
	int i, j, c;
#if	CMUCS
#if	ibmrt
	int hc_flag = 1;
	char *parg;
#endif	ibmrt
#endif	CMUCS

#if	CMUCS
	lpath = malloc(1024);
	if (lpath == NULL) {
		error("lpath malloc failed", 0);
		exit(1);
	}
	*lpath = '\0';
	lpath_len = 1024;
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-' && argv[i][1] == 'L') {
			t = &argv[i][2];
			if (strlen(t)+strlen(lpath)+1 >= lpath_len) {
				lpath_len = strlen(t)+strlen(lpath)+2;
				lpath = realloc(lpath, lpath_len);
			}
			if (lpath == NULL) {
				error("lpath malloc failed", 0);
				exit(1);
			}
			strcat(lpath, t);
			strcat(lpath, ":");
		}
	}
	if ((t = getenv("LPATH")) == NULL)
		t = "/lib:/usr/lib";
	if (strlen(t)+strlen(lpath) >= lpath_len) {
		lpath_len = strlen(t)+strlen(lpath)+1;
		lpath = realloc(lpath, lpath_len);
	}
	if (lpath == NULL) {
		error("lpath malloc failed", 0);
		exit(1);
	}
	strcat(lpath, t);
#if	ibmrt
	if ((t = rindex(argv[0], '/')) == NULL)
	    t = argv[0];
	else
	    t++;
	if (strcmp(t, "pcc") == 0)
	    hc_flag = 0;
	else {
	    t = "hccom";
	    if (findlp(t) == t)
		hc_flag = 0;
	}
#endif	ibmrt
#endif	CMUCS
	/* ld currently adds upto 5 args; 10 is room to spare */
	av = (char **)calloc(argc*3+20, sizeof (char **));
	clist = (char **)calloc(argc, sizeof (char **));
	llist = (char **)calloc(argc, sizeof (char **));
#if	CMUCS
#if	ibmrt
	if (hc_flag) {
	    plist = (char **)calloc(argc+2, sizeof (char **));
	    comlist = (char **)calloc(argc*3+10, sizeof (char **));
	    plist[np++] = "-D__HC__";
	} else
#endif	ibmrt
#endif	CMUCS
	plist = (char **)calloc(argc, sizeof (char **));
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') switch (argv[i][1]) {
#if	CMUCS
#if	ibmrt
		case 'H': {
			int add_arg;

			if (!hc_flag)
			    break;
			if (argv[i][2] == '\0')
			    break;
			add_arg = 1;
			parg = argv[i];
			switch (parg[2]) {
			case 'a':
			    if (strcmp("ansi", parg+2) == 0)
				ansi_flag++;
			    break;
			case 'd':
			    if (strcmp("debug", parg+2) == 0) {
				add_arg = 0;
				gflag++;
			    }
			    break;
			case 'l':
			    if (strcmp("list", parg+2) == 0) {
				comlist[ncom++] = ON;
				comlist[ncom++] = parg+2;
				add_arg = 0;
			    }
			    break;
			case 'p':
			    if (strcmp("ppo",parg+2) == 0) {
				nocpp_flag++;
				cflag++;
			    }
			    break;
			case 'c':
			    if (strcmp("cpp", parg+2) == 0) {
				cpp_flag++;
				add_arg = 0;
			    }
			    break;
			case 'm':
			    if (strcmp("make", parg+2) == 0) {
				cflag++;
				comlist[ncom++] = "-lines";
				comlist[ncom++] = "0";
				make_flag++;
			    }
			    break;
			case 'n':
			    if (strcmp("nocpp",parg+2) == 0) {
				nocpp_flag++;
				add_arg = 0;
			    } else if (strcmp("noobj", parg+2) == 0)
				cflag++;
			    break;
			case 'v':
			    if (strcmp("volatile", parg+2) == 0) {
				add_arg = 0;
				comlist[ncom++] = "-off";
				comlist[ncom++] = "OPTIMIZE";
			    }
			    break;
			case '+':
			    if (strcmp("+w", parg+2) == 0) {
				add_arg = 0;
				comlist[ncom++] = OFF;
				comlist[ncom++] = "PCC_msgs";
			    }
			    break;
			}
			if (add_arg) {
			    char *p = index(parg+2, '=');
			    /*
			     * convert token "-Hxxx=yyy" to two tokens:
			     *   "-xxx yyy"
			     */
			    parg[1] = '-';
			    if (p != 0) {
				*p = '\0';
				comlist[ncom++] = parg+1;
				comlist[ncom++] = p+1;
			    } else
				comlist[ncom++] = parg+1;
			}
			continue;
		}
#endif	ibmrt
#endif	CMUCS

		case 'S':
			sflag++;
			cflag++;
			continue;
		case 'o':
			if (++i < argc) {
				outfile = argv[i];
				switch (getsuf(outfile)) {
#if	CMUCS
				case 's':
				case 'a':
#endif	CMUCS
				case 'c':
					error("-o would overwrite %s",
					    outfile);
					exit(8);
				}
			}
			continue;
		case 'R':
			Rflag++;
			continue;
		case 'O':
			oflag++;
			continue;
		case 'p':
			proflag++;
#if	CMUCS
			crt0 = "mcrt0.o";
#else	CMUCS
			crt0 = "/lib/mcrt0.o";
#endif	CMUCS
			if (argv[i][2] == 'g')
#if	CMUCS
				crt0 = "gcrt0.o";
#else	CMUCS
				crt0 = "/usr/lib/gcrt0.o";
#endif	CMUCS
			continue;
		case 'f':
#if	sun
			DO_FPOPTS()
#else	sun
			fflag++;
#endif	sun
			continue;
		case 'g':
			if (argv[i][2] == 'o') {
#if	CMUCS
#if	ibmrt
			    if (hc_flag)
				fprintf(stderr,"-go: (warning) the obsolete debugger sdb is not supported\n");
			    else
#endif	ibmrt
#endif	CMUCS
			    Gflag++;	/* old format for -go */
			} else {
			    gflag++;	/* new format for -g */
			}
			continue;
		case 'w':
			wflag++;
			continue;
		case 'E':
			exflag++;
#if	CMUCS
			if (argv[i][2] == 'S')
				ESflag++;
#endif	CMUCS
		case 'P':
			pflag++;
			if (argv[i][1]=='P')
				fprintf(stderr,
	"cc: warning: -P option obsolete; you should use -E instead\n");
			plist[np++] = argv[i];
		case 'c':
			cflag++;
			continue;
		case 'M':
#if	CMUCS
			if (argv[i][2] == 'D') {
				MDflag++;
				plist[np++] = argv[i];
				continue;
			}
#endif	CMUCS
			exflag++;
			pflag++;
			Mflag++;
			/* and fall through */
		case 'D':
		case 'I':
		case 'U':
		case 'C':
			plist[np++] = argv[i];
#if	CMUCS
#if	ibmrt
			if (!hc_flag)
				continue;
			parg = argv[i];
			if (parg[1] == 'I') {
				if (ipath[0] != '\0')
					strcat(ipath, ":");
				strcat(ipath, parg+2);
				strcat(ipath, "/");
			} else if (argv[i][1] == 'D') {
				comlist[ncom++] = "-define";
				t = index(parg+2, '=');
				if (t != NULL) {
					*t = ' ';
					comlist[ncom++] = savestr(parg+2);
					*t = '=';
				} else {
					char buf[128];

					strcpy(buf, parg+2);
					strcat(buf, " 1");
					comlist[ncom++] = savestr(buf);
				}
			}
#endif	ibmrt
#endif	CMUCS
			continue;
		case 'L':
			llist[nl++] = argv[i];
			continue;
		case 't':
			if (chpass)
				error("-t overwrites earlier option", 0);
			chpass = argv[i]+2;
			if (chpass[0]==0)
				chpass = "012p";
			continue;
		case 'B':
			if (npassname)
				error("-B overwrites earlier option", 0);
			npassname = argv[i]+2;
			if (npassname[0]==0)
				npassname = "/usr/c/o";
			continue;
		case 'd':
			if (argv[i][2] == '\0') {
				debug++;
				continue;
			}
			dflag = argv[i];
			continue;
#if	CMUCS
#if	ibmrt
		case 'v':
			debug++;
			continue;
#endif	ibmrt
		case 'm':
			if (argv[i][2] == '\0')
				break;
#if	ibmrt
			if (hc_flag &&
			    argv[i][2] == 'a' && argv[i][3] == '\0') {
				if (!make_flag) {
					comlist[ncom++] = ON;
					comlist[ncom++] = "alloca";
				}
				continue;
			}
#endif	ibmrt
#if	sun
			if (argv[i][2] != '\0') {
				DO_MACHOPTS()
				continue;
			}
#endif	sun
		case 'A':
			if (argv[i][2] == '\0')
				break;
#if	sun
			/*
			 *	Pass assembler flags like "-A-20"
			 */
			if (argv[i][2] != '\0')
				continue;
#endif	sun
#endif	CMUCS
		}
#if	CMUCS
#if	ibmrt
		else if (hc_flag && *argv[i] == '+' && argv[i][1] == 'w' &&
			 argv[i][2] == '\0') {
		    comlist[ncom++] = OFF;
		    comlist[ncom++] = "PCC_msgs";
		}
#endif	ibmrt
#endif	CMUCS
		t = argv[i];
		c = getsuf(t);
		if (c=='c' || c=='s' || exflag) {
			clist[nc++] = t;
			t = setsuf(t, 'o');
		}
#if	sun
		else if (strcmp(getsufstr(t), "il") == 0)
			iflag++;
		if (!iflag && nodup(llist, t)) {
#else	sun
		if (nodup(llist, t)) {
#endif	sun
			llist[nl++] = t;
			if (getsuf(t)=='o')
				nxo++;
		}
	}
	if (gflag || Gflag) {
		if (oflag)
			fprintf(stderr, "cc: warning: -g disables -O\n");
		oflag = 0;
	}
#if	sun
	/*
	 *	Set default machine and/or floating point types
	 *	if they haven't been specified
	 */
	SETDEFAULTS()
#endif	sun
	if (npassname && chpass ==0)
		chpass = "012p";
	if (chpass && npassname==0)
		npassname = "/usr/new";
	if (chpass)
	for (t=chpass; *t; t++) {
		switch (*t) {

		case '0':
			if (fflag)
				sccom = strspl(npassname, "sccom");
			else
				ccom = strspl(npassname, "ccom");
			continue;
		case '2':
			c2 = strspl(npassname, "c2");
			continue;
		case 'p':
			cpp = strspl(npassname, "cpp");
			continue;
		}
	}
	if (nc==0)
		goto nocom;
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, idexit);
#if	CMUCS
	sprintf(tmp0, "/tmp/cc.%05.5d", getpid());
#else	CMUCS
	if (pflag==0)
		sprintf(tmp0, "/tmp/ctm%05.5d", getpid());
#endif	CMUCS
	tmp1 = strspl(tmp0, "1");
	tmp2 = strspl(tmp0, "2");
#if	sun
	tmp2_5 = strspl(tmp0, "2_5");	/* inline input file */
#endif	sun
	tmp3 = strspl(tmp0, "3");
	if (pflag==0)
		tmp4 = strspl(tmp0, "4");
	if (oflag)
		tmp5 = strspl(tmp0, "5");
#if	CMUCS
#if	ibmrt
	if (hc_flag) {
		register char *cpath, *p, *q;

		if ((cpath = getenv("CPATH")) == NULL)
			cpath = "/usr/include";
		p = ipath + strlen(ipath);
		do {
			*p++ = ':';
			q = cpath;
			while (*q != '\0' && *q != ':')
				*p++ = *q++;
			if (q == cpath)
				*p++ = '.';
			*p++ = '/';
			cpath = ((*q) ? q + 1 : q);
		} while (*q != '\0');
		*p = '\0';
	}
#endif	ibmrt
#endif	CMUCS
	for (i=0; i<nc; i++) {
		if (nc > 1 && !Mflag) {
			printf("%s:\n", clist[i]);
			fflush(stdout);
		}
#if	CMUCS
		if (!Mflag && !ESflag && getsuf(clist[i]) == 's') {
#else	CMUCS
		if (!Mflag && getsuf(clist[i]) == 's') {
#endif	CMUCS
			assource = clist[i];
			goto assemble;
		} else
			assource = tmp3;
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		av[0] = "cpp"; av[1] = clist[i];
		na = 2;
		if (!exflag)
			av[na++] = tmp4;
#if	CMUCS
		else if (MDflag)
			av[na++] = "-";
		if (MDflag)
			av[na++] = setsuf(clist[i], 'd');
#endif	CMUCS
		for (j = 0; j < np; j++)
			av[na++] = plist[j];
		av[na++] = 0;
#if	CMUCS
		if (calllib(cpp, av)) {
#else	CMUCS
		if (callsys(cpp, av)) {
#endif	CMUCS
			exfail++;
			eflag++;
		}
		if (pflag || exfail) {
			cflag++;
			continue;
		}
		if (sflag) {
			if (nc==1 && outfile)
				tmp3 = outfile;
			else
				tmp3 = setsuf(clist[i], 's');
			assource = tmp3;
		}
#if	CMUCS
#if	ibmrt
		if (hc_flag) {
		    av[0] = "hccom";
		    av[1] = tmp4; na = 2;
		    av[na++] = "-efile";
		    av[na++] = "@E";
		    av[na++] = "-silent";
		    av[na++] = "-fn";
		    if (sflag || (Rflag && !make_flag)) {
			av[na++] = "-obj";
			av[na++] = tmp2;
			av[na++] = "-S";
			av[na++] = tmp3;
		    } else {
			av[na++] = "-obj";
			if (cflag && nc==1 && outfile)
			    av[na++] = outfile;
			else
			    av[na++] = setsuf(clist[i], 'o');
		    }
		    if (proflag) {
			av[na++] = ON;
			av[na++] = "PROFILE";
		    }
		    av[na++] = "-ipath";
		    av[na++] = ipath;
		    if (wflag){
			av[na++] = OFF;
			av[na++] = "warn";
		    }
		    if (Rflag && !make_flag) {
			av[na++] = ON;
			av[na++] = "READ_ONLY_STRINGS";
		    }
		    /*
		     * -g turns cross jumping optimization off by
		     * default. We also want to permit unreferenced
		     * local routines to exists so that they can be
		     * invoked from the debugger. If "-O" specified,
		     * we turn cross-jumping back on.
		     */
		    if (gflag && !make_flag) {
			av[na++] = "-debug";
			av[na++] = ON;
			if (oflag)
			    av[na++] = "OPTIMIZE_XJMP";
			else
			    av[na++] = "KEEP_UNREFERENCED_ROUTINES";
		    }
		    /*
		     * Turn on long enums by default.  May be overriden
		     * by explicit command line switch.
		     */
		    av[na++] = "-on";
		    av[na++] = "long_enums";
		    for (j = 0; j < ncom; j++)
			av[na++] = comlist[j];
		    av[na++] = "-anno";
		    av[na++] = clist[i];
		    if (ansi_flag){
			av[na++] = "-st";
			av[na++] = findlp("hcansi.st");
			av[na++] = "-pt";
			av[na++] = findlp("hcansi.pt");
			av[na++] = "-spt";
			av[na++] = findlp("hcansip.pt");
		    }
		    if (!fflag && !make_flag){
			av[na++] = ON;
			av[na++] = "DOUBLE_MATH_ONLY";
		    }
		    av[na] = 0;
		    if (calllib(hccom, av)) {
			cflag++;
			eflag++;
			continue;
		    }
		    if (sflag || !Rflag)
			continue;
		    goto assemble;
		}
#endif	ibmrt
#endif	CMUCS
		av[0] = fflag ? "sccom" : "ccom";
#if	sun
		av[1] = tmp4; av[2] = iflag ? tmp2_5 : oflag?tmp5:tmp3; na = 3;
#else	sun
		av[1] = tmp4; av[2] = oflag?tmp5:tmp3; na = 3;
#endif	sun
		if (proflag)
			av[na++] = "-XP";
		if (gflag) {
			av[na++] = "-Xg";
		} else if (Gflag) {
			av[na++] = "-XG";
		}
		if (wflag)
			av[na++] = "-w";
#if	ibmrt
		if (Rflag)
			av[na++] = "-XR";
#endif	ibmrt
#if	sun
		PASS_CCOMOPTS()
#endif	sun
		av[na] = 0;
#if	CMUCS
		if (calllib(fflag ? sccom : ccom, av)) {
#else	CMUCS
		if (callsys(fflag ? sccom : ccom, av)) {
#endif	CMUCS
			cflag++;
			eflag++;
			continue;
		}
#if	sun
		if (iflag)
			DO_INLINE()
#endif	sun
		if (oflag) {
			av[0] = "c2"; av[1] = tmp5; av[2] = tmp3; av[3] = 0;
#if	ibmrt
			av[3] = "-a"; av[4] = 0;
#endif	ibmrt
#if	sun
			PASS_OPTIMOPTS()
#endif	sun
#if	CMUCS
			if (calllib(c2, av)) {
#else	CMUCS
			if (callsys(c2, av)) {
#endif	CMUCS
				unlink(tmp3);
				tmp3 = assource = tmp5;
			} else
				unlink(tmp5);
		}
		if (sflag)
			continue;
	assemble:
		cunlink(tmp1); cunlink(tmp2); cunlink(tmp4);
		av[0] = "as"; av[1] = "-o";
		if (cflag && nc==1 && outfile)
			av[2] = outfile;
		else
			av[2] = setsuf(clist[i], 'o');
		na = 3;
		if (Rflag)
			av[na++] = "-R";
		if (dflag)
			av[na++] = dflag;
#if	sun
		PASS_ASOPTS()
#endif	sun
		av[na++] = assource;
		av[na] = 0;
		if (callsys(as, av) > 1) {
			cflag++;
			eflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;
#if	CMUCS
		crt0 = findlp(crt0);
#endif	CMUCS
		av[0] = "ld"; av[1] = "-X"; av[2] = crt0; na = 3;
#if	sun
		PASS_LDOPTS()
#endif	sun
		if (outfile) {
			av[na++] = "-o";
			av[na++] = outfile;
		}
		while (i < nl)
			av[na++] = llist[i++];
		if (gflag || Gflag)
			av[na++] = "-lg";
		if (proflag)
#if	ibmrt
		{
			av[na++] = "-lfp_p";
#endif
			av[na++] = "-lc_p";
#if	ibmrt
		}
#endif
		else
			av[na++] = "-lc";
		av[na++] = 0;
		eflag |= callsys(ld, av);
		if (nc==1 && nxo==1 && eflag==0)
			unlink(setsuf(clist[0], 'o'));
	}
	dexit();
}

idexit()
{

	eflag = 100;
	dexit();
}

dexit()
{

	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp4);
		cunlink(tmp5);
	}
	exit(eflag);
}

error(s, x)
	char *s, *x;
{
	FILE *diag = exflag ? stderr : stdout;

	fprintf(diag, "cc: ");
	fprintf(diag, s, x);
	putc('\n', diag);
	exfail++;
	cflag++;
	eflag++;
}

getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while (t = *s++)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
	if (c <= MAXNAMLEN && c > 2 && *s++ == '.')
		return (*s);
	return (0);
}

char *
setsuf(as, ch)
	char *as;
{
	register char *s, *s1;

	s = s1 = savestr(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;
#if	CMUCS
	while (*--s != '.' && s > s1);
	s[1] = ch;
	s[2] = 0;
#else	CMUCS
	s[-1] = ch;
#endif	CMUCS
	return (s1);
}

callsys(f, v)
	char *f, **v;
{
	int t, status;
	char **cpp;

	if (debug) {
		fprintf(stderr, "%s:", f);
		for (cpp = v; *cpp != 0; cpp++)
			fprintf(stderr, " %s", *cpp);
		fprintf(stderr, "\n");
	}
	t = vfork();
	if (t == -1) {
		printf("No more processes\n");
		return (100);
	}
	if (t == 0) {
#if	CMUCS
		execvp(f, v);
#else	CMUCS
		execv(f, v);
#endif	CMUCS
		printf("Can't find %s\n", f);
		fflush(stdout);
		_exit(100);
	}
	while (t != wait(&status))
		;
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2) {
			printf("Fatal error in %s\n", f);
			eflag = 8;
		}
		dexit();
	}
	return ((status>>8) & 0377);
}

nodup(l, os)
	char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	if (getsuf(s) != 'o')
		return (1);
	while (t = *l++) {
		while (c = *s++)
			if (c != *t++)
				break;
		if (*t==0 && c==0)
			return (0);
		s = os;
	}
	return (1);
}

#define	NSAVETAB	1024
char	*savetab;
int	saveleft;

char *
savestr(cp)
	register char *cp;
{
	register int len;

	len = strlen(cp) + 1;
	if (len > saveleft) {
		saveleft = NSAVETAB;
		if (len > saveleft)
			saveleft = len;
		savetab = (char *)malloc(saveleft);
		if (savetab == 0) {
			fprintf(stderr, "ran out of memory (savestr)\n");
			exit(1);
		}
	}
	strncpy(savetab, cp, len);
	cp = savetab;
	savetab += len;
	saveleft -= len;
	return (cp);
}

char *
strspl(left, right)
	char *left, *right;
{
	char buf[BUFSIZ];

	strcpy(buf, left);
	strcat(buf, right);
	return (savestr(buf));
}

#if	CMUCS
static
searchp (path,file,fullname,func)
char *path,*file,*fullname;
int (*func)();
{
	register char *nextpath,*nextchar,*fname,*lastchar;
	int failure;

	nextpath = ((*file == '/') ? "" : path);
	do {
		fname = fullname;
		nextchar = nextpath;
		while (*nextchar && (*nextchar != ':'))
			*fname++ = *nextchar++;
		if (nextchar != nextpath && *file) *fname++ = '/';
		lastchar = nextchar;
		nextpath = ((*nextchar) ? nextchar + 1 : nextchar);
		nextchar = file;	/* append file */
		while (*nextchar)  *fname++ = *nextchar++;
		*fname = '\0';
		failure = (*func) (fullname);
	} 
	while (failure && (*lastchar));
	return (failure ? -1 : 0);
}

static
openlp(fnam)
char *fnam;
{
	int fd;

	if ((fd = open(fnam, O_RDONLY, 0)) < 0)
		return(1);
	close(fd);
	return(0);
}

char *findlp(f)
char *f;
{
	char fullname[256];

	if (searchp(lpath, f, fullname, openlp) < 0)
		return(f);
	return(savestr(fullname));
}

static char **vsave;

func(f)
char *f;
{
	execv(f, vsave);
	return(1);
}

execvlp(f, v)
	char *f, **v;
{
	char fullname[256];

	vsave = v;
	searchp(lpath, f, fullname, func);
}

calllib(f, v)
	char *f, **v;
{
	int t, status;
	char **cpp;

	if (debug) {
		fprintf(stderr, "%s:", f);
		for (cpp = v; *cpp != 0; cpp++)
			fprintf(stderr, " %s", *cpp);
		fprintf(stderr, "\n");
	}
	t = vfork();
	if (t == -1) {
		printf("No more processes\n");
		return (100);
	}
	if (t == 0) {
		execvlp(f, v);
		printf("Can't find %s\n", f);
		fflush(stdout);
		_exit(100);
	}
	while (t != wait(&status))
		;
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2) {
			printf("Fatal error in %s\n", f);
			eflag = 8;
		}
		dexit();
	}
	return ((status>>8) & 0377);
}
#endif	CMUCS
