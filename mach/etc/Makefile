# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.5  89/06/07  14:22:22  mrt
# 	Upgraded to use Makeconf
# 	[89/05/28            mrt]
# 
# Revision 1.4  89/05/05  18:27:12  mrt
# 	Removed MACH
# 	[89/05/02            mrt]
# 
#
# 13-Oct-88  Mary Thompson
#	Added netmemoryserver to SUBS
#  9-Sep-88  Mary Thompson
#	Added machtree.sh, kmerge.csh and cupdate.csh
#

INSTALL_DIR=${DSTBASE}/etc
TARGET_DIRS=$(INSTALL_DIR)
LIBS = $(LIBMACH)

PRGM1L = mach_swapon 
CSH = mach_install.csh mutil.csh runmake.csh kmerge.csh \
	cupdate.csh machtree.sh
SUBS=	envmgr ipcexecd lcp netmemoryserver netmsgserver rconsole service


include $(SRCBASE)/Make.inc

-include $(DEP_FILE)


