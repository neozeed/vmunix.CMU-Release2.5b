#
#	@(#)Makefile	4.2	(Berkeley)	83/02/11
#
######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.4  89/06/01  15:22:08  gm0w
# 	Removed yaccpar (now in /usr/lib).
# 	[89/06/01            gm0w]
# 
# Revision 1.3  89/01/28  17:45:55  berman
# 	Rewrote for SSP.
# 	[89/01/18            berman]
# 
# 14-May-87  Jonathan J. Chew (jjc) at Carnegie-Mellon University
#	Commented out rule for sources because it confuses make to
#	have both RCS files and rules.
#	Added dependencies for install.
#	Defined and put INSTALL to use.
#
######################################################################
IDIR			= /usr/bin/

PROGRAMS		= yacc
OFILES			= y1.o y2.o y3.o y4.o
HFILES			= dextern files

CFLAGS			= -DWORD32

include ../../../Makefile-common
