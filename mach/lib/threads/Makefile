# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.12.1.1  89/10/24  23:22:06  mrt
# 	Fixed c to o rule to print out a briefer line
# 
# 	Jonathan J. Chew (jjc) at Carnegie Mellon University
# 	Changed "sun3_" macros to "sun_".
# 	[89/10/24            mrt]
# 
# Revision 1.12  89/08/28  16:12:15  mrt
# 	Renamed the target msg.c to co_msg.c so that make
# 	doesn't get confused.
# 	Made MD, and ASFLAGS and inline stuff machine dependent 
# 	so that pmax could be different.
# 	[89/08/01            mrt]
# 
# Revision 1.11  89/07/30  20:37:47  mrt
# 	Got rid of references to $(SRCBASE)/lib/libmach and added
# 	../libmach to VPATH so that shadow builds would work.
# 	[89/07/20            mrt]
# 
# Revision 1.10  89/06/21  14:44:50  mrt
# 	Made lint and llib targets depend on $(SRCS) so that
# 	make would rewrite the file names to be in the source
# 	rather than object directory
# 	[89/06/21            mrt]
# 
# Revision 1.9  89/06/08  18:28:15  mrt
# 	Removed install of cthreads.h as it is now in the mach_include
# 	directory.
# 	[89/05/28            mrt]
# 
# Revision 1.8  89/05/24  15:35:31  mrt
# 	more upgrades for separate OBJECTDIR
# 
# Revision 1.7  89/05/18  11:17:00  mbj
# 	Upgraded Makefile for use with separate OBJECTDIR.
# 	[89/05/17            mbj]
# 
# 	Added stack_base.c.
# 	[89/03/31            mbj]
# 
# Revision 1.6  89/05/05  18:47:42  mrt
# 	Made COMMONFLAGS include -DMACH and -D$(machine) until cpp is
# 	fixed to define them.
# 	[89/05/04            mrt]
# 
# 24-Mar-89  Michael Jones (mbj) at Carnegie-Mellon University
#	Implement fork() for multi-threaded programs.
#	Pass SRCBASE to make subprocesses.
#	Made MTASK version work correctly again.  Don't build by default until
#	the kernel vm_inherit bug is fixed.
#

#
# Makefile for C Threads library.


# Parameters for "generic" rules:
#
#	LIB		C thread library
#			libthreads.a, libco_threads.a, or libtask_threads.a
#	IMPL		implementation option
#			MTHREAD, COROUTINE, or MTASK
#	PROFILE		profiling switch
#			(null, -p, or -pg)
#
# Implicit parameters:
#
#	machine		machine type, set by make
#
# A random value of LIB is given here so that
# the Makefile will be syntactically correct.
# The actual value of LIB will always be passed
# on the command line in a recursive call to make.

LIB = unknown.a

VPATH=:${machine}:../libmach

COMMONFLAGS = -I. -DDEBUG -D$(IMPL) -DMACH -D$(machine)
CFLAGS = -g $($(machine)_MD) $(COMMONFLAGS) $(PROFILE)
vax_ASFLAGS=
ibmrt_ASFLAGS=
sun_ASFLAGS=
pmax_ASFLAGS=-nocpp
vax_MD=-MD
ibmrt_MD=-MD
sun_MD=-MD
mmax_MD=-MD
pmax_MD=
#enable inline expansion
vax_INLINE_DATA = cthread_inline.awk
vax_INLINE = awk -f cthread_inline.awk
ibmrt_INLINE_DATA = cthread_inline.awk
ibmrt_INLINE = awk -f cthread_inline.awk
sun_INLINE_DATA = cthread_inline.awk
sun_INLINE = awk -f cthread_inline.awk
mmax_INLINE_DATA = cthread_inline.awk
mmax_INLINE = awk -f cthread_inline.awk
#disable inline expansion: pmax assembler can't handle it and it is unecessary
pmax_INLINE_DATA =
pmax_INLINE = cat
LINTFLAGS = -u $(COMMONFLAGS)

.SUFFIXES: .c .o

.c.o:
	@echo "$(CC) $(CFLAGS) $*.c"
	@trap "$(RM) $(RMFLAGS) $*.s $*.S" 0 1 2 3 10 15; \
	$(CC) $(CFLAGS) -S $*.c; \
	$(MV) $*.s $*.S; \
	$($(machine)_INLINE) $*.S > $*.s; \
	$(AS) $($(machine)_ASFLAGS) -o $@ $*.s; 
	$(RM) $(RMFLAGS) $*.s $*.S

SRCS = cprocs.c cthreads.c malloc.c \
       mig_support.c stack.c stack_base.c sync.c \
       time.c ipc.c msg.c unix.c fork.c \
       thread.c

OBJS = $(LIB)(cprocs.o) $(LIB)(cthreads.o) $(LIB)(malloc.o) \
       $(LIB)(mig_support.o) $(LIB)(stack.o) $(LIB)(stack_base.o) $(LIB)(sync.o) \
       $(LIB)(time.o) $(LIB)(ipc.o) $(LIB)(unix.o) $(LIB)(fork.o) \
       $(LIB)(co_msg.o) \
       $(LIB)(thread.o) \
       $(LIB)(csw.o) $(LIB)(lock.o)

$(SRCS:.c=.o): $($(machine)_INLINE_DATA)

