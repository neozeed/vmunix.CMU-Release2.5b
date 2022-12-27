# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
# HISTORY
# $Log:	Make.inc,v $
# Revision 1.7  89/08/28  16:03:01  mrt
# 		Added definition for PMAX_M. Added definitions for
# 		$(MACHINE)_AS_FLAGS
# 		Removed do_doc_mans and do_install_doc_mans rules. Renamed
# 		PS to ps, and added scribe rule to build .doc as well as 
# 		.ps files.
# 	[89/08/28            mrt]
# 
# Revision 1.6  89/07/28  13:58:12  mrt
# 	Added definition of MMAX_CCA.
# 	[89/07/28            mrt]
# 
# Revision 1.5  89/07/14  16:58:24  mrt
# 	Changed to work better with build. INSTALL targets no
# 	longer depend on ALL as build will do that for the -fromsource
# 	builds. Do not export CPATH and LPATH.
# 	[89/06/22            mrt]
# 
# Revision 1.4  89/06/08  17:57:52  mrt
# 	Changed the csh and sh.install to install the scripts
# 	with execute permission.
# 	[89/06/08            mrt]
# 
# Revision 1.3  89/06/07  16:02:54  mrt
# 	Added _CCFLAGS_ which are the union of DEF_CFLAGS, CFLAGS,
# 	CARGS and CENV. Added CC_W_ASM which is a compiler that
# 	understands .asm statements (e.g. not hc). Separated INSTALL_LIB
# 	and INSTALL_LIBP.
# 	[89/06/07            mrt]
# 
#

INCDIR=$(DSTBASE)/include

SERVERINCDIR=$(INCDIR)/servers
LIBMACH=$(DSTBASE)/lib/libmach.a
LIBTHREADS = $(DSTBASE)/lib/libthreads.a
LIBCPU = $(DSTBASE)/lib/libcpu.a
DEP_FILE = Makedep

I		= -c -d
MIGFLAGS 	= -MD 
VAX_M=$(machine)
IBMRT_M=$(machine)
SUN_M=sun3
PMAX_M=pmax

DEF_CFLAGS=${DEF_CFLAGS?${DEF_CFLAGS}:-O -MD -DMACH -D$($(MACHINE)_M)}
_CCFLAGS_=${$@_CFLAGS?${$@_CFLAGS}:${CFLAGS}} ${$@_CENV?${$@_CENV}:${CENV}} ${$@_CARGS?${$@_CARGS}:${CARGS}} ${$@_DEF_CFLAGS?${$@_DEF_CFLAGS}:${DEF_CFLAGS}}

VAX_AS_FLAGS=
SUN3_AS_FLAGS=
IBMRT_AS_FLAGS=
MMAX_AS_FLAGS=
PMAX_AS_FLAGS=-nocpp
CC=${CC?${CC}:cc}
VAX_CCA=${CC}
SUN_CCA=${CC}
MMAX_CCA=${CC}
IBMRT_CCA=pcc
PMAX_CCA=$(CC)
CC_W_ASM=${${MACHINE}_CCA}

_INCDIR=${${@:.install=}_INCDIR?${${@:.install=}_INCDIR}:${INCDIR}}
_INSTALL_DIR=${${@:.install=}_INSTALL_DIR?${${@:.install=}_INSTALL_DIR}:${INSTALL_DIR}}

RELEASE=release
IOWNER=${IOWNER?${IOWNER}:mach}
IGROUP=${IGROUP?${IGROUP}:mach}
IMODE=${IMODE?${IMODE}:755}

INSTALLING=${FROMBASE?x:install}
RELEASING=${FROMBASE?install:x}

OBJS		= $(SRCS:.c=.o)
MIG_HDRS	= $(DEFS:.defs=.h)
MIG_USRS	= $(DEFS:.defs=User.c) 
MIG_SRVS	= $(DEFS:.defs=Server.c) 
USR_OBJS	= $(DEFS:.defs=User.o) 
SRV_OBJS	= $(DEFS:.defs=Server.o) 
DOC_OBJS	= $(DOC:=.ps)
DOC_MSS_OBJS	= $(FSRC:.fsrc=.mss)
LIB_OBJS	= $(LIB_SRCS:.c=.o)
INSTALL_INCS	= $(INCS:=.install)
INSTALL_DOCS	= $(DOC:=.install)
INSTALL_OBJS	= $(PRGM:=.${INSTALLING})  $(PRGM1:=.${INSTALLING})  $(PRGM1L:=.${INSTALLING})
RELEASE_OBJS	= $(PRGM:=.${RELEASING})  $(PRGM1:=.${RELEASING})  $(PRGM1L:=.${RELEASING})
INSTALL_CSHS	= $(CSH:.csh=.install) $(CSH:.sh=.install)
INSTALL_MANS	= $(MAN_OBJS:.${SECTION}=.install)
INSTALL_LIB	= $(LIB_BLD:=.install)
INSTALL_LIBP	= $(LIBP_BLD:=.install)
MIG_OUTPUTS	= $(MIG_HDRS) $(MIG_USRS) $(MIG_SRVS)
SUB_DIR		= $(SUBS:=/dir)
SUB_ALL		= $(SUBS:=/all)
SUB_INSTALL	= $(SUBS:=/install)
FORCE		= xxFUNNYxx.force

