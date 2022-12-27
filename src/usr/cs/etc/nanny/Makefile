######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.9  88/11/10  12:32:01  dlc
# 	Changed the build rule for nanny.config to strip comments.
# 	[88/11/10            dlc]
# 
# Revision 1.8  88/11/08  20:18:51  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /usr/cs/etc/

PROGRAMS		= nanny
DATAFILES		= nanny.config
HFILES			= nanny.h
OFILES			= rcv_comm.o snd_comm.o comm.o load.o main.o \
			  config.o log.o cirque.o

nanny_IOWNER		= root
nanny_IGROUP		= opr
nanny_IMODE		= 6711
nanny_LIBS		= -lsys
nanny.config_IMODE	= 644
nanny.config_SED_OPTIONS= -e '/^\#/d'

include ../../../../Makefile-common
