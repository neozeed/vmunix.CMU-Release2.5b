######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.7  88/11/08  21:55:03  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /bin/

AS_DIR			= ../as
DEFS_H			= defs.h mac.h mode.h head.h machine.h

VPATH			= ${AS_DIR}
PROGRAMS		= adb
OFILES			= access.o command.o expr.o format.o input.o main.o\
			  message.o opset.o optab.o output.o pcs.o print.o\
			  runpcs.o setup.o sym.o
INCVERS			=
CFLAGS			= -w -DADB -I${MACHINE}
GARBAGE			= instrs.adb

message.o_CFLAGS	= ${CFLAGS} -R
opset.o_CFLAGS		= ${CFLAGS} -I. -I${AS_DIR}/${MACHINE}
optab.o_CFLAGS		= ${CFLAGS} -R -I. -I${AS_DIR}/${MACHINE}

include ../../Makefile-common

access.o:	${DEFS_H} access.c
command.o:	${DEFS_H} command.c
expr.o:		${DEFS_H} expr.c
format.o:	${DEFS_H} format.c
input.o:	${DEFS_H} input.c
main.o:		${DEFS_H} main.c
message.o:	mac.h mode.h machine.h message.c
pcs.o:		${DEFS_H} pcs.c
print.o:	${DEFS_H} print.c
runpcs.o:	${DEFS_H} runpcs.c
setup.o:	${DEFS_H} setup.c
sym.o:		${DEFS_H} sym.c
opset.o:	${DEFS_H} opset.c instrs.h asnumber.h
optab.o:	${DEFS_H} optab.c instrs.adb instrs.h assizetab.c
output.o:	${DEFS_H} output.c

instrs.adb: instrs
	(echo FLAVOR ADB; cat instrs) | awk -f instrs > instrs.adb
