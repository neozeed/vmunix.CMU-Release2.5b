# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
#  $Log:	Makefile,v $
# Revision 1.14.1.1  89/10/24  23:21:24  mrt
# 	Changed the LINTSRCS
# 	[89/10/24            mrt]
# 
# Revision 1.14  89/08/28  16:04:08  mrt
# 	Defined ASM_FLAGS per machine type so that -MD can be
# 	omitted for the pmax.
# 	[89/08/28            mrt]
# 
# Revision 1.13  89/07/30  20:35:57  mrt
# 	Changed VPATH elements to be relative pathnames so that
# 	shadowing will work.
# 	[89/07/20            mrt]
# 
# Revision 1.12  89/07/14  17:59:58  mrt
# 	Defined LINT to be the lint program, and changed old use
# 	of LINT to LINT_DIR.
# 
# Revision 1.11  89/06/13  16:03:14  mrt
# 	All the include files are now kept in mach_include and installed
# 	by the Makefile there. That must be done before the library can
# 	be built correctly.
# 	[89/05/28            mrt]
# 
# 	Turned type checking off in compilation of new_mach_user, so
# 	that libmach.a would work with older kernel. The new migcom
# 	both sets and checks the simple msg bit, but the CS5 kernels
# 	do not set the simple msg bit in their replies
# 	[89/05/26            mrt]
# 
# Revision 1.10  89/05/09  22:16:33  mrt
# 	Added include of $(DEP_FILE)
# 	[89/05/09            mrt]
# 
# Revision 1.9  89/05/05  18:38:00  mrt
# 	Removed mach_extra.h from INCS and added an errorlib.install rule
# 	Removed pagerUser.c and pager.defs. Added mig_strncpy
# 	Changed CFLAGS to _CCFLAGS_ and cc to $(CC)
# 	 
# 	[89/04/28            mrt]
# 
# Revision 1.8  89/03/22  14:06:42  mrt
# Added ./profiled to list of TARGET_DIRS
# 
# Revision 1.7  89/02/07  17:33:27  mrt
# Added XXX_new_vawp.c to SRCS and
# all the XXX stuff to LINTSRC
# 
# Revision 1.6  89/01/13  13:13:08  mrt
# removed mach.h from the LINTSRC as new_mach_user.c is now
# used. Changed /usr/cs/bin/cc to pcc to get ccom used instead
# of hccom.
# 
# Revision 1.5  88/11/21  13:20:17  mrt
# renamed msg_receive to msg.c and inlcuded
# new_mach_user.c in LINTSRCS
# 
# Revision 1.4  88/10/12  12:09:13  mrt
# Added memory_objectServer.c to list of SRCS
# 
#  25-Oct-88  Mary R. Thompson at Carnegie Mellon
#	Renamed msg_receive.c to msg.c as it now
#	contains a wrap around for msg_send.
#
#  12-Oct-88  Mary R.Thompson at Carnegie Mellon
#	Added memory_objectServer.o to libmach.a
# 
#  30-Aug-88  Mary R. Thompson at Carnegie Mellon
#	Added new_mach_user.c, XXX_{pdp,pds,pdu,plr}.c to
#	LIB_SRCS list. Also added NetMemory user stuff.
#	Removed references to msgnUser.c. Changed explicit
#	references to cc to /usr/cs/bin/cc, removed mach_user.c
#	from LIB_SRCS as it is now included by new_mach_user.c

INSTALL_DIR=${DSTBASE}/lib
LINT_DIR=${DSTBASE}/lib/lint
TARGET_DIRS=${INSTALL_DIR} $(LINT_DIR)  ./profiled

VAX_ASM_FLAGS= -A -MD
SUN3_ASM_FLAGS= -A -MD
IBMRT_ASM_FLAGS= -A -MD
MMAX_ASM_FLAGS= -A -MD
PMAX_ASM_FLAGS= -A
LINTLIB=llib-lmach.ln
LINT=/usr/cs/bin/lint

LIB_BLD=libmach.a
LIBP_BLD=libmach_p.a

.SUFFIXES: .cs


LIB_SRCS=	mach_traps.c mach_init.c  sbrk.c error_codes.c \
	mach_error.c fork.c Ovfork.c slot_name.c mig_support.c \
	mig_strncpy.o \
	ts_convert.c msg.c mach_exception.c XXX_new_vawp.c \
	new_mach_user.c XXX_pdp.c XXX_pds.c XXX_pdu.c XXX_plr.c \
	env_mgrUser.c ipcxUser.c netnameUser.c serviceUser.c \
	excUser.c excServer.c netmemoryUser.c netmemstatUser.c \
	memory_objectServer.c mach_debug_user.c

