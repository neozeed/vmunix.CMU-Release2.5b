######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.8  88/11/08  21:56:09  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /bin/

FILES			= awk.lx.o b.o main.o token.o tran.o lib.o\
			  run.o parse.o proctab.o freeze.o

PROGRAMS		= awk
OFILES			= awk.g.o ${FILES}
YFLAGS			= -d
LIBS			= -lm
GARBAGE			= awk.h token.c proc proc.o proctab.c\
			  y.tab.c y.tab.h yacc* temp

include ../../Makefile-common

awk.g.o: awk.def
proc.o:	awk.h
y.tab.h: awk.g.o
awk.h:	y.tab.h
	cp y.tab.h awk.h
$(FILES): awk.h awk.def
token.c: awk.h tokenscript token.c.ucb
	cat token.c.ucb > token.c
	ex - < tokenscript
proctab.c: proc
	./proc > proctab.c
proc:	awk.h proc.o token.o
	cc -o proc proc.o token.o
