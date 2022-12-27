# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

#! /bin/csh -f
# HISTORY
# $Log:	bin_tests.csh,v $
# Revision 1.4  89/05/05  19:03:15  mrt
# 	  10-14-88 Mary Thompson @ Carnegie Mellon
# 
#	Removed calls to unsuported programs
#   1-5-87   Mary Thompson @ Carnegie Mellon
#	Started. Calls most of the useful programs in /usr/mach/bin

	set a = $cwd b = ""
	while ("${a:h}" != "")
		set  b = ${a:t} a = ${a:h}
	end
	set a = ${a:t}
	set path = (. /$a/$b/{bin,etc,tests} /usr/cs/{bin,etc} \
	    /usr/ucb /usr/bin /{bin,etc} )
	setenv CPATH ":/$a/$b/include:/usr/cs/include:/usr/include"
	setenv EPATH ":/$a/$b/maclib:/usr/cs/maclib:/usr/maclib:"
	setenv LPATH ":/$a/$b/lib:/usr/cs/lib:/usr/lib:/lib:"
	setenv MPATH ":/$a/$b/man:/usr/cs/man:/usr/man:"
	printenv 
echo ""

echo "hostinfo"
	hostinfo
echo ""

echo "macherr 3"
	macherr 3
echo ""

echo ""

echo "vm_stat"
	vm_stat
echo ""

echo "zprint"
	zprint
echo ""

#echo "vmmprint -p 1"
#	vmmprint -p 1
#echo "vmoprint > /tmp/vmoprint.out"
#	vmoprint > /tmp/vmoprint.out
#echo "vmpinfo > /tmp/vmpinfo.out"
#	vmpinfo > /tmp/vmpinfo.out

