######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.5  89/05/14  11:52:24  gm0w
# 	Added getdirentries syscall.
# 	[89/05/14            gm0w]
# 
# Revision 2.4  89/04/27  15:13:45  gm0w
# 	Changed .errfix to _errfix.
# 	[89/04/27            gm0w]
# 
# Revision 2.3  88/11/17  18:47:38  gm0w
# 	Define MACH when compiling crt0.c.
# 	[88/11/17            gm0w]
# 
# Revision 2.2  88/11/08  20:42:13  gm0w
# 	Created for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################

#
# VAX specific object files
#
CSOBJS=	Ovadvise.o Ovfork.o _exit.o accept.o access.o acct.o adjtime.o \
	bind.o brk.o cerror.o chdir.o chmod.o chown.o chroot.o close.o \
	connect.o creat.o doprnt.o dup.o dup2.o execl.o execle.o \
	exect.o execv.o execve.o fchmod.o fchown.o fcntl.o fgets.o \
	flock.o fork.o fputs.o fstat.o fsync.o ftruncate.o getdirents.o \
	getdtablesz.o getegid.o geteuid.o getgid.o getgroups.o \
	gethostid.o gethostname.o getitimer.o getpagesize.o \
	getpeername.o getpgrp.o getpid.o getppid.o getpriority.o \
	getrlimit.o getrusage.o gets.o getsockname.o getsockopt.o \
	gettimeofdy.o getuid.o htonl.o htons.o ioctl.o kill.o killpg.o \
	link.o listen.o lseek.o lstat.o mkdir.o mknod.o mount.o ntohl.o \
	ntohs.o open.o pipe.o profil.o ptrace.o puts.o quota.o read.o \
	readlink.o readv.o reboot.o recv.o recvfrom.o recvmsg.o \
	rename.o reset.o rmdir.o sbrk.o select.o send.o sendmsg.o \
	sendto.o setgroups.o sethostid.o sethostname.o setitimer.o \
	setpgrp.o setpriority.o setquota.o setregid.o setreuid.o \
	setrlimit.o setsockopt.o settimeofdy.o shutdown.o sigblock.o \
	sigpause.o sigreturn.o sigsetmask.o sigstack.o sigvec.o \
	socket.o socketpair.o stat.o swapon.o symlink.o sync.o \
	syscall.o truncate.o umask.o umount.o unlink.o utimes.o \
	vhangup.o wait.o wait3.o write.o writev.o
SSOBJS=	_setjmp.o abs.o alloca.o atof.o bcmp.o bcopy.o bzero.o fabs.o \
	ffs.o frexp.o insque.o ldexp.o modf.o nargs.o remque.o setjmp.o \
	udiv.o urem.o
VAXSOBJS=${CSOBJS} ${SSOBJS}

VAXCOBJS=ecvt.o setjmperr.o

VAXOBJS=${VAXSOBJS} ${VAXCOBJS} mon.o

VAX_GARBAGE=moncrt0.o

include Makefile-common

moncrt0.o: crt0.c
	${_CC_} -S ${CPPFLAGS} -DMCRT0 -DMACH crt0.c
	${CPP} crt0.s > x.s
	${AS} -o x.o x.s
	${LD} -x -r -o moncrt0.o x.o
	${RM} -f x.s x.o crt0.s

crt0.o:	crt0.c
	${_CC_} -S ${CPPFLAGS} -DCRT0 -DMACH crt0.c
	${CPP} crt0.s > x.s
	${AS} -o x.o x.s
	${LD} -x -r -o crt0.o x.o
	${RM} -f x.s x.o crt0.s

gcrt0.o: moncrt0.o gmon.c gmon.ex gmon.h
	${_CC_} -S ${CPPFLAGS} gmon.c
	ex - gmon.s < gmon.ex
	${AS} -o x.o gmon.s
	${LD} -x -r -o gmon.o x.o
	${RM} -f x.o gmon.s
	${LD} -x -r -o gcrt0.o moncrt0.o gmon.o
	${RM} -f gmon.o

mcrt0.o: moncrt0.o mon.c mon.ex
	${_CC_} -S ${CPPFLAGS} mon.c
	ex - mon.s < mon.ex
	${AS} -o x.o mon.s
	${LD} -x -r -o cmon.o x.o
	${RM} -f x.o mon.s
	${LD} -x -r -o mcrt0.o moncrt0.o cmon.o
	${RM} -f cmon.o

mon.o mon_p.o: mon.c mon.ex
	${_CC_} -S ${CPPFLAGS} mon.c
	ex - mon.s < mon.ex
	${AS} -o x.o mon.s
	${LD} -x -r -o $@ x.o
	${RM} -f x.o mon.s

errlst.o errlst_p.o: errlst.c _errfix
	${_CC_} -S errlst.c
	ed - < _errfix errlst.s
	${AS} -o $@ errlst.s
	${RM} -f errlst.s

${CSOBJS}: $${@:.o=.c} DEFS.h SYS.h
	${CPP} -E ${CPPFLAGS} $*.c | ${AS} -o $*.o
	${LD} -x -r $*.o
	${MV} -f a.out $*.o

${CSOBJS:.o=_p.o}: $${@:_p.o=.c} DEFS.h SYS.h
	${CPP} -E ${CPPFLAGS} -DPROF ${@:_p.o=.c} | ${AS} -o $*.o
	${LD} -x -r $*.o
	${MV} -f a.out $*.o

${SSOBJS}: $${@:.o=.s} DEFS.h SYS.h
	${CPP} -E ${CPPFLAGS} $*.s | ${AS} -o $*.o
	${LD} -x -r $*.o
	${MV} -f a.out $*.o

${SSOBJS:.o=_p.o}: $${@:_p.o=.s} DEFS.h SYS.h
	${CPP} -E ${CPPFLAGS} -DPROF ${@:_p.o=.s} | ${AS} -o $*.o
	${LD} -x -r $*.o
	${MV} -f a.out $*.o
