######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.6  89/05/10  16:58:20  gm0w
# 	Changed sun CFLAGS to define sun3.  Added MACH definition.
# 	[89/05/10            gm0w]
# 
# Revision 1.5  88/11/08  20:41:57  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /usr/cs/lib/

VAXCFLAGS		= -Dvax=1
SUNCFLAGS		= -Dsun=1 -Dsun3=1 -DBYTE_MSF=1 -DBIT_MSF=1
SUN3CFLAGS		= -Dsun=1 -Dsun3=1 -DBYTE_MSF=1 -DBIT_MSF=1
IBMRTCFLAGS		= -Dibmrt=1 -DBYTE_MSF=1 -DBIT_MSF=1

PROGRAMS		= cpp
CFLAGS			= -Dunix=1 -DMACH=1 -DFLEXNAMES ${${MACHINE}CFLAGS}
OFILES			= cpp.o cpy.o rodata.o
GARBAGE			= cpy.c rodata.c

rodata.o_CFLAGS		= ${CFLAGS} -R

include ../../../../Makefile-common

cpy.o:	yylex.c
cpy.c rodata.c:	cpy.y .yyfix
	${YACC} ${_YFLAGS_} cpy.y
	.yyfix yyexca yyact yypact yypgo yyr1 yyr2 yychk yydef
	${MV} -f y.tab.c cpy.c
