######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.0  89/06/15  15:30:23  dimitris
#   Organized into a misc collection and SSPized
# 
# 
######################################################################

IDIR			= /usr/misc/.nfs/etc/

DATAFILES		= rc rc.add

PROGRAMS		= mount portmap rpcinfo rpc.mountd showmount umount nfsd

SUBDIRS			= pag

CFLAGS			= -I../include  
LDFLAGS			= -w -L../lib
IMODE			= 4711
IOWNER			= root

mount_LIBS		= -lrpcsvc -lrpc -lnfs -lsys
portmap_LIBS		= -lrpc
rpcinfo_LIBS		= -lrpcsvc -lnfs -lrpc 
rpc.mountd_LIBS		= -lrpcsvc -lrpc -lnfs
showmount_LIBS		= -lrpcsvc -lrpc -lnfs -lsys
umount_LIBS		= -lrpcsvc -lrpc -lnfs -lsys
nfsd_LIBS		= -lrpcsvc -lrpc -lnfs -lsys

include ../../../../Makefile-common
