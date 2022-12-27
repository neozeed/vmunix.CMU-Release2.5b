# @(#)Makefile	1.2 86/11/17 NFSSRC
#
# @(#)Makefile 1.1 86/09/25 SMI;
#
######################################################################
# HISTORY
# 26-May-89  Paul Parker (parker) at Carnegie-Mellon University
#	Completed SSP'izing except for defined CFLAGS to pick up
#	correct include files only in kernel source area.
#
# $Log:	Makefile,v $
# Revision 2.0  89/06/15  16:43:10  dimitris
#   Organized into a misc collection and SSPized
# 
# 
######################################################################

IDIR			= /usr/misc/.nfs/lib/

LIBRARIES		= librpcsvc.a

OFILES			= etherxdr.o klm_prot.o mountxdr.o nlm_prot.o \
			  sm_inter.o rex_xdr.o rquotaxdr.o rstatxdr.o \
			  rusersxdr.o rwallxdr.o sprayxdr.o util.o \
			  yppasswdxdr.o

HDRS			= ether.h klm_prot.h mount.h nlm_prot.h rex.h \
			  rquota.h rstat.h rusers.h rwall.h sm_inter.h \
			  spray.h yppasswd.h

CFLAGS			= -I../../include

IMODE			= 644

include ../../../../../Makefile-common

RANLIB  =       cp $@ .. ; ranlib ../$@

#librpcsvc.a: $(OBJS)
#	ar cr librpcsvc.a $(OBJS)

install:	librpcsvc.a installhdrs
	install -c -m 644 librpcsvc.a $(DESTDIR)/usr/lib
	ranlib ${DESTDIR}/usr/lib/librpcsvc.a

installhdrs: $(HDRS)
	for i in $(HDRS); do \
		(install -c -m 644 $$i ${DESTDIR}/usr/include/rpcsvc) done
#clean:
#	rm -f *.o librpcsvc.a
