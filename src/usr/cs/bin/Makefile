######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.3  88/11/08  19:48:52  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /usr/cs/bin/

BUILD_HERE		=

abandon-ship_ILINKS	= ${IDIR}welcome-aboard
bbedit_IGROUP		= cmu
bbedit_IMODE		= 6711
bbedit_IOWNER		= cmu
finger_ILINKS		= ${IDIR}f
finger_LIBS		= -lwp -lsys
host_ILINKS		= ${IDIR}net ${IDIR}service ${IDIR}protocol
host_LIBS		= -lresolv
hostq_LIBS		= -lresolv
mailq_IMODE		= 4711
mailq_IOWNER		= root
mailq_LIBS		= -lsys
man_ILINKS		= ${IDIR}apropos ${IDIR}whatis
name_LIBS		= -lwp
podtype_IMODE		= 111
pracct_LIBS		= -lsys
setmodes_LIBS		= -lsys
top_CFLAGS		= -I${USR_MACH}/include
top_IGROUP		= kmem
top_IMODE		= 2711
top_INCVERS		=
top_LDFLAGS		= -L${USR_MACH}/lib
top_LIBS		= -lmach -lcurses -ltermcap -lm -lsys
ttyinfo_LIBS		= -lsys
ttyis_LIBS		= -lsys

include ../../../Makefile-common
