######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.8  89/07/04  17:42:30  gm0w
# 	Removed bci target and added release_db.o to release_OFILES.
# 	[89/07/04            gm0w]
# 
# Revision 2.7  89/01/31  10:12:08  gm0w
# 	Removed bci-cover privs pending deletion.
# 	[89/01/31            gm0w]
# 
# Revision 2.6  89/01/28  23:07:05  gm0w
# 	Added build, project, release, workon and workoff.
# 	Moved bcsauth to ../lib/authcover.  Removed bci_ILINKS.
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

IDIR			= /usr/misc/.sdm/bin/

PROGRAMS		= build project release workon workoff
SCRIPTS			= files_here

build_OFILES		= build.o project_db.o
project_OFILES		= project.o project_db.o
release_OFILES		= release.o release_db.o project_db.o
workon_OFILES		= workon.o project_db.o
workoff_OFILES		= workoff.o project_db.o

include ../../../../Makefile-common

build.o project.o project_db.o release.o workon.o workoff.o: project.h
