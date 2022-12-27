# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	Makefile,v $
# Revision 1.5  89/07/14  17:55:22  mrt
# 	Changed to use $(MACHLIB) instead of -lmach
# 	[89/07/14            mrt]
# 
# Revision 1.4  89/06/08  18:03:37  mrt
# 	Updated to work with Makeconf
# 
# Revision 1.3  89/05/05  19:03:30  mrt
# 	Cleanup for Mach 2.5. Changed CFLAGS to _CCFLAGS_.
# 	[89/05/04            mrt]
# 
#

# Makefile for miscellaneous C Threads test programs.

# Defines the following suffixes for EXECUTABLE programs:
#	.th	linked with thread implementation
#	.th_p	profiled and linked with thread implementation
#	.co	linked with coroutine implementation
#	.co_p	profiled and linked with coroutine implementation
#	.ta	linked with task implementation
#	.ta_p	profiled and linked with task implementation

INSTALL_DIR=$(DSTBASE)/tests
LIBDIR=$(DSTBASE)/lib
LIB_th = $(LIBDIR)/libthreads.a
LIB_th_p = $(LIBDIR)/libthreads_p.a
LIB_co = $(LIBDIR)/libco_threads.a
LIB_co_p = $(LIBDIR)/libco_threads_p.a
LIB_ta = $(LIBDIR)/libtask_threads.a
LIB_ta_p = $(LIBDIR)/libtask_threads_p.a

COMMONFLAGS = -DDEBUG 
CFLAGS = -g $(COMMONFLAGS)
LINTFLAGS = -u $(COMMONFLAGS)
LINTLIBS = $(LIBDIR)/llib-lthreads.ln
PROFILE = -pg

.SUFFIXES: .th .th_p .co .co_p .ta .ta_p .lint .install

.o.th:
	$(CC) $(_CCFLAGS_) -o $@ $*.o $(LIB_th) $(LIBMACH)

.o.co:
	$(CC) $(_CCFLAGS_) -o $@ $*.o $(LIB_co) $(LIBMACH)

.o.ta:
	$(CC) $(_CCFLAGS_) -o $@ $*.o $(LIB_ta) $(LIBMACH)

.c.th_p:
	$(CC) $(_CCFLAGS_) $(PROFILE) -o $@ $*.c $(LIB_th_p) $(LIBMACH)

.c.co_p:
	$(CC) $(_CCFLAGS_) $(PROFILE) -o $@ $*.c $(LIB_co_p) $(LIBMACH)

.c.ta_p:
	$(CC) $(_CCFLAGS_) $(PROFILE) -o $@ $*.c $(LIB_ta_p) $(LIBMACH)

.c.lint:
	$(LINT) $(LINTFLAGS) $*.c $(LINTLIBS)

INSTALL_OBJS = jtest.th.install  jtest.co.install \
		procon.th.install procon.co.install \
		xtest.th.install xtest.co.install

all: all.th all.co 

profiled:  all.th_p all.co_p all.ta_p

all.th: jtest.th procon.th xtest.th

all.co: jtest.co procon.co xtest.co

all.ta: jtest.ta procon.ta xtest.ta

all.th_p: jtest.th_p procon.th_p xtest.th_p

all.co_p: jtest.co_p procon.co_p xtest.co_p

all.ta_p: jtest.ta_p procon.ta_p xtest.ta_p

jtest.th: jtest.o $(LIB_th)
jtest.th_p: jtest.o $(LIB_th_p)
jtest.co: jtest.o $(LIB_co)
jtest.co_p: jtest.o $(LIB_co_p)
jtest.ta: jtest.o $(LIB_ta)
jtest.ta_p: jtest.o $(LIB_ta_p)

procon.th: procon.o $(LIB_th)
procon.th_p: procon.o $(LIB_th_p)
procon.co: procon.o $(LIB_co)
procon.co_p: procon.o $(LIB_co_p)
procon.ta: procon.o $(LIB_ta)
procon.ta_p: procon.o $(LIB_ta_p)

xtest.th: xtest.o $(LIB_th)
xtest.th_p: xtest.o $(LIB_th_p)
xtest.co: xtest.o $(LIB_co)
xtest.co_p: xtest.o $(LIB_co_p)
xtest.ta: xtest.o $(LIB_ta)
xtest.ta_p: xtest.o $(LIB_ta_p)


install: all $(INSTALL_OBJS)

$(INSTALL_OBJS): $$(@:.install=)
	-@mach_install $I $(@:.install=) $(INSTALL_DIR)
	-@touch $@

include $(SRCBASE)/Make.inc
-include $(DEP_FILE)
