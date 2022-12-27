# @(#)Makefile	1.4 86/10/28 NFSSRC
#
# 	Makefile	1.1	86/09/24
#
######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.1  89/06/20  10:36:00  dimitris
# 	Added async_daemon.
# 	[89/06/20  10:34:48  dimitris]
# 
# Revision 2.0  89/06/15  16:45:38  dimitris
#   Organized into a misc collection and SSPized
# 
# 
######################################################################

IBMRTOBJS		= getdomainname.o getfh.o nfssvc.o unmount.o async_daemon.o

include Makefile-common

${IBMRTOBJS}: $${@:.o=.c} SYS.h
	${CPP} -E ${CPPFLAGS} $*.c >$*.cpp.s
	${AS} -o $*.o $*.cpp.s
	${RM} -f $*.cpp.s
	${LD} -x -r $*.o
	${MV} -f a.out $*.o

${IBMRTOBJS:.o=_p.o}: $${@:_p.o=.c} SYS.h
	${CPP} -E ${CPPFLAGS} -DPROF ${@:_p.o=.c} >$*.cpp.s
	${AS} -o $*.o $*.cpp.s
	${RM} -f $*.cpp.s
	${LD} -x -r $*.o
	${MV} -f a.out $*.o

