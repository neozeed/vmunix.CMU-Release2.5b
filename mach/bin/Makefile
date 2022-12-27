# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.6.1.1  89/08/28  17:26:00  mrt
# 	Removed zprint for the distribution as it does not
# 	work well enough.
# 	[89/08/28            mrt]
# 
# Revision 1.6  89/07/14  16:59:55  mrt
# 	Added special rules for md so that it would not need to use
# 	md.
# 	[89/07/04            mrt]
# 
# Revision 1.5  89/06/08  18:05:57  mrt
# 	Upgraded to use Makeconf
# 	[89/05/28            mrt]
# 
# 	Added zprint to PRGM1L list. 
# 	[89/05/16            mrt]
# 
# Revision 1.4  89/05/05  17:40:17  mrt
# 	Removed zprint.c as it no longer compiles. That was the
# 	last of the KMEM programs so I removed all that stuff.
# 	A new debugging interface for zprint should appear soon.
# 	[89/05/05            mrt]
# 
#
# 30-Aug-88 Mary Thompson
#	Removed vmpinfo, vmmprint and vmoprint as they no longer compile
#

INSTALL_DIR=$(DSTBASE)/bin
TARGET_DIRS=${INSTALL_DIR}
LIBS = $(LIBMACH)

# C programs that live in the current directory, have one source file
# and do not need any libraries

PRGM1=	format

# Need a special rule for md that does not use md

md:	md.c
	$(CC) $(_CCFLAGS_) -o md md.c
	-@rm md.d

md.install: md
	-@mach_install $I md ${INSTALL_DIR}
	-@touch md.install


# C programs that live in the current directory, have one source
# file and  need the mach libraries

PRGM1L=	hostinfo macherr vm_stat mpr 

#
# Programs that live in subdirectories, and have makefiles of their own.

SUBS=	 start_rem_server qvss mig gdb

# Programs that live in the current directory, but need to be
# installed with group = kmem and setgid
# have removed vmpinfo, vmmprint, vmoprint


include $(SRCBASE)/Make.inc

all: md
install: md.install

-include $(DEP_FILE)
