#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# Copyright (c) 1988 Carnegie-Mellon University
# Copyright (c) 1987 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.5  89/06/07  11:18:03  mrt
# 	Upgraded to work with Makeconf
# 	[89/05/28            mrt]
# 
# Revision 1.4  89/05/05  18:32:56  mrt
# 	Changed CFLAGS to _CCFLAGS_ to pick up all the flags.
# 	[89/05/04            mrt]
# 
# Revision 1.3  89/05/02  11:20:00  dpj
# 	Fixed all files to conform to standard copyright/log format
# 
# Revision 1.2  88/10/25  17:32:27  mrt
# added VPATH=../build and changed logstat_user to logstatUser
# 
# Revision 1.1  88/10/14  16:23:37  mrt
# Initial revision
# 
   
#
# $Header: Makefile,v 1.5 89/06/07 11:18:03 mrt Exp $
#
#  Makefile for netmsgserver 
#
#

INSTALL_DIR = $(DSTBASE)/tests
TARGET_DIRS = $(INSTALL_DIR)
LIBS = $(LIBTHREADS) $(LIBMACH)

VPATH=../server:../conf
LPATH=../lib:${LPATH}
PRGM1 = nmxlog
PRGM1L =  nmtest nmtime

nmcontrol: nmcontrol.o logstatUser.o ls_rec.o
	$(CC) $(_CCFLAGS_) -o nmcontrol nmcontrol.o logstatUser.o ls_rec.o -lmach

nmxportstat: nmxportstat.o ls_rec.o
	$(CC) $(_CCFLAGS_) -o nmxportstat nmxportstat.o

vmtptest: vmtptest.o
	$(CC) $(_CCFLAGS_) -o vmtptest vmtptest.o -lthreads -lmach ../lib/libvmtp.a


include $(SRCBASE)/Make.inc


all: nmcontrol nmxportstat

-include $(DEP_FILE)
