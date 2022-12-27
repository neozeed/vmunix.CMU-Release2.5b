#! /bin/csh -f

######################################################################
# HISTORY
# $Log:	files_here.csh,v $
# Revision 2.3  88/11/09  01:04:45  gm0w
# bci cannot handle this...
# 
# Revision 2.2.1.1  88/11/09  01:02:38  gm0w
# 	RCS screwed up end of file, lets try again...
# 	[88/11/09            gm0w]
# 
# Revision 2.2  88/11/09  00:26:24  gm0w
# 	Created.
# 	[88/11/09            gm0w]
# 
######################################################################

set prog=$0
set prog=${prog:t}

set usage="usage: ${prog} [ options ... ]"

set nonomatch

#  Init variables
set path_arg=""
set subdirs=(.)
set sed_dir=()
set sed_file=()
set find_dir_args=()
set find_file_args=()
set strip_ext=()
set rm_ext=()

#  Process arguments
while (${#argv} != 0)
    if ("${argv[1]}" !~ -*) break
    switch ("${argv[1]}")
    case -D:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set find_dir_args=(${argv[2]:q})
	shift
	breaksw
    case -F:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set find_file_args=(${argv[2]:q})
	shift
	breaksw
    case -d:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set sed_dir=(${sed_dir:q} -e ${argv[2]:q})
	shift
	breaksw
    case -f:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set sed_file=(${sed_file:q} -e ${argv[2]:q})
	shift
	breaksw
    case -e:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set strip_ext=(${strip_ext:q} -e 's;^\(.*\)\.'"${argv[2]:q}"'$;\1;')
	shift
	breaksw
    case -r:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set rm_ext=(${rm_ext:q} -e '/^\(.*\)\.'"${argv[2]:q}"'$/d')
	shift
	breaksw
    case -p:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set path_arg="${argv[2]}"
	shift
	breaksw
    case -s:
	if ($#argv < 2) then
	    echo "${prog}: missing argument to: ${argv[1]}"
	    exit 1
	endif
	set subdirs=(${subdirs:q} ${argv[2]:q})
	shift
	breaksw
    default:
	echo "$usage"
	exit 1
    endsw
    shift
end

#  Default is to just print subdirectories and regular files
if ($#sed_dir == 0) set sed_dir=(-e p)
if ($#sed_file == 0) set sed_file=(-e p)

#  Remove, strip and modify
set sed_file=(${rm_ext:q} ${strip_ext:q} ${sed_file:q})

#  Generate list of directories which exist from -p and -s arguments
set find_dirs=`(echo ${path_arg:q}; echo ${subdirs:q}) | awk ' NR == 1 { npaths = split($0, paths, ":"); next; } { for (i = 1; i <= npaths; i++) { p = paths[i]; for (j = 1; j <= NF; j++) { if (p == "." || p == "") { if ($j == "." || $j == "") f = "."; else f = $j; } else { if ($j != "." && $j != "") f = p "/" $j; else f = p; } print "if (-d \"" f "\") echo \"" f "\""; } } }' | /bin/csh -f`
if ("$find_dirs" == "") set find_dirs="."

#  Find subdirectories and files, sed, and generate sorted unique list
set noglob
(find $find_dirs -relative -maxdepth 1 -depth 1 \
    -type d ${find_dir_args} -print | sed -n ${sed_dir:q} ; \
 find $find_dirs -relative -maxdepth 1 -depth 1 \
    -type f ${find_file_args} -print | sed -n ${sed_file:q} ) \
 | \
sort -u

exit 0
