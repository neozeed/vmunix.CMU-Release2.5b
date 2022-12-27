# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.3  89/06/08  18:11:37  mrt
# 	Removed definition of SRCBASE and DSTBASE as they are now 
# 	defined in Makeconf.
# 	[89/05/19            mrt]
# 
# Revision 1.2  89/05/05  18:25:50  mrt
# 	Cleanup for Mach 2.5
# 
#


INSTALL_DIR=$(DSTBASE)/bin
TARGET_DIRS=${INSTALL_DIR}
LIBS = $(LIBMACH)

SRCS= qvss.c cmds.c
PRGM= qvss

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
