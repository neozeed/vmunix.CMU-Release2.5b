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
# Revision 1.8  89/06/07  11:16:56  mrt
# 	upgraded for new make procedures
# 	[89/05/24            mrt]
# 
# Revision 1.7  89/05/05  18:31:51  mrt
# 	Cleanup for Mach 2.5
# 
# Revision 1.6  89/05/02  11:03:06  dpj
# 	Fixed all files to conform to standard copyright/log format
# 
# Revision 1.5  89/04/24  20:34:28  dpj
# 	Fixed to correctly get the Makefile to be copied into build
# 	even when using an OBJECTDIR.
# 	Fixed to pass correct flags and variables when calling make in
# 	the build subdirectory.
# 	[89/04/19  23:20:52  dpj]
# 
# Revision 1.4  88/10/25  17:33:38  mrt
# fixed netmsgserver.install to depend on all
# 
# Revision 1.3  88/10/21  11:55:03  mrt
# removed camelot from SUBS
# 
# Revision 1.2  88/10/14  16:12:28  mach
# *** empty log message ***
# 
# Revision 1.1.1.1  88/10/09  00:23:44  dpj
# 	New version to build in separate subdirectory.
# 
#
   

SUBS =  lib utils server

include $(SRCBASE)/Make.inc


-include $(DEP_FILE)
