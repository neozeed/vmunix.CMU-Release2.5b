#!/bin/csh -f
# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

# HISTORY
# $Log:	mach_install.csh,v $
# Revision 1.4  89/06/08  17:56:27  mrt
# 	Added copyright and log
# 	[89/06/08            mrt]
# 
#  5-Jul-88 Mary Thompson
#	modified so that the file names could be
#	pathnames and the desination could be .
#  1-Jul-88 Mary Thompson
#	Forced cps and mvs to work even when the
#	destination doesn't have write permission
#  8-Jan-88  Mary Thompson 
#	Put the chmod after chown and chgrp so that
#	chgrp and setguid would both work.

set cmd=mv
set links
while (1)
	switch ($1)
	    case -s:	
			set strip="strip"
			shift
			breaksw
	    case -r:	
			set ranlib="ranlib"
			shift
			breaksw
	    case -c:	
			set cmd="cp -p"
			shift
			breaksw
	    case -d:	
			set cmp="cmp"
			shift
			breaksw
	    case -v:	
			set vecho="echo"
			shift
			breaksw
	    case -m:	
			set chmod="chmod $2"
			shift
			shift
			breaksw
	    case -o:	
			set chown="/etc/chown -f $2"
			shift
			shift
			breaksw
	    case -g:	
			set chgrp="/bin/chgrp -f $2"
			shift
			shift
			breaksw
	    case -xc:	
			set cmd="sed"
			set comments='/^[ 	]*#/d'
			shift
			breaksw
	    case -xs:	
			set cmd="sed"
			set comments='/^[ 	]*[#:]/d'
			shift
			breaksw
	    case -l:	
			set links="$links $2"
			shift
			shift
			breaksw
	    default:	
			break
			breaksw			
	endsw
end 

if ( $#argv < 2 ) then	
	echo "mach_install: no destination specified"
	exit 
endif
set dest=$argv[$#argv]

if ( $#argv > 2 ) then
if ( ! -d $dest ) then
	echo "usage: mach_install f1 f2 or f1 f2 ... dir"
	exit 
endif
endif 

if (  $1 == $dest  ) then
	echo "mach_install: can't move $1 onto itself"
	exit 
endif

foreach j ($argv)
if ( $j == $dest) then
	break
endif
if ( ! -f $j ) then
	echo "mach_install: can't open $j"
	exit 
endif
if ( -d $dest ) then
	set file=$dest/$j:t
else	set file=$dest
endif
if ( "$cmd" == "sed" ) then
	echo sed -e '<strip comments>' $j ">$file"
	sed -e '1s;^#!;&;p' -e "$comments" $j >$file
else if ( $?cmp ) then
	echo -n CMP $j $file
	if ( { cmp -s $j $file } ) then
		echo ';'
	else
		echo " THEN" $cmd
		if ( -e $file && ! -w $file ) then
			chmod u+w $file
		endif
		$cmd $j $file
	endif
else	echo $cmd $j $file
	if ( -e $file && ! -w $file ) then
		chmod u+w $file
	endif
	$cmd $j $file
endif
if ( $?strip ) then
	$strip $file
endif
if ( $?ranlib ) then
    if ($?vecho) then
	echo $ranlib $file
    endif
    $ranlib $file
endif
if ( $?chown ) then
    if ($?vecho) then
	echo $chown $file
    endif
    $chown $file
endif
if ( $?chgrp ) then
    if ($?vecho) then
	echo $chgrp $file
    endif
    $chgrp $file
endif
if ( $?chmod ) then
    if ($?vecho) then
	echo $chmod $file
    endif
    $chmod $file
endif
end

foreach i ( $links)
    if ($?vecho) then
	echo ln $file $i
    endif
    rm -f $i
    ln $file $i
end

exit

