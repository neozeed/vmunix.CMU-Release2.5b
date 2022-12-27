# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.4  89/06/23  16:26:57  mrt
# 	Upgraded for new make procedures.
# 	[89/05/24            mrt]
# 
# Revision 1.3  89/05/05  18:33:18  mrt
# 	Cleanup for Mach 2.5
# 
#
#
# "make interface CPDIR=<where you're at>" to make the User (and Server)
# side of an RPC interface for the service server.  This is called by 
# lib/libmach to get the interface into the mach library.
#
#  service should be installed as /etc/mach_init
#

INSTALL_DIR=${DSTBASE}/etc
TARGET_DIRS=${INSTALL_DIR}
LIBS=${LIBMACH}

PRGM = service
SRCS = service.c main.c

DEFS = service.defs
TEST_SRCS= test_service.o serviceUser.o

include $(SRCBASE)/Make.inc

all : test

test: ${TEST_SRCS} $(LIBS)
	${CC} $(_CCFLAGS_) ${TEST_SRCS} ${LIBS} -o test_service

-include $(DEP_FILE)
