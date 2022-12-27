######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 1.11  89/05/02  15:13:07  gm0w
# 	Removed temporary code to not make defunct targets.
# 	[89/05/02            gm0w]
# 
# Revision 1.10  89/01/21  08:43:59  parker
# 	Created for new and improved SSP.  Contains interim definition
# 	of FILES_HERE_FLAGS so that certain productions are not made.
# 	[89/01/21            parker]
# 
######################################################################

IDIR			= /usr/ucb/

BUILD_HERE		=

clear_LIBS		= -ltermlib
ul_LIBS			= -ltermlib
vacation_LIBS		= -ldbm
w_CFLAGS		= -DKERNEL_FEATURES -I${USR_MACH}/include
w_IGROUP		= kmem
w_IMODE			= 2711
w_INCVERS		=
w_LDFLAGS		= -L${USR_MACH}/lib
w_LIBS			= -lsys
finger_ILINKS		= ${IDIR}f
man_ILINKS		= ${IDIR}apropos ${IDIR}whatis
quota_IMODE		= 4711
quota_IOWNER		= root
users_ILINKS		= ${IDIR}u

include ../../Makefile-common
