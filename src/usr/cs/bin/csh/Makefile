######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.6  89/08/15  15:12:19  mja
# 	Eliminate all machine dependent rules.  These are now found in
# 	Makefile-machdep (which obsoletes RULES file).
# 
# Revision 2.5  89/07/17  15:51:20  mbj
# 	Moved doprnt.o rule to optional machine specific RULES file.
# 	Only build doprnt.o if included in ${MACHINE}_OFILES variable.
# 	[89/07/17  15:46:10  mbj]
# 
# Revision 2.4  89/05/13  19:38:55  gm0w
# 	Updated to use new XSTR support in Makefile-common.
# 	[89/05/13            gm0w]
# 
# Revision 2.3  88/11/08  19:53:32  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /usr/cs/bin/

PROGRAMS		= csh

CFLAGS			= -DTELL -DVFORK
XSTR_OFILES		= alloc_x.o printf_x.o sh_x.o sh.char_x.o \
			  sh.dir_x.o sh.dol_x.o sh.err_x.o sh.exec_x.o \
			  sh.exp_x.o sh.file_x.o sh.func_x.o \
			  sh.glob_x.o sh.hist_x.o sh.init_x.o \
			  sh.lex_x.o sh.misc_x.o sh.parse_x.o \
			  sh.print_x.o sh.proc_x.o sh.sem_x.o \
			  sh.set_x.o sh.time_x.o tenex_x.o dosetpath_x.o
OFILES			= ${XSTR_OFILES} strings.o
LINTFILES		= ${XSTR_OFILES:_x.o=.c}
GARBAGE			= ${XSTR_OFILES:.o=.c} strings.c strings x.c xs.c

LIBS			= -ltermcap

sh.char_x.o_CFLAGS	= ${CFLAGS} -R
sh.init_x.o_CFLAGS	= ${CFLAGS} -R
strings.o_CFLAGS	= ${CFLAGS} -R

include ../../../../Makefile-common

# strings.o, sh.init.o, and sh.char.o are specially processed to be shared
strings.c:
	${XSTR}
	${MV} -f xs.c strings.c

sh_x.o: sh.h sh.local.h sh.char.h
sh.char_x.o: sh.char.h
sh.dir_x.o: sh.h sh.local.h sh.dir.h
sh.dol_x.o: sh.h sh.local.h sh.char.h
sh.err_x.o: sh.h sh.local.h sh.char.h
sh.exec_x.o: sh.h sh.local.h sh.char.h
sh.exp_x.o: sh.h sh.local.h sh.char.h
sh.file_x.o: sh.h sh.local.h sh.char.h
sh.func_x.o: sh.h sh.local.h sh.char.h
sh.glob_x.o: sh.h sh.local.h sh.char.h
sh.hist_x.o: sh.h sh.local.h sh.char.h
sh.init_x.o: sh.local.h
sh.lex_x.o: sh.h sh.local.h sh.char.h
sh.misc_x.o: sh.h sh.local.h sh.char.h
sh.parse_x.o: sh.h sh.local.h sh.char.h
sh.print_x.o: sh.h sh.local.h sh.char.h
sh.proc_x.o: sh.h sh.local.h sh.dir.h sh.proc.h sh.char.h
sh.sem_x.o: sh.h sh.local.h sh.proc.h sh.char.h
sh.set_x.o: sh.h sh.local.h sh.char.h
sh.time_x.o: sh.h sh.local.h sh.char.h
tenex_x.o: sh.h sh.local.h sh.char.h
