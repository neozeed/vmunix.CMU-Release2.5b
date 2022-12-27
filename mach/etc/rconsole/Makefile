# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.3  89/06/23  16:25:10  mrt
# 	Upgraded to work with Makeconf
# 	[89/05/28            mrt]
# 
# Revision 1.2  89/05/05  18:33:05  mrt
# 	Cleanup for Mach 2.5
# 
#


ETCDIR=${DSTBASE}/etc
BINDIR=${DSTBASE}/bin
LIBDIR=${DSTBASE}/lib
TARGET_DIRS= $(ETCDIR) $(BINDIR) $(LIBDIR)

include $(SRCBASE)/Make.inc

RCONSOLE=	rconsole.o lookup.o
RCONSOLED=	rconsoled.o lookup.o


all: rconsole rconsoled

rconsole: ${RCONSOLE}
	$(CC) $(_CCFLAGS_) -o rconsole ${RCONSOLE}

rconsoled: ${RCONSOLED}
	$(CC) $(_CCFLAGS_) -o rconsoled ${RCONSOLED}



install: rconsole.install rconsoled.install rconsoles.install

rconsole.install: rconsole
	-@mach_install $I rconsole ${BINDIR}/rconsole
	-@touch rconsole.install

rconsoled.install: rconsoled
	-@mach_install $I rconsoled ${ETCDIR}/rconsoled
	-@touch rconsoled.install

rconsoles.install: rconsoles
	-@mach_install $I rconsoles ${LIBDIR}/rconsoles
	-@touch rconsoles.install

-include $(DEP_FILE)




