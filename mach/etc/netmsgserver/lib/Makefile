#
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# Copyright (c) 1988 Carnegie-Mellon University
# Copyright (c) 1987 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
#
# HISTORY
# $Log:	Makefile,v $
# Revision 1.6  89/08/28  16:49:12  mrt
# 	Got rid of includes of kernel directories
# 
# Revision 1.5  89/05/02  11:04:43  dpj
# 	Fixed all files to conform to standard copyright/log format
# 
# Revision 1.4  88/10/25  17:31:40  mrt
# added -I../conf to INCL
# 
# Revision 1.3.1.1  88/10/09  00:25:31  dpj
# 	Only use STDOBJECTS by default, and do not generate a VMTP library.
# 
#
   
-include ${MACHINE}/OBJECTS
-include ${MACHINE}/STDOBJECTS

VPATH=		${$(MACHINE)VPATH}:${VPATH}

CFLAGS=		-O
CPP=		/lib/cpp
#KERNELDIR=	/usr/mk

INCL=	-I. -I../conf -I${MACHINE} 

STDOBJS=	vmtplib.o


.c.o:
	${CPP} ${INCL} -E $*.c >$*.cpp.s
	${AS} -o $*.o $*.cpp.s
	-ld -x -r $*.o
	mv a.out $*.o
	rm -f $*.cpp.s

.s.o:
	${CC} -c $*.s

all:	${$(MACHINE)STDOBJS}
install:	${$(MACHINE)STDOBJS}


libvmtp.a: vmtplib.o ${$(MACHINE)OBJS}
	@echo "building normal libvmtp.a"
	@ar cru libvmtp.a $?

vmtplib.o:	vmtplib.c
	${CC} ${CFLAGS} ${INCL} -c vmtplib.c

clean:
	rm -f *.o errs a.out core libvmtp.a
