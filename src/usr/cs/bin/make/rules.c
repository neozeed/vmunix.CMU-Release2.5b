#ifndef lint
static char *RCSid = "$Header: rules.c,v 5.2 89/08/03 18:34:16 mja Exp $";
#endif

/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 ************************************************************************
 * HISTORY
 * $Log:	rules.c,v $
 * Revision 5.2  89/08/03  18:34:16  mja
 * 	Add PMAX (mips) symbol definitions.
 * 	[89/05/06            mja]
 * 
 * Revision 5.1  89/05/12  19:11:07  bww
 * 	Converted the CPU, MACHINE, machine, CPUTYPE, and cputype
 * 	definitions into global fixed size arrays so that they could
 * 	be easily patched.
 * 	[89/05/12  19:10:51  bww]
 * 
 * Revision 5.0  89/03/01  01:41:08  bww
 * 	Version 5.
 * 	[89/03/01  01:33:21  bww]
 * 
 */

static	char *sccsid = "@(#)rules.c	3.1";

/*
 * DEFAULT RULES FOR UNIX
 *
 * These are the internal rules that "make" trucks around with it at
 * all times. One could completely delete this entire list and just
 * conventionally define a global "include" makefile which had these
 * rules in it. That would make the rules dynamically changeable
 * without recompiling make. This file may be modified to local
 * needs.
 */

#include "defs.h"

#ifdef pdp11
#	define this_CPU		"CPU=pdp11"
#	define this_MACHINE	"MACHINE=PDP11"
#	define this_machine	"machine=pdp11"
#	define this_CPUTYPE	"CPUTYPE=PDP11"
#	define this_cputype	"cputype=pdp11"
#endif
#ifdef vax
#	define this_CPU		"CPU=vax"
#	define this_MACHINE	"MACHINE=VAX"
#	define this_machine	"machine=vax"
#	define this_CPUTYPE	"CPUTYPE=VAX"
#	define this_cputype	"cputype=vax"
#endif
#ifdef sun
#ifdef sparc
#	define this_CPU		"CPU=sparc"
#	define this_MACHINE	"MACHINE=SUN4"
#	define this_machine	"machine=sun4"
#	define this_CPUTYPE	"CPUTYPE=SPARC"
#	define this_cputype	"cputype=sparc"
#else
#ifdef mc68020
#	define this_CPU		"CPU=mc68020"
#ifdef CMUCS
#	define this_MACHINE	"MACHINE=SUN"  /* SUN3 */
#	define this_machine	"machine=sun"  /* sun3 */
#else
#	define this_MACHINE	"MACHINE=SUN3"
#	define this_machine	"machine=sun3"
#endif
#	define this_CPUTYPE	"CPUTYPE=MC68020"
#	define this_cputype	"cputype=mc68020"
#else
#	define this_CPU		"CPU=mc68000"
#	define this_MACHINE	"MACHINE=SUN2"
#	define this_machine	"machine=sun2"
#	define this_CPUTYPE	"CPUTYPE=MC68000"
#	define this_cputype	"cputype=mc68000"
#endif
#endif
#endif
#ifdef ibm032
#	define this_CPU		"CPU=ibm032"
#	define this_MACHINE	"MACHINE=IBMRT"
#	define this_machine	"machine=ibmrt"
#	define this_CPUTYPE	"CPUTYPE=IBM032"
#	define this_cputype	"cputype=ibm032"
#endif
#ifdef ibm370
#	define this_CPU		"CPU=ibm370"
#	define this_MACHINE	"MACHINE=IBM"
#	define this_machine	"machine=ibm"
#	define this_CPUTYPE	"CPUTYPE=IBM370"
#	define this_cputype	"cputype=ibm370"
#endif
#ifdef ns32000
#ifdef balance
#	define this_CPU		"CPU=ns32032"
#	define this_MACHINE	"MACHINE=BALANCE"
#	define this_machine	"machine=balance"
#	define this_CPUTYPE	"CPUTYPE=NS32032"
#	define this_cputype	"cputype=ns32032"
#endif
#ifdef MULTIMAX
#	define this_CPU		"CPU=ns32032"
#	define this_MACHINE	"MACHINE=MMAX"
#	define this_machine	"machine=mmax"
#	define this_CPUTYPE	"CPUTYPE=NS32032"
#	define this_cputype	"cputype=ns32032"
#endif
#endif
#ifdef mips
#	define this_CPU		"CPU=mips"
#	define this_MACHINE	"MACHINE=PMAX"
#	define this_machine	"machine=pmax"
#	define this_CPUTYPE	"CPUTYPE=MIPS"
#	define this_cputype	"cputype=mips"
#endif

char CPU[32]		= this_CPU;	/* patch me */
char MACHINE[32]	= this_MACHINE;	/* patch me */
char machine[32]	= this_machine;	/* patch me */
char CPUTYPE[32]	= this_CPUTYPE;	/* patch me */
char cputype[32]	= this_cputype;	/* patch me */


