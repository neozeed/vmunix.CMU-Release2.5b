#
# HISTORY
#  9-Jun-88  Michael Jones (mbj) at Carnegie-Mellon University
#	Installed READ_ME documentation file.
#
#  3-Mar-88  Michael Jones (mbj) at Carnegie-Mellon University
#	Wrote it.
#


INSTALL_DIR=${DSTBASE}/parallel
TARGET_DIRS=$(INSTALL_DIR) $(INSTALL_DIR)/lib $(INSTALL_DIR)/include
LIBS = $(LIBMACH)
_MFLAGS=$(MFLAGS) -m
SUBS = include 
DOCS = READ_ME

all:	$(TARGET_DIRS) $(SUB_ALL) $(PRGM) $(PRGM1) $(PRGM1L) $(LIB_BLD) \
	$(LIBP_BLD) $(DOC_OBJS) \
	libc/all

install: $(TARGET_DIRS) $(SUB_INSTALL) $(INSTALL_INCS) $(INSTALL_OBJS) \
	 $(INSTALL_DOCS) $(INSTALL_CSHS) $(INSTALL_LIB) $(INSTALL_MANS) \
	libc/install READ_ME.install

libc/all: $(FORCE)
	-@if [ ! -d $(@D) ]; then echo "mkdir ./$(@D)" ; \
	mkdir ./$(@D) ;\
	else true; fi
	cd $(@D) && $(MAKE) $(_MFLAGS) DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) \
	GROUP=$(GROUP) LIBMACH=$(LIBMACH) I="$(I)" \
	libc.a libc_p.a

libc/install: libc/all
	cd $(@D) && $(MAKE) $(_MFLAGS) DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) \
	GROUP=$(GROUP) LIBMACH=$(LIBMACH) I="$(I)" \
	libc.a_ordered libc_p.a_ordered
	-@mach_install $I libc/libc.a libc/libc_p.a $(INSTALL_DIR)/lib

READ_ME.install: $$(@:.install=)
	-@mach_install $I $(@:.install=) $(INSTALL_DIR)
	-@touch $@

include $(SRCBASE)/Make.inc
