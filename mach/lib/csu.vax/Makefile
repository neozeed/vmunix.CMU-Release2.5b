# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.3  89/05/05  18:37:49  mrt
# 	Cleanup for Mach 2.5
# 
#
#  23-Feb-87   Mary Thompson
#	set MCRTO and CRT0 switches to cc for crt0
#	and moncrt0

# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4 (Berkeley) 9/5/85
#
#	crt0	Normal C run time startoff
#	mcrt0	C run time start off for profiling, ``prof'' conventions
#	gcrt0	C run time start off for profiling, ``gprof'' conventions
#


DSTBASE=/usr/mach
DIR=${DSTBASE}/lib

I = -c -d

SRCS=	crt0.c mon.c gmon.c
OBJS=	crt0.o mcrt0.o gcrt0.o mon.o gmon.o


all: ${OBJS}

crt0.o:	crt0.c
	cc -S  -DCRT0 -DMACH crt0.c
	/lib/cpp crt0.s > x.s
	as -o x.o x.s
	ld -x -r -o crt0.o x.o
	rm -f x.s x.o crt0.s

moncrt0.o: crt0.c
	cc -S -DMCRT0 -DMACH crt0.c
	/lib/cpp crt0.s > x.s
	as -o x.o x.s
	ld -x -r -o moncrt0.o x.o
	rm -f x.s x.o crt0.s

gcrt0.o: moncrt0.o gmon.o
	ld -x -r -o gcrt0.o moncrt0.o gmon.o

mcrt0.o: moncrt0.o mon.o
	ld -x -r -o mcrt0.o moncrt0.o mon.o

mon.o: mon.c mon.ex
	cc -S  mon.c
	ex - mon.s < mon.ex
	as -o x.o mon.s
	ld -x -r -o mon.o x.o
	rm -f x.o mon.s

gmon.o: gmon.c gmon.h gmon.ex
	cc -S  gmon.c
	ex - gmon.s < gmon.ex
	as -o x.o gmon.s
	ld -x -r -o gmon.o x.o
	rm -f x.o gmon.s




install: ${OBJS}
	-@mach_install $I crt0.o ${DIR}/crt0.o
	-@mach_install $I mcrt0.o ${DIR}/mcrt0.o
	-@mach_install $I gcrt0.o ${DIR}/gcrt0.o




