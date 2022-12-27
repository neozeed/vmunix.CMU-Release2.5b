# @(#)Makefile	1.4 86/10/28 NFSSRC
#
# 	Makefile	1.1	86/09/24
#
######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.1  89/06/19  17:17:50  dimitris
# 	Added async_daemon.
# 	[89/06/19  17:16:44  dimitris]
# 
# Revision 2.0  89/06/15  17:05:48  dimitris
#   Organized into a misc collection and SSPized
# 
# 
######################################################################

VAXOBJS			= getdomainname.o getfh.o nfssvc.o unmount.o async_daemon.o

include Makefile-common

${VAXOBJS}: $${@:.o=.c} SYS.h
	${CPP} -E ${CPPFLAGS} $*.c | ${AS} -o $*.o
	${LD} -x -r $*.o
	${MV} a.out $*.o

${VAXOBJS:.o=_p.o}: $${@:_p.o=.c} SYS.h
	${CPP} -E ${CPPFLAGS} -DPROF ${@:_p.o=.c} | ${AS} -o $*.o
	${LD} -x -r $*.o
	${MV} a.out $*.o
