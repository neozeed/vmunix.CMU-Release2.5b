# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.6  89/08/24  17:47:56  mrt
# 	Cleanup
# 
# Revision 1.5  89/06/23  16:14:26  mrt
# 	upgraded for new make procedures
# 	[89/05/24            mrt]
# 
# Revision 1.4  89/05/05  18:31:22  mrt
# 	 13-Oct-88 Mary Thompson (mrt) at Carnegie Mellon
# 	 	Removed memory_object.defs as it is now is libmach.a
# 
#
#  11-Jul-88 Joseph Barrera (jsb) at Carnegie-Mellon University
#	Created.

INSTALL_DIR = $(DSTBASE)/etc
TARGET_DIRS = $(INSTALL_DIR)


INCS = netmemory.h netmemory_defs.h
SRCS = netmemory.c object.c scheduler.c netmemory_aux.c netname_aux.c
DEFS = internal.defs netmemory.defs netmemstat.defs 
PRGM = netmemoryserver
LIBS = internalUser.o $(LIBTHREADS) $(LIBMACH)

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
