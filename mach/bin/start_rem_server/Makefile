# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.2  89/06/08  18:17:09  mrt
# 	upgraded to work with Makeconf
# 	[89/05/28            mrt]
# 
#

INSTALL_DIR=$(DSTBASE)/bin
TARGET_DIRS=$(INSTALL_DIR)
LIBS = $(LIBMACH)

PRGM= start_rem_server
SRCS= start_rem_server.c startproc.c uid.c

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
