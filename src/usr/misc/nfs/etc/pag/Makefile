#######################################################################
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.0  89/06/15  15:36:34  dimitris
#   Organized into a misc collection and SSPized
# 
# 
# 
# 
#######################################################################
IDIR 		= /usr/misc/.nfs/etc/pag/

PROGRAMS	= pag pagd

fakevice.o_CFLAGS	= -I../../include 

pag_LDFLAGS	= -w  -L../../lib
pag_LIBS	= -lrpcsvc -lrpc -lnfs -lsys

pag_OFILES	= client.o transport.o fakevice.o pag.o

pagd_OFILES	= server.o transport.o pioctl.o pagd.o 

pagd_LIBS 	= -lsys

pioctl.o_CFLAGS	= -I../../include

server.o : service.c
	cc -c service.c -DSERVER=1
	mv service.o server.o

client.o : service.c
	cc -c service.c -DSERVER=0
	mv service.o client.o


include ../../../../../Makefile-common
