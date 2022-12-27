#! /bin/csh -f
# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#

#
# HISTORY
# $Log:	mutil.csh,v $
# Revision 1.4  89/06/08  17:56:41  mrt
# 	Added copyright and log
# 
# 	21-Nov-88  Mike Kupfer @ Olivetti Research Center
# 	[89/06/08            mrt]
# 
#	scrub: handle missing FILES and handle file names beginning
#	with "."
#
#  1-Jul-88  Mary Thomoson (mrt) at Carnegie Mellon
#	Changed rules for features. added mach.h and removed ncpus.h   
#
#  8-Feb-88  Mary Thomoson (mrt) at Carnegie Mellon
# 	Changed DSTDIR to DSTBASE
#
#  5-Feb-88  Mary Thompson (mrt) at Carnegie Mellon
#	Added needtree variable which is used to avoid
#	doing the retree if the dirs$ are not needed.
#
#  8-Jan-88  Mary Thompson (mrt) at Carnegie-Mellon
#	Changed the source collection to cs.mach.sources
#
# 31-Dec-87  Mary Thompson (mrt) at Carnegie-Mellon
#	Changed the features command to define MACH
#

unalias *

set nonomatch
set loop = 1		# loop 
set begin = 0		# explicit user begin end in use
set retree = 0		# have we computed src tree directories yet
set needtree = 0	# do we need the tree
set repath = 0		# regenerate the path's

set echoo = ""		# to "test"
set reset = 0		# set var's to zero

