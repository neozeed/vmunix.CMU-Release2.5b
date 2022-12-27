#
# HISTORY
# 14-Aug-89  Mary Thompson (mrt) at Carnegie-Mellon University
#	Removed setting of DSTBASE and SRCBASE
#	Made install depend on INCS
#
# $Log:$
#  3-Mar-88  Michael Jones (mbj) at Carnegie-Mellon University
#	Wrote it.
#


INSTALL_DIR=${DSTBASE}/parallel/include
TARGET_DIRS=$(INSTALL_DIR)

I = -c -d
INCS = rec_mutex.h stdio.h

all:
install: $(TARGET_DIRS) $(INCS)
	-@mach_install $I ${INCS} ${INSTALL_DIR}

$(TARGET_DIRS):
	mkdir $@
