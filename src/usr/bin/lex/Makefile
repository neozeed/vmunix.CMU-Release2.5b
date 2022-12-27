######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.5  89/06/08  17:10:15  gm0w
# 	Fixed up dependencies.  Moved /usr/lib/lex stuff.
# 	[89/06/08            gm0w]
# 
# Revision 1.4  89/02/14  13:12:11  berman
# 	Fixed typos in ncform nrform IDIR's 
# 	[89/02/14            berman]
# 
# Revision 1.3  89/01/28  17:43:29  berman
# 	SSP-ized Makefile.
# 	[89/01/08            berman]
# 
# 12-May-87  Jonathan J. Chew (jjc) at Carnegie-Mellon University
#	Changed install t depend on ncform and nrform so they'll
#	be checked out if they're RCS files.
#	Also, defined and put INSTALL to use.
#
######################################################################

IDIR			= /usr/bin/

PROGRAMS		= lex

OFILES			= lmain.o parser.o sub1.o sub2.o header.o
HFILES			= ldefs.c

include ../../../Makefile-common

lmain.o: once.c