char *builtin[] = {
#ifdef pwb
	".SUFFIXES : .L .out .o .c .f .e .r .y .yr .ye .l .s .z .x .t .h .cl",
#else
	".SUFFIXES : .out .o .s .c .F .f .e .r .y .yr .ye .l .p .sh .csh .h",
#endif

	/*
	 * PRESET VARIABLES
	 */
	"MAKE=make",
	"AR=ar",
	"ARFLAGS=",
	"RANLIB=ranlib",
	"LD=ld",
	"LDFLAGS=",
	"LINT=lint",
	"LINTFLAGS=",
#ifdef CMUCS
	"CO=rcsco",
#else
	"CO=co",
#endif
	"COFLAGS=-q",
	"CP=cp",
	"CPFLAGS=",
	"MV=mv",
	"MVFLAGS=",
	"RM=rm",
	"RMFLAGS=-f",
	"YACC=yacc",
	"YACCR=yacc -r",
	"YACCE=yacc -e",
	"YFLAGS=",
	"LEX=lex",
	"LFLAGS=",
	"CC=cc",
	"CFLAGS=",
	"AS=as",
	"ASFLAGS=",
	"PC=pc",
	"PFLAGS=",
	"RC=f77",
	"RFLAGS=",
	"EC=efl",
	"EFLAGS=",
	"FC=f77",
	"FFLAGS=",
	"LOADLIBES=",
#ifdef pwb
	"SCOMP=scomp",
	"SCFLAGS=",
	"CMDICT=cmdict",
	"CMFLAGS=",
#endif
	CPU,
	MACHINE,
	machine,
	CPUTYPE,
	cputype,

	/*
	 * SINGLE SUFFIX RULES
	 */
	".s :",
	"\t$(AS) $(ASFLAGS) -o $@ $<",

	".c :",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) $< $(LOADLIBES) -o $@",

	".F .f :",
	"\t$(FC) $(LDFLAGS) $(FFLAGS) $< $(LOADLIBES) -o $@",

	".e :",
	"\t$(EC) $(LDFLAGS) $(EFLAGS) $< $(LOADLIBES) -o $@",

	".r :",
	"\t$(RC) $(LDFLAGS) $(RFLAGS) $< $(LOADLIBES) -o $@",

	".p :",
	"\t$(PC) $(LDFLAGS) $(PFLAGS) $< $(LOADLIBES) -o $@",

	".y :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
	"\t$(RM) $(RMFLAGS) y.tab.c",

	".l :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
	"\t$(RM) $(RMFLAGS) lex.yy.c",

	".sh :",
	"\t$(CP) $(CPFLAGS) $< $@",
	"\tchmod +x $@",

	".csh :",
	"\t$(CP) $(CPFLAGS) $< $@",
	"\tchmod +x $@",

	".CO :",
	"\t$(CO) $(COFLAGS) $< $@",

	".CLEANUP :",
	"\t$(RM) $(RMFLAGS) $?",

	/*
	 * DOUBLE SUFFIX RULES
	 */
	".s.o :",
	"\t$(AS) -o $@ $<",

	".c.o :",
	"\t$(CC) $(CFLAGS) -c $<",

	".F.o .f.o :",
	"\t$(FC) $(FFLAGS) -c $<",

	".e.o :",
	"\t$(EC) $(EFLAGS) -c $<",

	".r.o :",
	"\t$(RC) $(RFLAGS) -c $<",

	".y.o :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(CFLAGS) -c y.tab.c",
	"\t$(RM) $(RMFLAGS) y.tab.c",
	"\t$(MV) $(MVFLAGS) y.tab.o $@",

	".yr.o:",
	"\t$(YACCR) $(YFLAGS) $<",
	"\t$(RC) $(RFLAGS) -c y.tab.r",
	"\t$(RM) $(RMFLAGS) y.tab.r",
	"\t$(MV) $(MVFLAGS) y.tab.o $@",

	".ye.o :",
	"\t$(YACCE) $(YFLAGS) $<",
	"\t$(EC) $(EFLAGS) -c y.tab.e",
	"\t$(RM) $(RMFLAGS) y.tab.e",
	"\t$(MV) $(MVFLAGS) y.tab.o $@",

	".l.o :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(CFLAGS) -c lex.yy.c",
	"\t$(RM) $(RMFLAGS) lex.yy.c",
	"\t$(MV) $(MVFLAGS) lex.yy.o $@",

	".p.o :",
	"\t$(PC) $(PFLAGS) -c $<",

#ifdef pwb
	".cl.o :",
	"\tclass -c $<",
#endif

	".y.c :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(MV) $(MVFLAGS) y.tab.c $@",

	".yr.r:",
	"\t$(YACCR) $(YFLAGS) $<",
	"\t$(MV) $(MVFLAGS) y.tab.r $@",

	".ye.e :",
	"\t$(YACCE) $(YFLAGS) $<",
	"\t$(MV) $(MVFLAGS) y.tab.e $@",

	".l.c :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(MV) $(MVFLAGS) lex.yy.c $@",

#ifdef pwb
	".o.L .c.L .t.L:",
	"\t$(SCOMP) $(SCFLAGS) $<",

	".t.o:",
	"\t$(SCOMP) $(SCFLAGS) -c $<",

	".t.c:",
	"\t$(SCOMP) $(SCFLAGS) -t $<",

	".h.z .t.z:",
	"\t$(CMDICT) $(CMFLAGS) $<",

	".h.x .t.x:",
	"\t$(CMDICT) $(CMFLAGS) -c $<",
#endif

	".o.out .s.out .c.out :",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) $< $(LOADLIBES) -o $@",

	".F.out .f.out :",
	"\t$(FC) $(LDFLAGS) $(FFLAGS) $< $(LOADLIBES) -o $@",

	".e.out :",
	"\t$(EC) $(LDFLAGS) $(EFLAGS) $< $(LOADLIBES) -o $@",

	".r.out :",
	"\t$(RC) $(LDFLAGS) $(RFLAGS) $< $(LOADLIBES) -o $@",

	".y.out :",
	"\t$(YACC) $(YFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
	"\t$(RM) $(RMFLAGS) y.tab.c",

	".l.out :",
	"\t$(LEX) $(LFLAGS) $<",
	"\t$(CC) $(LDFLAGS) $(CFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
	"\t$(RM) $(RMFLAGS) lex.yy.c",

	".p.out :",
	"\t$(PC) $(LDFLAGS) $(PFLAGS) $< $(LOADLIBES) -o $@",

	0
};
