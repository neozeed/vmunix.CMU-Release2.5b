#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.9.1.1  89/10/24  23:29:20  mrt
# 	Removed inc target that made the /usr/mach/include/machine and
# 	mach/machine links as the kernel include directories are no
# 	longer in /usr/mach/include.
# 	[89/10/24            mrt]
# 
# Revision 1.9  89/08/28  16:31:25  mrt
# 	Added printenv of PATHs to install and all targets
# 	as a sanity check.
# 	[89/08/28            mrt]
# 
# Revision 1.8  89/07/30  20:32:44  mrt
# 	Removed the ./ from the file names for etc/mach_install and
# 	mach_include/mach_error.h.
# 	[89/07/29            mrt]
# 
# Revision 1.7  89/07/28  13:58:25  mrt
# 	Removed use of SRCBASE so that shadowing would work.
# 	[89/07/28            mrt]
# 
# Revision 1.6  89/06/07  16:09:42  mrt
# 	Changed the bootstrap procedure. The bootstrap directory no
# 	longer exists. The stuff in its Makefile was moved to here.
# 	All the inlcudes that need to be installed are now in
# 	mach_include and the install_includes target gets them installed.
# 	It is no longer necessary to run a separate bootstrap target,
# 	install will do that automatically.
# 	[89/06/07            mrt]
# 
# Revision 1.5  89/05/05  17:38:49  mrt
# 	Linked mach/mig_errors.h instead of having the mig makefile
# 	install it.
# 
# 	Jan-12-89 Mary Thompson @ Carnegie Mellon
# 	Removed kernel from list of Subdirectories
# 	[89/04/29            mrt]
# 
# Revision 1.4  89/01/13  13:02:08  mrt
# 	Removed building of kernel subdirectroy
#
# Jul-1-88   Mary Thompson @ Carnegie Mellon
#	Changed cp's to mach_install -p so that files with
#	read only access could be reinstalled.
#
# Dec-11-87  Mary Thompson @ Carnegie Mellon
#	Separated out the making of the man and doc
#	sections to be done under install.docs rather
#	than all and install.
#
# Jun-1-87   Mary Thompson @ Carnegie Mellon
#	Changed to include Make.inc
#
#
#  Top level makefile for /usr/mach tree


INCDIR=${DSTBASE}/include
TARGET_DIRS = $(INCDIR) $(DSTBASE) $(DSTBASE)/etc $(DSTBASE)/bin \
	./bin/mig ./lib/libmach

SUBS=	lib bin etc tests parallel mach_include

all:	printenv bootstrap install-libs 
install: printenv bootstrap install-libs

printenv:
	printenv PATH
	printenv CPATH
	printenv LPATH

include $(SRCBASE)/Make.inc

bootstrap:  $(SUB_DIR) $(TARGET_DIRS) inst md mig install_includes

inst: etc/mach_install.csh mach_include/mach_error.h
	-cp -p etc/mach_install.csh ${DSTBASE}/etc/mach_install
	-@mach_install $I mach_include/mach_error.h $(DSTBASE)/include

md:
	-@echo cd bin; cd bin; \
 	make ${MFLAGS} "I=$I" DSTBASE=${DSTBASE} SRCBASE=${SRCBASE} md.install
mig:
	-@echo cd bin/mig; cd bin/mig; \
 	make ${MFLAGS} "I=$I" DSTBASE=${DSTBASE} SRCBASE=${SRCBASE} install

install_includes:
	-@echo cd mach_include; cd mach_include; \
	make ${MFLAGS} "I=$I" DSTBASE=${DSTBASE} SRCBASE=${SRCBASE} install
	
install-libs:
	cd lib/libmach; \
	make ${MFLAGS} "I=$I" DSTBASE=${DSTBASE} SRCBASE=${SRCBASE} \
		INCDIR=${INCDIR} install


install.doc:
	cd doc; \
	make ${MFLAGS} "I=$I" DSTBASE=${DSTBASE} SRCBASE=${SRCBASE} \
		INCDIR=${INCDIR} GROUP=${GROUP} install

install.man:
	cd man;\
	make ${MFLAGS} "I=$I" DSTBASE=${DSTBASE} SRCBASE=${SRCBASE} \
		INCDIR=${INCDIR} GROUP=${GROUP} install

 
