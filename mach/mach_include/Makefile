#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.3.1.3  89/10/31  23:51:39  mrt
# 	Added exception_al.h to INCS
# 
# 
# Revision 2.6  89/10/31  23:42:02  mrt
# 	Added exception_all.h to INCS list.
# 	[89/10/30            mrt]
# 
# Revision 2.5  89/10/28  11:29:18  mrt
# 	Put mach_interface.h and mach_debug.h into top level
# 	(INCDIR) instead of $(INCDIR)/mach which no longer exists.
# 	[89/10/27            mrt]
# 
# Revision 2.4  89/10/24  23:56:51  mrt
# 	Added files to inlcude mach/ and sys/ files, rather than making
# 	symbolic links to them, as the mach and sys directories are no
# 	longer in /usr/mach/include.
# 	[89/10/24            mrt]
# 
# Revision 2.3  89/07/30  20:39:03  mrt
# 	Changed VPATH items to be relative pathnames so that shadowing
# 	will work.
# 	[89/07/20            mrt]
# 
# Revision 2.2  89/07/14  17:45:07  mrt
# 	Added  netname.h to list of INCS
# 	[89/07/04            mrt]
# 
# makefile to install includes for /usr/mach/include

TARGET_DIRS =  $(DSTBASE) $(INCDIR) ${SERVERINCDIR} 

INCS=	mach_extra.h msg_type.h mach_types.h mig_errors.h exception.h \
	mach.h mach_init.h mach_error.h mach_exception.h \
	cthreads.h emdefs.h ipcx_types.h netmemory_defs.h \
	netname_defs.h mach_interface.h env_mgr.h ipcx.h \
	netmemory.h netmemstat.h netname.h service.h exc.h \
	mach_debug.h errorlib.h exception_all.h

DEFS= env_mgr.defs ipcx.defs netmemory.defs netmemstat.defs \
	netname.defs service.defs exc.defs mach_debug.defs

emdefs.h_INCDIR=$(SERVERINCDIR)
errorlib.h_INCDIR=$(SERVERINCDIR)
ipcx_types.h_INCDIR=$(SERVERINCDIR)
netmemory_defs.h_INCDIR=$(SERVERINCDIR)
netname_defs.h_INCDIR=$(SERVERINCDIR)
service.h_INCDIR=$(SERVERINCDIR)
env_mgr.h_INCDIR=$(SERVERINCDIR)
ipcx.h_INCDIR=$(SERVERINCDIR)
netname.h_INCDIR=$(SERVERINCDIR)
netmemory.h_INCDIR=$(SERVERINCDIR)
netmemstat.h_INCDIR=$(SERVERINCDIR)
serice.h_INCDIR=$(SERVERINCDIR)


VPATH=:../etc/envmgr:../etc/ipcexecd:../etc/netmemoryserver:../etc/netmsgserver/server:../etc/service:../lib/libmach

all:	include
install: include

include $(SRCBASE)/Make.inc

include: $(TARGET_DIRS)  $(INCS_INSTALL) 

mach_interface.h: mach.defs
	mig $(MIGFLAGS)  $(MIGENV) $(MIGARGS)  mach.defs \
	    -header mach_interface.h -server /dev/null \
	    -user /dev/null
	md -u $(DEP_FILE) -d mach.d

-include $(DEP_FILE)
