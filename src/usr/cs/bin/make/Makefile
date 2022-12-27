#
# $Header: Makefile,v 5.0 89/03/01 01:36:45 bww Exp $
#
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
########################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 5.0  89/03/01  01:36:45  bww
# 	Version 5.
# 	[89/03/01  01:27:08  bww]
# 
########################################################################

IDIR			= /usr/cs/bin/

MMAX_DEFS		= -DCOFF

PROGRAMS		= make
OFILES			= main.o doname.o misc.o files.o dirs.o libs.o\
			  rules.o dosys.o gram.o dyndep.o
HFILES			= defs.h
CFLAGS			= ${${MACHINE}_DEFS?${${MACHINE}_DEFS}:-DASCARCH}
LINTFLAGS		= -ax ${CFLAGS}
GARBAGE			= gram.c

include ../../../../Makefile-common
