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
# Revision 1.24  89/05/05  18:32:01  mrt
# 	Changed CFLAGS to _CCFLAGS_ to pick up all the flags
# 	[89/05/04            mrt]
# 
# Revision 1.23  89/05/02  11:03:41  dpj
# 	Fixed all files to conform to standard copyright/log format
# 
# Revision 1.22  88/10/21  11:40:51  mrt
# removed references to camelot stuff
# 
# Revision 1.21  88/10/14  16:13:57  mrt
# *** empty log message ***
# 
# Revision 1.20  88/10/09  16:01:14  dpj
# Re-organized for Mach source tree.
# 
#
#
# $Header: Makefile,v 1.24 89/05/05 18:32:01 mrt Exp $
#
# Makefile for the network server and all associated programs.
#
    
SRCBASE = $(DSTBASE)/src
INSTALL_DIR = $(DSTBASE)/etc
TARGET_DIRS = $(INSTALL_DIR)
CPDIR = ""
LIBS = $(LIBTHREADS) $(LIBMACH) $(VMTPLIB) $(${MACHINE}_LIB)
COPY_H = netname_defs.h netname.h
DEFS = netname.defs km.defs logstat.defs

LINTFLAGS= -u $(LIBMACH) $(LIBTHREADS)
IBMRT_LIB=../lib/scksum.o
VAX_LIB=
SUN_LIB=
VMTPLIB=


VPATH=../server:../utils
CPATH=../server:$(CPATH)

PRGM = netmsgserver

#
# Sources for the netmsgserver.
#
# For the non RPCMOD version, remove ipc_rpc.c, ipc_exception.c, ipc_msg.c,
# vmtp2.c, tcp.c and add ipc_in.c, ipc_out.c, vmtp1.c.
#
#
SRCS =  nmserver.c dispatcher.c lock_queue.c mem.c nm_extra.c \
	rwlock.c sbuf.c timer.c uid.c \
	crypt.c crypt_multperm.c crypt_newdes.c \
	crypt_xor.c multperm.c \
	ipc.c ipc_block.c ipc_msg.c ipc_rpc.c \
	ipc_exception.c ipc_swap.c \
	keyman.c km_procs.c km_utils.c \
	netname.c nn_handler.c nn_procs.c \
	portrec.c portcheck.c pc_handler.c \
	pc_startup.c portsearch.c ps_auth.c \
	portops.c po_message.c po_notify.c po_utils.c \
	datagram.c deltat.c deltat_handler.c \
	deltat_send.c deltat_utils.c \
	srr.c srr_handler.c srr_utils.c netipc.c \
	network.c transport.c \
	nm_init.c logstat.c ls_rec.c \
	vmtp2.c nm_enable.c crypt_des.c des.c \
	tcp.c 


all: config.h

include $(SRCBASE)/Make.inc
I = -c -d -g kmem -m 2755
DEP_FILE=./Makedep

config.h: ../conf/config.h
	cp -p ../conf/config.h config.h

lint:
	$(LINT) $(LINTFLAGS) $(SRCS)


#
# Special programs not to be installed
#
UTILPROG=	nmcontrol nmxlog nmtest nmtime
utilprog:	$(UTILPROG) config.h


nmcontrol: nmcontrol.o logstatUser.o ls_rec.o
	$(CC) $(_CCFLAGS_) -o nmcontrol nmcontrol.o logstatUser.o ls_rec.o -lmach

nmxlog: nmxlog.o
	$(CC) $(_CCFLAGS_) -o nmxlog nmxlog.o

nmxportstat: nmxportstat.o ls_rec.o
	$(CC) $(_CCFLAGS_) -o nmxportstat nmxportstat.o

nmtest: nmtest.o
	$(CC) $(_CCFLAGS_) -o nmtest nmtest.o -lthreads -lmach

vmtptest: vmtptest.o
	$(CC) $(_CCFLAGS_) -o vmtptest vmtptest.o -lthreads -lmach ../lib/libvmtp.a

nmtime: nmtime.o
	$(CC) $(_CCFLAGS_) -o nmtime nmtime.o -lmach

clean:
	rm -f eddep tags *.o $(PRGM) $(MIG_HDRS) $(MIG_USRS) $(MIG_SRVS)  \
		$(UTILPROG) errs linterrs makelinks *.log *LOG*

