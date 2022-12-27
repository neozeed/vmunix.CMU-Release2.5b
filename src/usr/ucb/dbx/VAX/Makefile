######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.7  89/05/13  19:39:46  gm0w
# 	Cleaned up rules a bit.  Added more GARBAGE targets.
# 	[89/05/13            gm0w]
# 
# Revision 1.6  89/01/23  08:53:42  parker
# 	Modified for new and improved SSP.
# 	[89/01/23            parker]
# 
#
######################################################################

IDIR			= /usr/ucb/

PROGRAMS		= dbx

OFILES			= commands.o asm.o events.o c.o cerror.o check.o \
			  coredump.o debug.o eval.o fortran.o keywords.o \
			  languages.o library.o lists.o machine.o main.o \
			  mappings.o modula-2.o names.o object.o operators.o \
			  pascal.o printsym.o process.o runtime.o scanner.o \
			  source.o stabstring.o symbols.o tree.o ops.o date.o
HFILES			= defs.h

HDRS			= asm.h events.h c.h check.h coredump.h debug.h \
			  eval.h fortran.h keywords.h languages.h lists.h \
			  machine.h main.h mappings.h modula-2.h names.h \
			  object.h operators.h pascal.h printsym.h process.h \
			  runtime.h scanner.h source.h stabstring.h symbols.h \
			  tree.h ops.h

dbx_INCVERS		=
CFLAGS			= '-DMAINTAINER=""'
YFLAGS			= -d

GARBAGE			= makedefs makedefs.o y.tab.c y.tab.h date.c mkdate \
			  ${HDRS}

include ../../../Makefile-common

commands.o: ${HDRS}

symbols.o tree.o check.o eval.o events.o: operators.h

date.c:	mkdate version.ucb
	rm -f date.c version
	cp version.ucb version
	./mkdate > date.c
	rm -f version

${HDRS}: makedefs $${@:.h=.c}
	-./makedefs -f ${@:.h=.c} $@

makedefs: makedefs.o library.o cerror.o defs.h
	${_CC_} ${_CCFLAGS_} makedefs.o library.o cerror.o -o makedefs
