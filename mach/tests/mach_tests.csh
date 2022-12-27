#!/bin/csh -f
# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	mach_tests.csh,v $
# Revision 1.4  89/06/08  18:01:27  mrt
# 	Put the /bin/csh -f line back at the top of the file
# 	[89/06/08            mrt]
# 
# Revision 1.3  89/05/05  19:04:25  mrt
# 	 25-10-88  Mary Thompson @ Carnegie Mellon
# 
#	Changed the way the base directory is set so
#	that it would work with a base directory more
#	that two levels deep. Assumes that the
#	current directory is on the same level as bin,etc
#
#   5-9-88   Mary Thompson @ Carnegie Mellon
#	Added -n and -h switches and usgage message
#   4-27-88  Mary Thompson @ Carnegie Mellon
#	Put " around echo of lcp server question.
#   1-5-88   Mary Thompson @ Carnegie Mellon
#	Added a call to test_netname
#  12-18-87  Mary Thompson @ Carnegie Mellon
#	Removed test for kernel version name containing TT as
#	all mach kernels are now TT and do not have that in the name
#

if ( $#argv == 0 ) then
	set a = "/usr/mach"
else if ( $1 == "-n" ) then
	set a = "/usr/nmach"
else if ( $1 == "-h" ) then
	set a = $cwd:h
else 
	echo "usage is $0 [-n] | [-h]"
	echo "  if no switch -  programs in /usr/mach are used"
	echo "  if -n switch - programs in /usr/nmach are used"
	echo "  if -h switch - programs in the current directory are used"
	exit (1)
endif
	set path = (. $a/{bin,etc,tests} /afs/cs/project/mach/root/tests \
	     /usr/cs/{bin,etc} /usr/ucb /usr/bin /{bin,etc} )
	printenv PATH
echo ""

vm_read
if (! $status) then
	echo vm_read: SUCCESSFUL TEST.
else
	echo ERROR: vm_read failed.
endif
echo ""


cowtest
if (! $status) then
	echo cowtest: SUCCESSFUL TEST.
else 
	echo ERROR: cowtest failed.
endif
echo ""


test_service
if (! $status) then
	echo test_service: SUCCESSFUL TEST.
else
	echo ERROR: test_service failed.
endif
echo ""

test_netname
if (! $status) then
	echo test_netname: SUCCESSFUL TEST.
else
	echo ERROR: test_netname failed.
endif
echo ""


ipc_test
if (! $status) then
	echo ipc_test: SUCCESSFUL TEST.
else 
	echo ERROR: ipc_test failed.
endif
echo ""


ipc_test complex
if (! $status) then
	echo ipc_test complex: SUCCESSFUL TEST.
else 
	echo ERROR: ipc_test failed.
endif
echo ""


ipc_test complex rpc
if (! $status) then
	echo ipc_test complex rpc: SUCCESSFUL TEST.
else 
	echo ERROR: ipc_test failed.
endif
echo ""


echo "Testing threads libraries (co-routine implementation):"
echo ""

procon.co $0 >&! /tmp/mach_test$$procon  
if (! $status) then
	diff /tmp/mach_test$$procon $0 > /tmp/mach_test$$diff
	if (! $status) then
		echo procon.co: SUCCESSFUL TEST.
	else 
		echo ERROR: procon.co failed.  Output differed.
	endif
	rm /tmp/mach_test$$diff
else 
	echo ERROR: procon.co failed.
endif
rm /tmp/mach_test$$procon
echo ""


xtest.co >&! /tmp/mach_test$$xtest 
if (! $status) then
	grep AAAAAAAAAA /tmp/mach_test$$xtest >&! /tmp/mach_test$$grepA
	grep BBBBBBBBBB /tmp/mach_test$$xtest >&! /tmp/mach_test$$grepB
	if (-z /tmp/mach_test$$grepA && -z /tmp/mach_test$$grepB) then
		echo xtest.co: USER CHECK: check the file /tmp/mach_test$$xtest.  
		echo This file should contain approximately the same number
		echo of A's as B's. No string of more than approximately 
		echo 10 A's or B's should be present.
	else  
		echo xtest.co: POSSIBLE ERROR: 10 concurrent A's or B's found.
		echo See file /tmp/mach_test$$xtest.
	endif
	rm /tmp/mach_test$$grepA /tmp/mach_test$$grepB
else
	echo ERROR: xtest.co failed.
endif
echo ""

jtest.co
if (! $status) then
	echo jtest.co: SUCCESSFUL TEST.
else 
	echo ERROR: jtest.co failed.
endif
echo ""




echo "Testing threads libraries (mach-threads implementation):"
echo ""

procon.th $0 >&! /tmp/mach_test$$procon
if (! $status) then
	diff /tmp/mach_test$$procon $0 > /tmp/mach_test$$diff
	if (! $status) then
		echo procon.th: SUCCESSFUL TEST.
	else 
		echo ERROR: procon.th failed.  Output differed.
	endif
	rm /tmp/mach_test$$diff
else 
	echo ERROR: procon.th failed.
endif
rm /tmp/mach_test$$procon
echo ""

xtest.co >&! /tmp/mach_test$$xtest.th
if (! $status) then
	grep AAAAAAAAAA /tmp/mach_test$$xtest >&! /tmp/mach_test$$grepA
	grep BBBBBBBBBB /tmp/mach_test$$xtest >&! /tmp/mach_test$$grepB
	if (-z /tmp/mach_test$$grepA && -z /tmp/mach_test$$grepB) then
		echo xtest.th: USER CHECK: check the file 
		echo /tmp/mach_test$$xtest.th.  This file should contain 
		echo approximately the same number of A's as B's. No 
		echo string of more than approximately 10 A's or B's 
		echo should be present.
	else  
		echo xtest.th: POSSIBLE ERROR: 10 concurrent A's or 
		echo B's found.  See file /tmp/mach_test$$xtest.th.
	endif
	rm /tmp/mach_test$$grepA /tmp/mach_test$$grepB
else
	echo ERROR: xtest.th failed.
endif
echo ""

jtest.th
if (! $status) then
	echo jtest.th: SUCCESSFUL TEST.
else 
	echo ERROR: jtest.th failed.
endif
echo ""


echo "trying lcp server: have you installed the new one?"
lcp mrt:/usr/mrt/.login /tmp/mach_test$$junk.tests
if (! $status) then
	echo lcp: SUCCESSFUL TEST
	diff /tmp/mach_test$$junk.tests /../mrt/usr/mrt/.login > /tmp/mach_test$$diff
	if (-z /tmp/mach_test$$diff) then
		echo lcp: file copied correctly.
	else 
		echo ERROR: lcp failure.  file copied /tmp/mach_test$$junk.tests 
		echo differs from /../mrt/usr/mrt/.login.  Note lcp returned successfully.
	endif
	rm /tmp/mach_test$$diff
else
	echo ERROR: lcp returned failure.  Possible timeout.
endif
echo ""