if ($#argv) then
	set interactive = 0
else
	set interactive = 1
endif

set test = 0		# don't actually do it
set verbos = 0

while ($loop)

	unset verbose	#ignore "parsing" stuff.  Never verbose until we are executing
#			the routines.  Note: Anything that is executed immediately in
#			the switch, will never have verbose on -- so "status" and "help"
#			look good even though verbose is on.

	if (! $reset) then
		set reset = 1			#don't reset till we do the body
		set CD = 0
		set checkMakefile = 0 checkFILES = 0
		set cleanBAK = 0 cleanCKP = 0
		set cleanO = 0 cleanA = 0
		set cleanPS = 0 cleanInstall = 0
		set cleanIncludes = 0 cleanmach = 0
		set scrub = 0
		set mksuplist = 0
		set Makedep = 0
		set sources = 0 includes = 0 features = 0
		set bootstrap = 0 makeall = 0 makeinstall = 0
	endif

	if ($interactive) then
		echo -n mutil:\ \ 
		set arglist = ($<)
		set arglist = ($arglist)
	else
		set arglist = ($*)
		set loop = 0
	endif
	
	foreach arg ($arglist)
		switch($arg)
		case "begin":
			@ begin ++
			breaksw
		case "end":
			@ begin --
			breaksw

		case "cd":
			@ CD = ! $CD
			breaksw

		case "q*":
			set loop = 0
			breaksw

		case "check":
			@ checkMakefile = ! $checkMakefile
			@ checkFILES = ! $checkFILES
			set needtree = 1
			breaksw
		case "Makefile":
			@ checkMakefile = ! $checkMakefile
			set needtree = 1
			breaksw
		case "F*":
			@ checkFILES = ! $checkFILES
			set needtree = 1
			breaksw

		case "cleanBAK":
			@ cleanBAK = ! $cleanBAK
			set needtree = 1
			breaksw
		case "cleanCKP":
			@ cleanCKP = ! $cleanCKP
			set needtree = 1
			breaksw
		case "cleanO":
			@ cleanO = ! $cleanO
			set needtree = 1
			breaksw
		case "cleanA":
			@ cleanA = ! $cleanA
			set needtree = 1
			breaksw
		case "cleanPS":
			@ cleanPS = ! $cleanPS
			set needtree = 1
			breaksw
		case "cleanInstall":
			@ cleanInstall = ! $cleanInstall
			set needtree = 1
			breaksw
		case "cleanIncludes":
			@ cleanIncludes = ! $cleanIncludes
			set needtree = 1
			breaksw
		case "cleanmach":
			@ cleanmach = ! $cleanmach
			set needtree = 1
			breaksw
		case "Makedep":
			@ Makedep = ! $Makedep
			set needtree = 1
			breaksw
		case "scrub":
			@ scrub =  ! $scrub
			set needtree = 1
			breaksw

		case "mksuplist":
			@ mksuplist = ! $mksuplist
			set needtree = 1
			breaksw

		case "includes":
			@ includes = ! $includes
			breaksw
		case "src:"
		case "sources":
			@ sources = ! $sources
			breaksw
		case "features":
			@ features = ! $features
			breaksw
		case "bootstrap":
			@ bootstrap = ! $bootstrap
			breaksw
		case "makeall":
			@ makeall = ! $makeall
			breaksw
		case "makeinstall":
			@ makeinstall = ! $makeinstall
			breaksw
		case "t*":
			@ test = ! $test
			if ($test) then
				set echoo = echo
				else
				set echoo = ""
			endif
			continue
		case "v*":
			@ verbos = ! $verbos
			continue
		case "retree":
			@ retree = ! $retree
			breaksw

		case "status":
			echo begin = $begin
			echo CD = $CD
			echo checkMakefile = $checkMakefile
			echo checkFILES = $checkFILES
			echo cleanBAK = $cleanBAK
			echo cleanCKP = $cleanCKP
			echo cleanO = $cleanO
			echo cleanA = $cleanA
			echo cleanPS = $cleanPS
			echo cleanInstall = $cleanInstall
			echo cleanIncludes = $cleanIncludes
			echo cleanmach = $cleanmach
			echo scrub = $scrub
			echo mksuplist = $mksuplist
			echo Makedep = $Makedep
			echo includes = $includes
			echo sources = $sources
			echo features = $features
			echo bootstrap = $bootstrap
			echo makeall = $makeall
			echo makeinstall = $makeinstall
			echo retree = $retree
			echo needtree = $needtree
			echo test = $test
			echo verbose = $verbos
			continue
		case "help":
			echo cd: change the working directory
			echo Makefile: check that Makefiles exist in all directories
			echo FILES: check that FILES exist in all directories 
			echo cleanBAK: remove *.BAK
			echo cleanCKP: remove *.CKP
			echo cleanO: remove *.o
			echo cleanA: remove *.a
			echo cleanPS: remove *.PS
			echo cleanInstall: remove *.install
			echo cleanIncludes: remove all files in ../include tree
			echo cleanmach: remove all files in ../{bin,doc,etc,man,lib,tests}
			echo scrub: remove all source files not in FILES files
			echo mksuplist: make a sup list from all the FILES files
			echo Makedep: make zero length Makedep file wherever necessary
			echo includes: copy include files from /../mrt or a local /usr/mk directory
			echo sources: copy long or short sources from /../mrt
			echo features: build ../include/machine/DEFAULT.h
			echo bootstrap: run make with bootstrap option
			echo makeall: run make with all option
			echo makeinstall: run make with install option
			echo retree: re-evaluate the list of directories
			echo test: do not actually do anything
			echo verbose: set shell echo on
			breaksw

		case "commands":
			set verbose
			breaksw
		case "":
			continue
		default:
			echo type mutil -help for options
			breaksw

		case "old":
			set cleanBAK = 1
			set cleanCKP = 1
			set cleanO = 1
			set cleanA = 1
			set cleanPS = 1
			set cleanInstall = 1
			set Makedep = 1
			set includes = 1
			set features = 1
			set bootstrap = 1
			set makeall  = 1
			set needtree = 1
			breaksw
		case "new":
			set sources = 1
			set Makedep = 1
			set includes = 1
			set features = 1
			breaksw
		case "clean":
			set cleanBAK = 1
			set cleanCKP = 1
			set cleanO = 1
			set cleanA = 1
			set cleanPS = 1
			set cleanInstall = 1
			set Makedep = 1
			set needtree = 1
			breaksw
		case "prereq*":
			set Makedep = 1
			set needtree = 1
			set includes = 1
			set sources = 1
			set features = 1
			set bootstrap = 1
			breaksw
		endsw
	end

	if ($begin) continue
	set reset = 0
#			 let the games begin
	if ($verbos) then
		set verbose
	else
		unset verbose
	endif

	if ($CD) then
		echo Please type the directory to cd to:\ \ 
		cd $<
		set repath = 0
		set retree = 0
	endif

	if (! $repath) then
		set a = $cwd b = ""
		set basedir = ${a:h}
		while ("${a:h}" != "")
			set  b = ${a:t} a = ${a:h}
		end
		set a = ${a:t}

		set path = (. /$a/$b/{bin,etc} /usr/cs/{bin,etc} \
		    /usr/ucb /usr/bin /{bin,etc} )
		setenv CPATH ":/$a/$b/include:/usr/cs/include:/usr/include"
		setenv EPATH ":/$a/$b/maclib:/usr/cs/maclib:/usr/maclib:"
		setenv LPATH ":/$a/$b/lib:/usr/cs/lib:/usr/lib:/lib:"
		setenv MPATH ":/$a/$b/man:/usr/cs/man:/usr/man:"
		printenv | grep PATH
		set repath = 1
		echo $a $b $basedir
	endif

	if ($sources) then
		echo; time
		if (! -d /$a/$b/src) mkdir /$a/$b/src
		echo We are going to use sup to check that the sources are up to date
		if ($test) then
			set f = f
		else
			set f = ""
		endif

		set sup = /$a/$b/sup/cs.mach.sources

		echo -n Type name of log file for sup \[may be null\] \ \ 
		set logf = $<
		set ans
		while ($?ans)
			echo -n Do you want to pick up the collection from /../mrt [y]\ \ 
			set mrt = $<
			if ("$mrt" != "y" && "$mrt" != "n" && "$mrt" != "") continue
			unset ans
		end
		if ("$mrt" == "y" || "$mrt" == "") then
			cat $sup/supfile
			eval sup -dov$f $sup/supfile |& tee $logf
		else
			echo not mrt
			set ans
			while ($?ans)
				echo -n Please type machine to sup from.\ \ 
				set host = $<
				echo -n Is \"$host\" the machine you wish to use? [yes]  \ \ 
				set yn = $<
				if ("$yn" == "y" || "$yn" == "") unset ans
			end			sed "s/mrt.mach.cs.cmu.edu/$host/" $sup/supfile
			eval sed "s/mrt.mach.cs.cmu.edu/$host/" $sup/supfile | sup -dovl$f - |& tee $logf
		endif
		unset logf
	endif

		
	if ($includes) then
		echo; time
		echo We need to update the mach INCLUDE files.
		if (! -d /$a/$b/include) mkdir /$a/$b/include
		if ($test) then
			set f = f
		else
			set f = ""
		endif
		set ans
		while ($?ans)
			echo -n Machine to fetch from \[local, mrt, ... \]\ \ 
			set machine = $<
			if ("$machine" == "") then
				continue
			else if ("$machine" == "local") then
				set machine = `hostname`
				set localf
			endif
			echo -n Machine = $machine. ok? [yes]  \ \ 
			set yn = $<
			if ("$yn" == "y" || "$yn" == "") unset ans
		end
		if ($?localf) then
			set coll = cs.mk.include
			set ans
			while ($?ans)
				echo -n Please type the kernel build directory from.\ \ 
				set build = $<
				echo Is -n \"$build\" where you you built the kernel? [y]  \ \ 
				set yn = $<
				if ("$yn" == "y" || "$yn" == "") unset ans
			end
			echo setting $coll/prefix to $build
			echo $build > /$a/$b/sup/$coll/prefix
		else
			set ans
			while ($?ans)
				echo -n Collection name \[cs.mach.include, cs.mk.include\]\ \ 
				set coll = $<
				if (-d /$a/$b/sup/$coll) unset ans
			end
		endif
		set sup = /$a/$b/sup/$coll

		echo -n Type name of sup log file \[may be null\]\ \ 
		set logf = $<
		eval set `sed "s/^.*edu \(.*\)/\1/" $sup/supfile`
		if ($?localf) then
			echo $coll host=$machine hostbase=/$a/$b base=/$a/$b prefix=include crypt=$crypt
			eval echo $coll host=$machine hostbase=/$a/$b base=/$a/$b prefix=include crypt=$crypt| sup -dovel$f - |& tee $logf
		else
			echo $coll host=$machine hostbase=$hostbase base=/$a/$b prefix=include crypt=$crypt
			eval echo $coll host=$machine hostbase=$hostbase base=/$a/$b prefix=include crypt=$crypt| sup -dove$f - |& tee $logf
		endif
		unset localf host hostbase base prefix crypt logf
	endif
	
	if ($features>) then
		echo; time
		set ans
		while ($?ans)
			echo -n Please type the kernel build directory to generate sys/features.h from.\ \ 
			set build = $<
			echo -n Is \"$build\" where you you built the kernel? [y]  \ \ 
			set yn = $<
			if ("$yn" == "y" || "$yn" == "") unset ans
		end
		set ans
		while ($?ans)
			echo -n Where do you want to put the features.h file? [/$a/$b/include/machine/DEFAULT.h]
			set featfile = $<
			if ($featfile == "") set featfile = "/$a/$b/include/machine/DEFAULT.h"
			echo -n Is $featfile correct? [y]  \ \ 
			set yn = $<
			if ("$yn" == "y" || "$yn" == "") unset ans
		end
		cat $build/{cs_*.h,mach_*.h,mach.h,net_*.h,cputypes.h,romp_dualcall.h,vice.h} | sort -o /tmp/features.h

		if (-e $featfile) then
			echo diff /tmp/features.h  $featfile
			diff /tmp/features.h $featfile
		endif
		$echoo cp /tmp/features.h $featfile
		rm -f /tmp/features.h
	endif
	
	if ($bootstrap) then
		echo; time
		echo make -k DSTBASE=/$a/$b bootstrap
		$echoo make -k DSTBASE=/$a/$b bootstrap
	endif
	
	if ($makeall) then
		echo; time
		echo make -k DSTBASE=/$a/$b all
		$echoo make -k DSTBASE=/$a/$b all
	endif
	
	if ($makeinstall) then
		echo; time
		echo make -k DSTBASE=/$a/$b install
		$echoo make -k DSTBASE=/$a/$b install
	endif
	
	if (! $retree && $needtree ) then
		echo; time
		set dirs = `find . -type d -print | sed -e 's/^..//' -e 's/$/\/$sufx/' -e 's/^\.$/$sufx\//' | sort`
		echo There are $#dirs directories in this subtree
		set sufx=""
	eval	ls -d $dirs
		set retree = 1
	endif
	
	if ($checkMakefile) then
		echo; time
		set sufx=Makefile
		echo verify Makefile
		foreach i ($dirs)
	eval		set e=$i
			if (! -f $e) then
				echo $e does not exist.  Please create it.
			endif
	end
	endif
		
	if ($checkFILES) then
		echo; time
		set sufx=FILES
		echo verify FILES
		foreach i ($dirs)
	eval		set e=$i
			if (! -f $e) then
				echo $e does not exist.  Please create it.
				continue
			endif
			if (! { grep -s Makefile $e } ) then
				echo $e is missing Makefile.
			endif
			if (! { grep -s FILES $e } ) then
				echo $e is missing FILES.
			endif
		end
	endif
	
	if ($cleanBAK) then
		echo; time
		set sufx="*.BAK"
		echo flush .BAKs
	eval	$echoo rm -f $dirs
	endif	
	
	if ($cleanCKP) then
		echo; time
		set sufx="*.CKP"
		echo flush .CKPs
	eval	$echoo rm -f $dirs
	endif
	
	if ($cleanO) then
		echo; time
		set sufx="*.o"
		echo flush .os
	eval	$echoo rm -f $dirs
	endif
	
	if ($cleanA) then
		echo; time
		set sufx="*.a"
		echo flush .as
	eval	$echoo rm -f $dirs
	endif
	
	if ($cleanPS) then
		echo; time
		set sufx="*.PS"
		echo flush .PS
	eval	$echoo rm -f $dirs
	endif
	
	if ($cleanInstall) then
		echo; time
		set sufx="*.install"
		echo flush .installs
	eval	$echoo rm -f $dirs
	endif
	
	if ($cleanIncludes) then
		echo; time
		echo flush include tree
		$echoo rm -fr ../include
	endif
	
	if ($scrub) then
		echo; time
		set sufx=""
		echo scrub src tree
		set mstdir = $basedir/mstdir
		mkdir $mstdir
		echo safe directory $mstdir
		set confirm = 0
		echo -n Do you want to confirm each directory?\ \ 
		set yn = $<
		if ("$yn" == "yes" || "$yn" == "y") set confirm = 1
		echo confirm is $confirm
		foreach i ($dirs)
	eval		set e=$i
			if (! -d $e) continue
			pushd $e > /dev/null
			echo scrub: cd $cwd
			if (! -e FILES) then
				echo scrub: no FILES, skipping this directory
				popd > /dev/null
				continue
			endif
			mv `cat FILES` $mstdir
			echo scrub: removing *
			if ( ($confirm == 1) && ("`echo *`" != "*" )) then
				echo -n Are you sure you want to remove these?\ \ 
				set yn = $<
				if ("$yn" == "y" ) $echoo rm -rf *
			else 
				$echoo rm -rf *
			endif
			# can't "mv $mstdir/* ." because of files
			# starting with "."
			set putback = $cwd
			pushd $mstdir > /dev/null
			mv `cat FILES` $putback
			popd > /dev/null
			popd > /dev/null
		end
		rmdir $mstdir
	endif 

	if ($cleanmach) then
		echo; time
		echo flush mach binaries
		$echoo rm -fr /$a/$b/{bin,doc,etc,man,lib,tests}
	endif 
	
	if ($Makedep) then
		echo; time
		set sufx=Makefile
		echo generate necessary Makedeps
		foreach i ($dirs)
	eval		set e=$i
			if (! -f $e) continue
			if { grep -s Makedep $e } then
				set sufx=Makedep
	eval			$echoo cp /dev/null $i
				set sufx=Makefile
			endif
		end
	endif
		
	if ($mksuplist) then
		echo; time
		echo make sup/list from FILES
		set ans
		while ($?ans)
			echo Please type the filename for the sup list file.\ \ 
			set filename = $<
			echo Is \"$filename\" where you want to create the sup list file? [y]  \ \ 
			set yn = $<
			if ("$yn" == "y" || "$yn" == "") unset ans
		end
		set fn=/tmp/mksuplist
		cp /dev/null $fn
		foreach i ($dirs)
			set sufx=FILES
	eval		set e=$i
			if (! -f $e) continue
			foreach file (`cat $e`)
				if (-f ${e:h}/$file) echo upgrade ${e:h}/$file >>$fn
			end
		end
		sort $fn | sed '/Makedep/d' $fn
		$echoo cp $fn $filename
	#	rm -f $fn
	endif
	
	unset verbose
	echo; time
end