DEFS=	exc.defs memory_object.defs mach.defs mach_debug.defs \
	env_mgr.defs ipcx.defs netmemory.defs netmemstat.defs \
	netname.defs service.defs

VPATH=:../../etc/envmgr:../../etc/ipcexecd:../../etc/netmemoryserver:../../etc/netmsgserver/server:../../etc/service

include $(SRCBASE)/Make.inc

.c.o:
	$(CC) -p $(_CCFLAGS_) -c  $*.c
	mv $*.o profiled
	$(CC) $(_CCFLAGS_) -c  $*.c
	md -u $(DEP_FILE) -d $*.d

.cs.o:
	$(CC) -p -ES $(_CCFLAGS_) $*.cs > $*.as
	as $($(MACHINE)_AS_FLAGS) -o $*.o $*.as
#	rm -f $*.as
	mv $*.o profiled
	$(CC) -ES  $(_CCFLAGS_) $*.cs > $*.as
	as $($(MACHINE)_AS_FLAGS) -o $*.o $*.as
#	rm -f $*.as
	md -u Makedep -d $*.d

# Sources for raw traps are taken from kernel include files.
# Routines that shadow libc versions are ignored.
# .h files are used for mig generated interfaces instead of the .c files
# message.h defines msg_send, msg_receive and msg_rpc so msg.c is not
# included

LINTSRC=error_codes.c msg.c \
	mach_init.c sbrk.c Ovfork.c slot_name.c mig_support.c \
	mach_error.c mach_exception.c \
	env_mgr.h new_mach_user.c XXX_new_vawp.c \
	XXX_pdp.c XXX_pds.c XXX_pdu.c XXX_plr.c \
	ipcx.h exc.h netmemory.h netmemstat.h


all:	${LINTLIB}

# sbrk requires special rule because it contains asm statements.
# sbrk cannot be compiled by hc
sbrk.o: sbrk.c
	$(CC_W_ASM) -p ${${MACHINE}_ASM_CFLAGS} -c $*.c
	mv sbrk.o profiled
	$(CC_W_ASM) ${${MACHINE}_ASM_CFLAGS} -c $*.c
	md -u Makedep -d $*.d

mach_traps.o: mach_traps.cs
	$(CC) -ES -p  $(_CCFLAGS_) mach_traps.cs > mach_traps.as
	as $($(MACHINE)_AS_FLAGS) -o mach_traps.o mach_traps.as
#	rm -f mach_traps.as
	mv mach_traps.o profiled
	$(CC) -ES  $(_CCFLAGS_) mach_traps.cs > mach_traps.as
	as $($(MACHINE)_AS_FLAGS) -o mach_traps.o mach_traps.as
#	rm -f mach_traps.as
	md -u Makedep -d mach_traps.d


fork.o: fork.$(machine).cs
	$(CC) -ES $(_CCFLAGS_) -p $*.$(machine).cs > $*.as
	as $($(MACHINE)_AS_FLAGS) -o $*.o $*.as
	rm -f $*.as
	mv $*.o profiled
	$(CC) -ES $(_CCFLAGS_) $*.$(machine).cs > $*.as
	as $($(MACHINE)_AS_FLAGS) -o $*.o $*.as
	rm -f $*.as
	md -u Makedep -d $*.$(machine).d

${LINTLIB}:	${LINTSRC}
	-if [ -f $(LINT) ]; then \
		$(LINT) -Cmach -DLINTLIBRARY ${LINTSRC}; else exit 0; fi

lint:	${LINTSRC}
	-if [ -f $(LINT) ]; then \
		$(LINT)  $(LINTSRC); else exit 0; fi


mach_user.c: mach.defs
	mig mach.defs -DMIG_USER -user mach_user.c -server /dev/null \
		-header mach_interface.h
new_mach_user.o: new_mach_user.c mach_user.c
	$(CC) $(_CCFLAGS_) -DTypeCheck=0 -c new_mach_user.c

mach_debug_user.c: mach_debug.defs
	mig mach_debug.defs -DMIG_USER -user mach_debug_user.c \
		-server /dev/null 

install:	${LINTLIB}.install 

${LINTLIB}.install:	${LINTLIB}
	-@mach_install $(I) -c ${LINTLIB} ${LINT_DIR}/${LINTLIB}
	-@touch ${LINTLIB}.install

-include $(DEP_FILE)
