######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.5  89/01/28  23:57:55  gm0w
# 	Added explicit SUBDIRS and targets since the root of the
# 	source tree is filled with clutter and probably always
# 	will be.  Since the vice-virtue and kernel subdirectories
# 	are not SSP, we should not cd into them to build a full
# 	facilities release at this time.  Moved Makefile-common
# 	rule here where it belongs.
# 	[89/01/26            gm0w]
# 
# Revision 2.4  88/12/20  23:21:26  mja
# 	Added KERNEL.ADB_SED_OPTIONS definition and commented out INOBJECTDIR,
# 	and FILES_HERE_* definitions for the time being so that KERNEL.ADB
# 	build would work.  A build from here should work pretty much the same
# 	as anywhere else so its not clear that these definitions belong in any
# 	case.
# 
# Revision 2.3  88/11/09  00:15:51  gm0w
# 	Created for new and improved SSP format.
# 	[88/11/09            gm0w]
# 
######################################################################

IDIR			= /

SUBDIRS			= bin etc usr

include Makefile-common

#
#  New Makefile-common rule
#
Makefile-common: Makefile-common.m4
	m4 Makefile-common.m4 | sed -e '1,/^END_OF_M4$$/d' -e '/^#/d' > Makefile-common.out
	${MV} -f Makefile-common.out Makefile-common

#
# Copyright (c) 1986 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	4.13	(Berkeley)	5/15/86
#
# This makefile is designed to be run as:
#	make build
#	make installsrc
# The `make build' will compile and install the libraries
# before building the rest of the sources. The `make installsrc'
# will then install the remaining binaries.
# 
# It can also be run in the more conventional way:
#	make
#	make install
# The `make' will compile everything without installing anything.
# The `make install' will then install everything. Note however
# that all the binaries will have been loaded with the old libraries.
#
# C library options: passed to libc makefile.
# See lib/libc/Makefile for explanation.
# HOSTLOOKUP must be either named or hosttable.
# DFLMON must be either mon.o or gmon.o.
# DEFS may include -DLIBC_SCCS, -DSYSLIBC_SCCS, both, or neither.
#
HOSTLOOKUP=named
DFLMON=mon.o
#DEFS= -DLIBC_SCCS
LIBCDEFS= HOSTLOOKUP=${HOSTLOOKUP} DFLMON=${DFLMON} DEFS="${DEFS}"

# global flags
# SRC_MFLAGS are used on makes in command source directories,
# but not in library or compiler directories that will be installed
# for use in compiling everything else.
#

buildlib: ${ALWAYS}
	@echo installing /usr/include
	cd usr/include; make ${MFLAGS} DESTDIR=${DESTDIR} install
	@echo
	@echo compiling libc.a
	cd lib/libc; make ${MFLAGS} ${LIBCDEFS}
	@echo installing /lib/libc.a
	cd lib/libc; make ${MFLAGS} DESTDIR=${DESTDIR} install
	@echo
	@echo compiling C compiler
	cd lib; make ${MFLAGS} pcc cpp c2
	@echo installing C compiler
	cd lib/pcc; make ${MFLAGS} DESTDIR=${DESTDIR} install
	cd lib/cpp; make ${MFLAGS} DESTDIR=${DESTDIR} install
	cd lib/c2; make ${MFLAGS} DESTDIR=${DESTDIR} install
	cd lib; make ${MFLAGS} clean
	@echo
	@echo re-compiling libc.a
	cd lib/libc; make ${MFLAGS} ${LIBCDEFS}
	@echo re-installing /lib/libc.a
	cd lib/libc; make ${MFLAGS} DESTDIR=${DESTDIR} install
	@echo
	@echo re-compiling C compiler
	cd lib; make ${MFLAGS} pcc cpp c2
	@echo re-installing C compiler
	cd lib/pcc; make ${MFLAGS} DESTDIR=${DESTDIR} install
	cd lib/cpp; make ${MFLAGS} DESTDIR=${DESTDIR} install
	cd lib/c2; make ${MFLAGS} DESTDIR=${DESTDIR} install
	@echo
	@echo compiling /usr/lib
	cd usr/lib; make ${MFLAGS} ${SRC_MFLAGS}
	@echo installing /usr/lib
	cd usr/lib; make ${MFLAGS} ${SRC_MFLAGS} DESTDIR=${DESTDIR} install
