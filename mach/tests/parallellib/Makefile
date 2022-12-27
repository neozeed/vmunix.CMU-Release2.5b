# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 2.1  89/08/29  23:16:04  mrt
# Created.
# 

INSTALL_DIR=${DSTBASE}/tests
TARGET_DIRS=$(INSTALL_DIR)
LIBC=$(DSTBASE)/parallel/lib/libc.a
LIBS=$(LIBTHREADS) $(LIBMACH) $(LIBC) $(LIBTHREADS) $(LIBMACH)

CFLAGS = -DDEBUG -I$(DSTBASE)/parallel/include

CSH=test_out.csh
PRGM1L= cc_threads cc_cthreads multi_out lock_file


include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