I = -c -d -v

TH_LIB = threads
CO_LIB = co_threads
TA_LIB = task_threads

all: thread coroutine # task

all.prof: thread.prof coroutine.prof # task.prof

all.lint: thread.lint coroutine.lint # task.lint

all.llib: thread.llib coroutine.llib # task.llib

thread:
	$(MAKE) $(MFLAGS) DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) LIB=lib$(TH_LIB).a IMPL=MTHREAD lib

thread.prof:
	$(MAKE) $(MFLAGS)  DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) LIB=lib$(TH_LIB)_p.a IMPL=MTHREAD PROFILE=-pg lib

thread.lint:
	$(MAKE) $(MFLAGS) LIB=lib$(TH_LIB).a IMPL=MTHREAD lint

thread.llib:
	$(MAKE) $(MFLAGS) LIB=$(TH_LIB) IMPL=MTHREAD llib

coroutine:
	$(MAKE) $(MFLAGS)  DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) LIB=lib$(CO_LIB).a IMPL=COROUTINE lib

coroutine.prof:
	$(MAKE) $(MFLAGS)  DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) LIB=lib$(CO_LIB)_p.a IMPL=COROUTINE PROFILE=-pg lib

coroutine.lint:
	$(MAKE) $(MFLAGS) LIB=lib$(CO_LIB).a IMPL=COROUTINE lint

coroutine.llib:
	$(MAKE) $(MFLAGS) LIB=$(CO_LIB) IMPL=COROUTINE llib

task:
	$(MAKE) $(MFLAGS)  DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) LIB=lib$(TA_LIB).a IMPL=MTASK lib

task.prof:
	$(MAKE) $(MFLAGS)  DSTBASE=$(DSTBASE) SRCBASE=$(SRCBASE) LIB=lib$(TA_LIB)_p.a IMPL=MTASK PROFILE=-pg lib

task.lint:
	$(MAKE) $(MFLAGS) LIB=lib$(TA_LIB).a IMPL=MTASK lint

task.llib:
	$(MAKE) $(MFLAGS) LIB=$(TA_LIB) IMPL=MTASK llib

install: all all.prof all.llib ckdir
	mach_install $I -r lib$(TH_LIB).a $(DSTBASE)/lib
	mach_install $I -r lib$(TH_LIB)_p.a $(DSTBASE)/lib
	mach_install $I llib-l$(TH_LIB).ln $(DSTBASE)/lib/lint
	mach_install $I -r lib$(CO_LIB).a $(DSTBASE)/lib
	mach_install $I -r lib$(CO_LIB)_p.a $(DSTBASE)/lib
	mach_install $I llib-l$(CO_LIB).ln $(DSTBASE)/lib/lint

ckdir:
	-if [ -d $(DSTBASE)/lib ]; then true; else mkdir $(DSTBASE)/lib; fi
	-if [ -d $(DSTBASE)/lib/lint ]; then true; else mkdir $(DSTBASE)/lib/lint; fi

clean:
	$(RM) $(RMFLAGS) machine lib*.a Makedep* co_msg.c a.out core errs \
			 *.d *.s *.S *.o *.BAK *.CKP */*.BAK */*.CKP

# The following are the "generic" rules,
# parameterized by LIB, IMPL, and PROFILE.

lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) r$(ARFLAGS) $(LIB) $?
	$(RANLIB) $(LIB)
	$(RM) $(RMFLAGS) $?
# Avoid complaints from md in case Makedep doesn't exist yet.
	touch Makedep$(IMPL)
	md -m Makedep$(IMPL) -d *.d
# Massage dependency file to reflect object files within library.
	cp Makedep$(IMPL) Makedep$(IMPL).BAK
	sed 's/^[^ #$$]*\.o/$$(LIB)(&)/' < Makedep$(IMPL).BAK > Makedep$(IMPL)
	rm Makedep$(IMPL).BAK

co_msg.c:  msg.c msg.sed
	sed -f msg.sed msg.c > co_msg.c

$(LIB)(co_msg.o): co_msg.c
	-if [ "$(IMPL)" = COROUTINE ]; then \
		$(CC) $(CFLAGS) -c co_msg.c ; \
	else \
		$(AS) -o co_msg.o /dev/null ; \
	fi

$(LIB)(thread.o): thread.c
	$(CC) $(CFLAGS) -c thread.c

$(LIB)(csw.o): csw.s
	-if [ "$(IMPL)" = COROUTINE ]; then \
		$(AS) -o csw.o csw.s ; \
	else \
		$(AS) -o csw.o /dev/null ; \
	fi

$(LIB)(lock.o): lock.s
	-if [ "$(IMPL)" = COROUTINE ]; then \
		$(AS) -o lock.o /dev/null ; \
	else \
		$(AS) -o lock.o lock.s ; \
	fi

lint:  $(SRCS)
	-if [ -f /usr/cs/bin/lint ]; then \
		$(LINT) $(LINTFLAGS) $(SRCS) -lmach ; \
	else \
		exit 0 ; \
	fi

llib:  $(SRCS)
	-if [ -f /usr/cs/bin/lint ]; then \
		$(LINT) $(LINTFLAGS) -C$(LIB) $(SRCS) ; \
	else \
		exit 0 ; \
	fi

-include Makedep$(IMPL)
