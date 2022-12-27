######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.10  89/08/23  14:55:19  gm0w
# 	Replaced msg<xxx>.o modules with new supmsg.o module.
# 	[89/08/23            gm0w]
# 
# Revision 1.9  88/11/08  20:09:05  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /usr/cs/bin/
ETC_IDIR		= /usr/cs/etc/

SUPCL			= supcmain.o supcvers.o supcparse.o supcname.o \
			  supcmisc.o supcmeat.o
SUPS			= scm.o scmio.o stree.o log.o supmsg.o

PROGRAMS		= sup supfilesrv supscan

sup_OFILES		= ${SUPCL} ${SUPS}
supfilesrv_OFILES	= supfilesrv.o scan.o ${SUPS}
supfilesrv_LIBS		= -lsys
supfilesrv_IDIR		= ${ETC_IDIR}
supscan_OFILES		= supscan.o stree.o scan.o

include ../../../../Makefile-common

scan.o:	sup.h
scm.o:	sup.h
scmio.o: sup.h supmsg.h
stree.o: sup.h
supcmain.o: sup.h supmsg.h supcdefs.h
supcmeat.o: sup.h supmsg.h supcdefs.h
supcmisc.o: sup.h supmsg.h supcdefs.h
supcname.o: sup.h supmsg.h supcdefs.h
supcparse.o: sup.h supmsg.h supcdefs.h
supfilesrv.o: sup.h supmsg.h
supmsg.o: sup.h supmsg.h
supscan.o: sup.h
