#! /bin/csh -f
# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	runmake.csh,v $
# Revision 1.7  89/06/08  17:56:35  mrt
# 	Set CENV to define MACH and machine name until cpp does so.
# 	[89/05/06            mrt]
# 
#

#
#  Sets the paths correctly, sets the CC value to
#  hc if it exists or cc otherwise
#  runs make
	set a = $cwd b = ""
	while ("${a:h}" != "")
		set  b = ${a:t} a = ${a:h}
	end
	set a = ${a:t}

	echo $a  $b

	set path = (. /$a/$b/{bin,etc} /usr/misc/bin \
		/usr/cs/{bin,etc} /usr/ucb /usr/bin /{bin,etc} )
	setenv CPATH ":/$a/$b/include:/usr/cs/include:/usr/include"
	setenv EPATH ":/$a/$b/maclib:/usr/cs/maclib:/usr/maclib:"
	setenv LPATH ":/$a/$b/lib:/usr/misc/.hc/lib:/usr/cs/lib:/usr/lib:/lib"
	setenv MPATH ":/$a/$b/man:/usr/cs/man:/usr/man:"
	printenv | grep PATH

	if ( `/etc/machine` == "SUN" ) then
		setenv CENV "-DMACH -Dsun3"
	else if ( `/etc/machine` == "VAX" ) then
		setenv CENV "-DMACH -Dvax"
	else if ( `/etc/machine` == "IBMRT" ) then
		setenv CENV "-DMACH -Dibmrt"
	endif

	echo make DSTBASE=/$a/$b $*
	make  DSTBASE=/$a/$b  $*

