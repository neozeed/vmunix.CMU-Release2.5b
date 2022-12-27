# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.3.1.1  89/10/24  23:19:54  mrt
# 
# 	Added subdirectory parallellib
# 
# 
# Revision 1.4  89/08/29  23:34:10  mrt
# 	Added subdirectory parallellib
# 	[89/08/29            mrt]
# 
# Revision 1.3  89/06/08  18:02:01  mrt
# 	Upgraded to work with Makeconf
# 	[89/05/28            mrt]
# 
# Revision 1.2  89/05/05  19:03:08  mrt
# 	Cleanup for Mach 2.5
# 
#  5-Jan-88   Mary Thompson (mrt) at Carnegie Mellon 
#	added test_netname and bin_tests.csh
#
# 23-Jul-87   Mary Thompson (mrt) at Carnegie Mellon 
#	added subdir cthreads
#
#

INSTALL_DIR=${DSTBASE}/tests
TARGET_DIRS=$(INSTALL_DIR)
LIBS=$(LIBMACH)
CSH=mach_tests.csh bin_tests.csh

SUBS= cthreads parallellib

PRGM1L=	ipc_test cowtest vm_read test_service test_netname

include $(SRCBASE)/Make.inc

-include $(DEP_FILE)
