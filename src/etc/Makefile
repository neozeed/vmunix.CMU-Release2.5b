######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.12  89/06/08  16:17:40  gm0w
# 	Added IMODE 644 for rc.{ask,page}.
# 	[89/06/08            gm0w]
# 
# Revision 2.11  89/04/28  15:39:45  gm0w
# 	Don't bother with dump.4.1.
# 	[89/04/28  14:46:44  gm0w]
# 
# Revision 2.10  89/03/05  16:53:30  gm0w
# 	Updated gettytab.  Added modttys and ttys.master.
# 	Removed enbttys/disttys.
# 	[89/03/05            gm0w]
# 
# Revision 2.9  89/02/01  15:19:06  parker
# 	Added SED_OPTIONS for inetd.conf.
# 	[89/02/01            parker]
# 
# Revision 2.8  88/11/11  16:26:23  berman
# 	Added SED_OPTIONS to disktab to strip comments.  Also added \ to
# 	all #'s in sed rules to keep make from ignoring the rest of the
# 	line.
# 	[88/11/11            berman]
# 
# Revision 2.7  88/11/08  23:06:48  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

IDIR			= /etc/

BUILD_HERE		=
FILES_HERE_FLAGS 	= -D '! -name dump.4.1' -F '! -name [Mm]akefile*'

MASTERHOST_ILINKS	= ${IDIR}masterhost
MASTERHOST_IMODE	= 644
PRINTCAP_IMODE		= 644
PRINTCAP_SED_OPTIONS	= -e '/^\#/d' -e 's/^%/\#/'
access_IMODE		= 400
access_IOWNER		= root
access_SED_OPTIONS	= -e '/^\#/d'
arp_IGROUP		= kmem
arp_IMODE		= 2711
autodump_IGROUP		= opr
autodump_IMODE		= 4750
autodump_IOWNER		= root
disktab_IMODE		= 644
disktab_SED_OPTIONS	= -e '/^\#/d'
dmesg_IGROUP		= kmem
dmesg_IMODE		= 2711
dumpattributes_IMODE	= 644
modttys_IGROUP		= opr
modttys_IMODE		= 4710
modttys_IOWNER		= root
modttys_LIBS		= -ltermcap
gettytab_SED_OPTIONS	= ${_SED_SOURCEWARNING_} -e '/^\#/d'
gettytab_IMODE		= 644
halt_IMODE		= 4711
halt_IOWNER		= root
halt_LIBS		= -lsys
inetd.conf_SED_OPTIONS	= ${_SED_SOURCEWARNING_} -e '/^\#/d'
inetd.conf_IMODE	= 644
init_IMODE		= 700
init_LIBS		= -lsys
kgmon_IGROUP		= kmem
kgmon_IMODE		= 2711
machine_SED_OPTIONS	= -e 's;MACHINE;${MACHINE};'
mount_IMODE		= 4711
mount_IOWNER		= root
mount_LIBS		= -lsys
mvtmp_IMODE		= 4711
mvtmp_IOWNER		= root
mvtmp_LIBS		= -lsys
neton_IGROUP		= opr
neton_IMODE		= 4750
neton_IOWNER		= root
oldkernel_IGROUP	= opr
oldkernel_ILINKS	= ${IDIR}newkernel
oldkernel_IMODE		= 4750
oldkernel_IOWNER	= root
overkill_IGROUP		= opr
overkill_IMODE		= 4750
overkill_IOWNER		= root
passwd.dict_IMODE	= 644
ping_IMODE		= 4711
ping_IOWNER		= root
profile_ILINKS		= ${IDIR}.profile
profile_IMODE		= 644
pstat_IGROUP		= kmem
pstat_IMODE		= 2711
pstat_INCVERS		=
quot_IGROUP		= sys
quot_IMODE		= 2711
quotaon_ILINKS		= ${IDIR}quotaoff
rc.ask_IMODE		= 644
rc.page_IMODE		= 644
reboot_IMODE		= 4711
reboot_IOWNER		= root
reboot_LIBS		= -lsys
renice_IMODE		= 4711
renice_IOWNER		= root
renice_LIBS		= -lsys
rewrite_IGROUP		= opr
rewrite_IMODE		= 4711
rewrite_IOWNER		= sys
rewrite_LIBS		= -lsys
rshd_LIBS		= -lsys
shells_IMODE		= 644
shutdown_IMODE		= 4711
shutdown_IOWNER		= root
shutdown_LIBS		= -lsys
syslog.conf_IMODE	= 644
trpt_IGROUP		= kmem
trpt_IMODE		= 2711
trsp_IGROUP		= kmem
trsp_IMODE		= 2711
ttys.master_SED_OPTIONS	= ${_SED_SOURCEWARNING_} -e '/^\#/d'
ttys.master_IMODE	= 644
umount_IMODE		= 4711
umount_IOWNER		= root
umount_LIBS		= -lsys
unix_tm_IMODE		= 644

include ../Makefile-common
