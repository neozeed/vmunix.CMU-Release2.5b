# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	cthread_inline.awk,v $
# Revision 1.3  89/05/05  19:01:53  mrt
# 	Cleanup for Mach 2.5
# 

# vax/cthread_inline.awk
#
# Awk script to inline critical C Threads primitives on VAX.

NF == 2 && $1 == "calls" && $2 == "$1,_mutex_try_lock" {
	print	"#	BEGIN INLINE mutex_try_lock"
	print	"	bbssi	$0,*(sp)+,1f"
	print	"	movl	$1,r0		# yes"
	print	"	jbr	2f"
	print	"1:	clrl	r0		# no"
	print	"2:"
	print	"#	END INLINE mutex_try_lock"
	continue
}
NF == 2 && $1 == "calls" && ($2 == "$1,_mutex_unlock" || $2 == "$1,_spin_unlock") {
	print	"#	BEGIN INLINE " $2
	print	"	bbcci	$0,*(sp)+,1f"
	print	"1:"
	print	"#	END INLINE " $2
	continue
}
NF == 2 && $1 == "calls" && $2 == "$0,_cthread_sp" {
	print	"#	BEGIN INLINE cthread_sp"
	print	"	movl	sp,r0"
	print	"#	END INLINE cthread_sp"
	continue
}
# default:
{
	print
}