.SUFFIXES: .mss .fsrc .ps .install .csh .sh .$(SECTION)

.fsrc.mss:
	format $*.fsrc

.c.o:
	$(CC) $(_CCFLAGS_) -c $*.c
	md -u $(DEP_FILE) -d $*.d

.csh.install:
	-@mach_install $I -c -m 555 $*.csh ${INSTALL_DIR}/$*
	-@touch $*.install

.sh.install:
	-@mach_install $I -c -m 555 $*.sh ${INSTALL_DIR}/$*
	-@touch $*.install

.$(SECTION).install:
	-@mach_install $I -c $*.$(SECTION) ${INSTALL_MAN_DIR}
	-@touch $*.install

all:	$(TARGET_DIRS) $(SUB_DIR) $(SUB_ALL) $(PRGM) $(PRGM1) $(PRGM1L) \
 	 $(LIB_BLD) $(LIBP_BLD) $(DOC_OBJS) 

include: $(TARGET_DIRS) $(INSTALL_INCS)


install: $(TARGET_DIRS) $(SUB_DIR) $(SUB_INSTALL) $(INSTALL_INCS) \
	 $(INSTALL_OBJS) $(INSTALL_DOCS) \
	 $(INSTALL_CSHS) $(INSTALL_LIB) $(INSTALL_LIBP) $(INSTALL_MANS) 
do_toc:
	catman -M $(DSTBASE)/man $(SECTION)


$(TARGET_DIRS):
	mkdir $@


$(SUB_ALL) $(SUB_INSTALL): $(FORCE)
	-cd $(@D) && $(MAKE) $(MFLAGS) DSTBASE=$(DSTBASE)\
	 SRCBASE=$(SRCBASE) LIBMACH=$(LIBMACH) \
	 LIBTHREADS=$(LIBTHREADS)  $(@F)

$(SUB_DIR):
	-@if [ ! -d $(@D) ]; then echo "mkdir ./$(@D)" ; \
	mkdir ./$(@D) ;\
	else true; fi

$(FORCE):

#.EXPORT: CPATH LPATH PATH

$(PRGM):  $(SRV_OBJS) $(OBJS) $(LIBS)
	-$(CC) $(_CCFLAGS_) -o $(PRGM) $(OBJS) $(SRV_OBJS) $(LIBS)

$(PRGM1L): $$@.o $(LIBS)
	-$(CC)  $(_CCFLAGS_) -o $@  $@.o $(LIBS)

$(PRGM1): $$@.o
	-$(CC)  $(_CCFLAGS_) -o $@  $@.o 

$(DOC_MSS_OBJS): $$(@:.mss=.fsrc)

$(DOC_OBJS): $(MSS)
	scribe $* -device:postscript
	scribe $* -device:file

$(LIB_BLD) : $(LIB_OBJS)
	@rm -f $(LIB_BLD)
	ar crv ${LIB_BLD} ${LIB_OBJS} 
	ranlib ${LIB_BLD}

$(LIBP_BLD) : $(LIB_OBJS)
	@rm -f profiled/$(LIBP_BLD)
	if [ -d profiled ]; then true; \
	else mkdir profiled; fi;
	cd profiled && ar crv ${LIBP_BLD} ${LIB_OBJS} 
	ranlib profiled/${LIBP_BLD}
	mv profiled/${LIBP_BLD} .

$(INSTALL_INCS): $$(@:.install=)
	-@mach_install $I $(@:.install=) $(_INCDIR)
	-@touch $@

$(INSTALL_OBJS): $$(@:.install=)
	-@mach_install $I $(@:.install=) $(_INSTALL_DIR)
	-@touch $@

$(RELEASE_OBJS):
	${_RELEASE_} -o ${%_IOWNER?${%_IOWNER}:${IOWNER}}\
		-g ${%_IGROUP?${%_IGROUP}:${IGROUP}}\
		-m ${%_IMODE?${%_IMODE}:${IMODE}}\
		-tostage ${TOSTAGE}\
		-fromstage ${FROMSTAGE}\
		${_INSTALL_DIR}/%


$(INSTALL_DOCS): $$(@:.install=.ps)
	-@mach_install $I $? $(INSTALL_DIR)
	-@touch $@

$(INSTALL_LIB): $(LIB_BLD)
	-@mach_install $I -r $(LIB_BLD) $(INSTALL_DIR)
	-@touch $@

$(INSTALL_LIBP): $(LIBP_BLD)
	-@ mach_install $I -r $(LIBP_BLD) $(INSTALL_DIR)
	-@touch $@

$(MIG_HDRS): $$(@:.h=.defs)
	mig $(MIGFLAGS) $(MIGENV) $(MIGARGS) $(@:.h=.defs) \
	   -server /dev/null -user /dev/null
	md -u $(DEP_FILE) -d $(@:.h=.d)

$(MIG_USRS): $$(@:User.c=.defs)
	mig $(MIGFLAGS)  $(MIGENV) $(MIGARGS) $(@:User.c=.defs)
	md -u $(DEP_FILE) -d $(@:User.c=.d)

$(MIG_SRVS): $$(@:Server.c=.defs)
	mig $(MIGFLAGS)  $(MIGENV) $(MIGARGS) $(@:Server.c=.defs)
	md -u $(DEP_FILE) -d $(@:Server.c=.d)

-include $(OBJECTDIR)/$(DEP_FILE)

