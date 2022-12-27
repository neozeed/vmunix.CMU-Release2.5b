# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.6  89/07/14  17:48:32  mrt
# 	corrected vinstall, cleaned up history
# 
# Revision 1.5  89/06/08  18:22:28  mrt
# 	upgraded to work with Makeconf
# 	[89/05/28            mrt]
# 
# Revision 1.4  89/05/05  18:33:38  mrt
# 
# 	 12-Jan-89 Mary Thompson @ Carnegie Mellon
# 	Commented out csu.$(machine) targets as these
# 	programs are now in /usr/cs/lib. They  need to
# 	exported to site outside of CMU-CS
# 

INSTALL_DIR=$(DSTBASE)/lib
TARGET_DIRS=${INSTALL_DIR)
SUBS= libmach  threads

include $(SRCBASE)/Make.inc

# If you don't have /usr/cs/lib/{crt0.o,mcrt0.o,gmon0.o} you will
# need to build them here.
#MACHINE_ALL	= csu.$(machine)/all
#MACHINE_INSTALL	= csu.$(machine)/install
MACHINE_ALL =
MACHINE_INSTALL =

all:		$(MACHINE_ALL)
install:	 machpaths.install  $(MACHINE_INSTALL)

#$(MACHINE_ALL) $(MACHINE_INSTALL): $(FORCE)
#	cd $(@D) && $(MAKE) $(MFLAGS) DSTBASE=$(DSTBASE) GROUP=$(GROUP) $(@F)

machpaths.install:	machpaths
	-@mach_install $I -c machpaths ${INSTALL_DIR}
	-@touch machpaths.install

-include $(DEP_FILE)



