# @(#)Makefile	1.4 86/10/28 NFSSRC
#
# 	Makefile	1.1	86/09/24
#
######################################################################
# HISTORY
# $Log:	Makefile,v $
# Revision 2.1  89/07/31  10:58:33  dimitris
# 	Modified for new and improved SSP and to make librpc.a.
# 	[89/01/25            parker]
# 
# Revision 2.0  89/06/15  16:38:28  dimitris
#   Organized into a misc collection and SSPized
# 
# 
######################################################################

IDIR			= /usr/misc/.nfs/lib/

LIBRARIES		= librpc.a

#ifdef	KERNEL
DISTUSER		= distuser
DISTKERN		= distkern
#endif

OFILES			= auth_none.o auth_unix.o authunix_prot.o \
			  clnt_perror.o clnt_raw.o clnt_simple.o clnt_tcp.o \
			  clnt_udp.o get_myaddress.o pmap_clnt.o \
			  pmap_getmaps.o pmap_getport.o pmap_prot.o \
			  pmap_prot2.o pmap_rmt.o rpc_prot.o rpc_callmsg.o \
			  svc.o svc_auth.o svc_auth_unix.o svc_raw.o \
			  svc_simple.o svc_tcp.o svc_udp.o xdr.o xdr_array.o \
			  xdr_float.o xdr_mem.o xdr_rec.o xdr_reference.o \
			  xdr_stdio.o

CFILES			= ${OFILES:.o=.c}

HFILES			= auth.h auth_unix.h clnt.h pmap_clnt.h \
			  pmap_prot.h rpc.h rpc_msg.h svc.h svc_auth.h \
			  types.h xdr.h


#REL_BASE		= ${SOURCEDIR:h}/${RELEASE_BASE:t}
#CFLAGS			= -DMACH_NFS \
#			  -I${REL_BASE}/kernel
CFLAGS			= -I../../include

IMODE			= 644

LINTFLAGS		= -bnuvx

include ../../../../../Makefile-common

#
# Ugh!!!
#

RANLIB	=	cp $@ .. ; ranlib ../$@

#ifdef KERNEL

KSRC			= auth_kern.c clnt_kudp.c kern_syscall.c \
			  pmap_kgetport.c subr_kudp.c svc_kudp.c xdr_mbuf.c \

KALL			= auth.h auth_unix.h clnt.h rpc_msg.h svc.h \
			  svc_auth.h types.h xdr.h auth_none.c auth_unix.c \
			  authunix_prot.c clnt_perror.c pmap_getport.c \
			  pmap_prot.c rpc_prot.c rpc_callmsg.c svc.c \
			  svc_auth.c svc_auth_unix.c xdr.c xdr_array.c \
			  xdr_float.c xdr_mem.c xdr_reference.c $(KSRC)

#endif

#install: ${HFILES}
#	-mkdir ${DESTDIR}/usr/include/rpc && \
#		chown bin ${DESTDIR}/usr/include/rpc && \
#		chmod 755 ${DESTDIR}/usr/include/rpc
#	-for i in ${HFILES}; do \
#		(install -c -m 644 $$i ${DESTDIR}/usr/include/rpc) done

tags: ${CFILES} $(KSRC) ${HFILES}
	ctags -tw ${CFILES} $(KSRC) ${HFILES}

ref: tags
	sed 's,	/.*,,' tags | \
	awk ' { printf("%-26s%-16s%s\n", $$1, $$2, $$3) }' > ref

#ifdef KERNEL
distuser:
	-mkdir ${DISTUSER}
	@for i in ${CFILES} ${HFILES} ; do \
	    echo $$i; \
	    cp disclaimer ${DISTUSER}/$$i; \
	    chmod +w ${DISTUSER}/$$i; \
	    unifdef.sh -UKERNEL $$i\
		| sed -e "s,./. <> ./,," >> ${DISTUSER}/$$i;\
	done
	@unifdef.sh -UKERNEL Makefile >> ${DISTUSER}/Makefile
	
distkern:
	-mkdir ${DISTKERN}
	@for i in $(KALL) ; do \
	    echo $$i; unifdef.sh -DKERNEL $$i > ${DISTKERN}/$$i; done
#endif
