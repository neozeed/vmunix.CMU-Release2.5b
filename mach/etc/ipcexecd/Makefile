# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.3  89/06/23  16:12:06  mrt
# 	Upgraded for to use Makeconf
# 	[89/05/24            mrt]
# 
#
# 14-May-87 Mary Thompson at Carnegie Mellon
#   Added interface rule.
# "make interface CPDIR=<where you're at>" to make the User (and Server)
# side of an RPC interface for the env_mgr.  This is called by lib/libmach
# to get the interface into the mach library.
#

INSTALL_DIR=${DSTBASE}/etc
TARGET_DIRS=$(INSTALL_DIR)
LIBS=$(LIBMACH)

DEFS = ipcx.defs
SRCS = assert.c error.c main.c patch.c printmsg.c procs.c 
PRGM = ipcexecd

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
