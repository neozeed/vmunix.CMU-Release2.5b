# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.3  89/06/23  16:05:51  mrt
# 	upgrade for new make procedures
# 	[89/05/24            mrt]
# 
# Revision 1.2  89/05/05  18:28:50  mrt
# 	Cleanup for Mach 2.5
# 
#

#
# "make interface CPDIR=<where you're at>" to make the User (and Server)
# side of an RPC interface for the env_mgr.  This is called by lib/libmach
# to get the interface into the mach library.
#

INSTALL_DIR=$(DSTBASE)/etc
TARGET_DIRS=${INSTALL_DIR}
CPDIR= ""
COPY_H = emdefs.h


SRCS = envmgrmain.c envprocs.c
DEFS = env_mgr.defs
PRGM = envmgr
LIBS = $(LIBMACH)

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
