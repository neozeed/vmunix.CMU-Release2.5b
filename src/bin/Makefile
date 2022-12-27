######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.26  89/05/13  19:38:30  gm0w
# 	Added y.tab.c to expr_GARBAGE.
# 	[89/05/03            gm0w]
# 
# Revision 1.25  89/01/22  12:33:20  gm0w
# 	Removed /usr/andy from login.
# 	[89/01/22            gm0w]
# 
# Revision 1.24  88/11/08  21:54:14  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /bin/

BUILD_HERE		=

date_IMODE		= 4711
date_IOWNER		= root
date_LIBS		= -lsys
df_IGROUP		= kmem
df_IMODE		= 2711
ed_ILINKS		= ${IDIR}e
expr_GARBAGE		= y.tab.c
login_IMODE		= 4711
login_IOWNER		= root
login_LIBS		= -lsys
mail_IMODE		= 4711
mail_IOWNER		= root
passwd_ILINKS		= ${IDIR}chsh
passwd_IMODE		= 4711
passwd_IOWNER		= root
passwd_LIBS		= -lsys
ps_CFLAGS		= -I${USR_MACH}/include
ps_IGROUP		= kmem
ps_IMODE		= 2711
ps_INCVERS		=
ps_LDFLAGS		= -L${USR_MACH}/lib
ps_LIBS			= -lmach -lm -ltermcap -lsys
su_ILINKS		= ${IDIR}nu ${IDIR}ru
su_IMODE		= 4711
su_IOWNER		= root
su_LIBS			= -lsys
test_ILINKS		= "${IDIR}["
wall_IMODE		= 4711
wall_IOWNER		= root
wall_LIBS		= -lsys
write_IGROUP		= tty
write_IMODE		= 2711

include ../Makefile-common
