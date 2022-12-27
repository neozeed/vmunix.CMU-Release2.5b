######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.6  89/01/28  23:07:52  gm0w
# 	Copied and redone for authcover.
# 	[89/01/26            gm0w]
# 
# Revision 2.5  89/01/24  23:42:56  mja
# 	Add "bcsauth" program.
# 	[89/01/24  23:40:44  mja]
# 
# Revision 2.4  88/11/09  21:49:46  gm0w
# 	Added release SED_OPTIONS.
# 	[88/11/09            gm0w]
# 
# Revision 2.3  88/11/09  00:25:46  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /usr/misc/.sdm/lib/

SUBDIRS			= project
PROGRAMS		= authcover
DATAFILES		= authcover.list

ANDREWDIR		= /usr/andrew

authcover_LIBS		= ${ANDREWDIR}/lib/afs/libauth.a -lsys
authcover_IOWNER	= root
authcover_IGROUP	= systems
authcover_IMODE		= 4710
authcover.list_SED_OPTIONS = ${_SED_SOURCEWARNING_} -e '/^\#/d'

include ../../../../Makefile-common
