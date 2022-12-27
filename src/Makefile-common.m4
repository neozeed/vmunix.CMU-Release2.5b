undefine(`include')
changequote(`~',`~')
######################################################################
# HISTORY
# $Log:	Makefile-common.m4,v $
# Revision 2.26  89/07/13  16:54:58  mja
# 	Added definition for NOTSITE prefix;  added silent includes
# 	of Makefile-machdep and Makefile-${SITE}.
# 	[89/07/11            mja]
# 
# Revision 2.25  89/07/11  01:21:19  mbj
# 	Wrote a second style of "%_p.o: %.c" rule for producing profiled
# 	versions of .o files from .c files when cc won't allow the "-o"
# 	option to be specified along with "-c".
# 	[89/06/28            mbj]
# 
# Revision 2.24  89/07/05  16:42:08  gm0w
# 	Removed old release rules.
# 	[89/07/05            gm0w]
# 
# Revision 2.23  89/07/01  19:31:33  mja
# 	Also omit PMAX, SUN3, SUN4, and MMAX in _FILES_HERE_DEPS_.
# 
# Revision 2.22  89/06/25  12:30:23  gm0w
# 	Added OTHERS target as catch all for things not in previously
# 	defined categories.
# 	[89/06/25            gm0w]
# 
# Revision 2.21  89/05/19  13:51:17  gm0w
# 	Removed code to make cat files for manual entries.
# 	[89/05/19            gm0w]
# 
# Revision 2.20  89/05/16  15:21:12  gm0w
# 	Fixed bug in generic clean_all rule.
# 	[89/05/16            gm0w]
# 
# Revision 2.19  89/05/16  14:18:46  gm0w
# 	Split rm args for clean into two groups so that libc clean
# 	will not exceed max args.
# 	[89/05/16            gm0w]
# 
# Revision 2.18  89/05/13  19:38:17  gm0w
# 	Added initial support for "xstr".  This will hopefully be
# 	improved in the future.
# 	[89/05/13            gm0w]
# 
# Revision 2.17  89/05/02  15:11:07  gm0w
# 	Added .uu support for BUILD_HERE.
# 	[89/05/02            gm0w]
# 
# Revision 2.16  89/04/25  18:35:50  gm0w
# 	Added .uu (uudecode) single suffix rule.
# 	[89/04/25            gm0w]
# 
# Revision 2.15  89/02/06  09:20:42  gm0w
# 	Removed remaining install dependencies on build targets.
# 	[89/02/06            gm0w]
# 
# Revision 2.14  89/02/01  10:43:03  berman
# 	Added y to list of FILES_HERE extensions for making programs
# 	with single .y sources.
# 	[89/02/01            berman]
# 
# Revision 2.13  89/01/31  14:51:20  gm0w
# 	Added '@' to null commands to make them silent.  Flattened
# 	out some FILES_HERE macros.
# 	[89/01/31            gm0w]
# 
# Revision 2.12  89/01/28  23:58:02  gm0w
# 	Added SUBDIRS support.  Added tags support.  Moved
# 	Makefile-common rule to the root Makefile.
# 	[89/01/26            gm0w]
# 
# Revision 2.11  89/01/17  22:46:31  gm0w
# 	Added PERCENTD variable for %_iv.c printf()s.
# 	[89/01/17            gm0w]
# 
# Revision 2.10  89/01/14  11:49:37  gm0w
# 	Renamed old style release support to use orelease.
# 	[89/01/14            gm0w]
# 
# Revision 2.9  89/01/10  15:16:40  gm0w
# 	Fixed minor typo in previous edit ("_IDIR_" vs. "IDIR").
# 	[89/01/10            gm0w]
# 
# Revision 2.8  89/01/10  08:06:48  gm0w
# 	Removed default lint flags since none of then were always
# 	appropriate.  Changed files_here to use MAKEPSD, not VPATH,
# 	for target searching.  Added %.o as a last resort lint target
# 	when BUILD_HERE is set until there is something better to do.
# 	Added support for release stages in addition to bases so that
# 	new release program can be used.  Changed iv.c to be %_iv.c so
# 	that two kernel dependent programs can be made with one Makefile.
# 	[89/01/10            gm0w]
# 
# Revision 2.7  88/11/15  22:40:01  gm0w
# 	Only define release rules if TOBASE is defined.
# 	[88/11/15            gm0w]
# 
# Revision 2.6  88/11/12  18:21:07  gm0w
# 	Removed IDIRS code (which only worked in the non-FILES_HERE
# 	case anyway).  Reworked install rules to use more general
# 	release targets.
# 	[88/11/12            gm0w]
# 
# Revision 2.5  88/11/10  20:15:40  gm0w
# 	Changed pattern for makefile avoidance.
# 	[88/11/10            gm0w]
# 
# Revision 2.4  88/11/09  16:14:13  gm0w
# 	Missed a spot in previous bugfix.
# 	[88/11/09            gm0w]
# 
# Revision 2.3  88/11/09  15:04:23  gm0w
# 	Added small bugfixes to manual/cat rules.
# 	[88/11/09            gm0w]
# 
# Revision 2.2  88/11/08  23:20:53  gm0w
# 	Heavily modified for new and improved SSP format.
# 	[88/10/23            gm0w]
# 
######################################################################
#
# M4 preprocessor section
#
# (everything from beginning to END_OF_M4 will not appear in the output file)
#
define(dep_if,${$@_$1?$2:${$1?$2:$3}})
define(dyn_if,${$%_$1?$2:${$1?$2:$3}})
define(pct_if,${%_$1?$2:${$1?$2:$3}})
define(dep_var,${$@_$1?${$@_$1}:${$1}})
define(dyn_var,${$%_$1?${$%_$1}:${$1}})
define(pct_var,${%_$1?${%_$1}:${$1}})
define(dep_or,${$@_$1?${$@_$1}:$2})
define(dyn_or,${$%_$1?${$%_$1}:$2})
define(pct_or,${%_$1?${%_$1}:$2})
define(var_or,${$1?${$1}:$2})
define(flags_env_args,dep_var($1FLAGS) dep_var($1ENV) dep_var($1ARGS))
define(all_rule,${BUILD_HERE?_$2_files_here_:${SUBDIRS/*/__&_$2__/} ${_ALL_$1_TARGETS_/^/_$2_prefix_}})
define(make_targets,all_rule($1,$2))

#
# That's all folks
#
END_OF_M4
#
#  Default rule - All other rules appear after variable definitions
#
default: build_all;@

#
#  Use this as a dependency for any rule which should always be triggered
#  (e.g. recursive makes).
#
ALWAYS=_ALWAYS_

#
#  Set defaults for input variables that are not already defined
#
DEF_CFLAGS=var_or(DEF_CFLAGS,-O)
DEF_FILES_HERE_FLAGS=var_or(DEF_FILES_HERE_FLAGS,-s ${MACHINE})
DEF_RMFLAGS=var_or(DEF_RMFLAGS,-ef)
DEF_ARFLAGS=var_or(DEF_ARFLAGS,cr)

IOWNER=var_or(IOWNER,cs)
IGROUP=var_or(IGROUP,cs)
IMODE=var_or(IMODE,755)

#
#  Path of /usr/mach
#
USR_MACH=${RELEASE_BASE}/usr/mach

#
#  Program macros
#
AR=ar
AS=as
CC=var_or(CC,cc)
CHMOD=chmod
CP=cp
ECHO=echo
FILES_HERE=files_here
LD=ld
LINT=lint
MAKEPATH=makepath
MV=mv
NROFF=nroff
PC=pc
RANLIB=ranlib
RELEASE=release
RM=rm
SED=sed
SORT=sort
TAGS=ctags
TR=tr
XSTR=xstr
YACC=yacc

#
#  Prefix used for targets which are neither installed nor built by
#  default at the local site.  They can still be named specifically but
#  will not be included by "clean_all", "build_all", "lint_all" or
#  "install_all" rules.
#
NOTSITE=_NOTSITE_

#
#  Include any optional machine dependent alterations/overrides to the
#  current Makefile.  It often makes more sense to isolate these few
#  differences in a separate file, rather than duplicating all the
#  common contents in every ${MACHINE}/Makefile file.
#
-include .${MAKEDIR}/Makefile-machdep

#
#  Include any optional site dependent alterations/overrides to the
#  current Makefile definitions.  Typically used to attach a
#  ${NOTSITE} prefix to inconvenient targets.
#
-include .${MAKEDIR}/Makefile-${SITE}

#
#  Program flags for makefile, environment and command line args
#
_CCFLAGS_NO_IV_=flags_env_args(C) dep_var(DEF_CFLAGS)
_CCFLAGS_=${_CCFLAGS_NO_IV_} dep_if(INCVERS,-DKERNEL_FEATURES)
_PCFLAGS_=flags_env_args(P)
_YFLAGS_=flags_env_args(Y)
_LDFLAGS_=flags_env_args(LD)
_LINTFLAGS_=flags_env_args(LINT)
_TAGSFLAGS_=flags_env_args(TAGS)
_LIBS_=dep_var(LIBS)

#
#  Allow these programs to be overrided per target
#
_CC_=dep_var(CC)

#
#  Define these with default options added
#
_FILES_HERE_=${FILES_HERE_PREFIX}${FILES_HERE} -p $$MAKEPSD ${DEF_FILES_HERE_FLAGS} var_or(FILES_HERE_FLAGS,-F '! -name [Mm]akefile*')

_RELEASE_=${RELEASE_PREFIX}${RELEASE} ${RELEASE_OPTIONS}

#
#  Definitions for files_here program
#
_FILES_HERE_DIRPAT_=var_or(FILES_HERE_DIRPAT,-d 's;^.*$$;__&_$%__;p')
_FILES_HERE_FILPAT_=var_or(FILES_HERE_FILPAT,-e c -e csh -e sh -e y -e uu -f 's;^.*$$;_$%_prefix_&;p')
_FILES_HERE_DEPS_=`@${_FILES_HERE_} -D '! -name VAX ! -name SUN ! -name IBMRT ! -name PMAX ! -name SUN4 ! -name SUN3 ! -name MMAX' ${_FILES_HERE_DIRPAT_} ${_FILES_HERE_FILPAT_}`

#
#  Define all targets
#
_SET_BINARY_TARGETS_=${PROGRAMS?_BINARY_TARGETS_:${LIBRARIES?_BINARY_TARGETS_:${LIBRARIES_NO_P?_BINARY_TARGETS_:}}}
${_SET_BINARY_TARGETS_}=${PROGRAMS} var_or(LIBRARIES,${LIBRARIES_NO_P})
_ALL_TARGETS_=${_BINARY_TARGETS_} ${OBJECTS} ${SCRIPTS} ${DATAFILES} ${OTHERS}

#
# definitions for build
#
_ALL_BUILD_TARGETS_=${_BINARY_TARGETS_} ${LIBRARIES:.a=_p.a} ${OBJECTS} ${SCRIPTS} ${DATAFILES} ${OTHERS}

_BUILD_IV_=dyn_if(INCVERS,$%.iv)
_BUILD_TARGET_=dyn_if(INCVERS,$%_BUILD_IV,$%)

_PROGRAMS_=var_or(PROGRAMS,_PROGRAMS_)
_LIBRARIES_=var_or(LIBRARIES_NO_P,var_or(LIBRARIES,_LIBRARIES_))
_LIBRARIES_P_=${LIBRARIES?${LIBRARIES:.a=_p.a}:_LIBRARIES_P_}

_LIB_OFILES_=${LIBRARIES?${_OFILES_}:${LIBRARIES_NO_P?${OFILES}:}}
_LIB_P_OFILES_=${LIBRARIES?${_OFILES_:.o=_p.o}:}

_PROG_OFILES_=${PROGRAMS/*/var_or(&_OFILES,&.o)}
_ALL_OFILES_=var_or(OFILES,${_PROG_OFILES_:u})

_OFILES_=var_or(OFILES,dep_or(OFILES,${_BINARY_TARGETS_?$@.o:}))

#
# definitions for clean
#
_ALL_CLEAN_TARGETS_=${_ALL_TARGETS_}

_OFILES_CLEAN_=var_or(OFILES,dyn_or(OFILES,${_BINARY_TARGETS_?$%.o:}))
_OFILES_P_CLEAN_=${LIBRARIES?${LIBRARIES:.a=_p.a} ${_OFILES_CLEAN_:.o=_p.o}:}
_CLEAN_SOME_=$% ${_OFILES_CLEAN_} dyn_if(INCVERS,$%.iv*)
_CLEAN_MORE_=$%.out ${_OFILES_P_CLEAN_}
_CLEANSOMEFILES_=var_or(CLEANFILES,dyn_or(CLEANFILES,${_CLEAN_SOME_} dyn_var(GARBAGE)))
_CLEANMOREFILES_=dyn_if(CLEANFILES,core,${_CLEAN_MORE_})

#
# definitions for lint
#
_ALL_LINT_TARGETS_=${PROGRAMS}

_LINT_OFILES_=var_or(OFILES,dyn_or(OFILES,${PROGRAMS?$%.o:${BUILD_HERE?$%.o:}}))
_LINTFILES_=var_or(LINTFILES,dyn_or(LINTFILES,${_LINT_OFILES_:.o=.c}))

#
# definitions for tags
#
_ALL_TAGS_TARGETS_=${PROGRAMS}

_TAGS_OFILES_=var_or(OFILES,dyn_or(OFILES,${PROGRAMS?$%.o:${BUILD_HERE?$%.o:}}))
_TAGSFILES_=var_or(TAGSFILES,dyn_or(TAGSFILES,${_TAGS_OFILES_:.o=.c}))

#
# definitions for install
#
_ALL_INSTALL_TARGETS_=${_ALL_BUILD_TARGETS_}

_ITARGET_=${TOSTAGE?_TOSTAGE_:_NO_TOSTAGE_}

_INSTALL_TARGET_=dyn_if(INCVERS,$%_INSTALL_IV,${_ITARGET_}$%)
_INSTALL_BUILD_=dyn_if(INCVERS,_need_iv_prefix_$%)

_STAGEFILES_=${TOSTAGE?${FROMSTAGE?:_TOSTAGE_%}:}
_STAGEBASES_=${TOSTAGE?${FROMSTAGE?_TOSTAGE_%:}:}
_STAGEFILES_IV_=${TOSTAGE?${FROMSTAGE?:_TOSTAGE_%.iv%2}:}
_STAGEBASES_IV_=${TOSTAGE?${FROMSTAGE?_TOSTAGE_%.iv%2:}:}

#
#  Definitions for .INOBJECTDIR rule
#
_INOBJECTDIR_OPTIONS_=var_or(INOBJECTDIR_OPTIONS,-e '')
_INOBJECTDIR_FLAGS_=var_or(INOBJECTDIR_FLAGS,-d d -r c -r csh -r sh -r y -r uu)
_INOBJECTDIR_=var_or(INOBJECTDIR,${BUILD_HERE?`@${_FILES_HERE_} ${_INOBJECTDIR_FLAGS_}`:${DATAFILES}})

#
#  Definitions for default .c.o rules used by programs or libraries
#
_PROGRAM_C_O_=${PROGRAMS?.c.o:_PROGRAM_C_O_}
_LIBRARY_C_O_=${LIBRARIES?.c.o:${LIBRARIES_NO_P?.c.o:_LIBRARY_C_O_}}

# MMAX cc can't handle "-o" on a "cc -c" cmd line
_MMAX_NO_C_WITH_O_=USE_DEFICIENT_P_C_O_RULE

_NORMAL_LIBRARY_P_C_O_=${LIBRARIES?${_${MACHINE}_NO_C_WITH_O_?_NORMAL_LIBRARY_P_C_O_:%_p.o}:_NORMAL_LIBRARY_P_C_O_}
_DEFICIENT_LIBRARY_P_C_O_=${LIBRARIES?${_${MACHINE}_NO_C_WITH_O_?%_p.o:_DEFICIENT_LIBRARY_P_C_O_}:_DEFICIENT_LIBRARY_P_C_O_}

#
#  Definitions for using xstr
#
_XSTR_C_O_=${XSTR_OFILES?%_x.o:_XSTR_C_O_}

#
#  Definitions for rules using sed
#
_NOT_A_SOURCE_FILE_=THIS IS NOT A SOURCE FILE - DO NOT EDIT
_SED_SOURCEWARNING_=-e 's;^#\(.*\)\@SOURCEWARNING\@;\1${_NOT_A_SOURCE_FILE_};p'
_SED_STRIP_=-e '1s;^#!;&;p'
_SED_CSH_STRIP_=-e '/^[ 	]*\#/d'
_SED_SH_STRIP_=-e '/^[ 	]*[\#:]/d'
_SED_CHMOD_=+x

#
#  Default single suffix compilation rules
#
.SUFFIXES:
.SUFFIXES: .o .s .c .y .l .p .sh .csh .txt .uu

.c:
	${_CC_} ${_LDFLAGS_} ${_CCFLAGS_} -o $*.out $*.c ${_LIBS_}
	${MV} $*.out $@

.p:
	${PC} ${_PCFLAGS_} -o $*.out $*.p ${_LIBS_}
	${MV} $*.out $@

.csh:
	@echo "csh-compile $< >$*.out"
	@${SED}\
	 ${_SED_STRIP_}\
	 ${_SED_SOURCEWARNING_}\
	 dep_var(SED_OPTIONS)\
	 ${_SED_CSH_STRIP_} $< >$*.out
	${CHMOD} ${_SED_CHMOD_} $*.out
	${MV} -f $*.out $*

.sh:
	@echo "sh-compile $< >$*.out"
	@${SED}\
	 ${_SED_STRIP_}\
	 ${_SED_SOURCEWARNING_}\
	 dep_var(SED_OPTIONS)\
	 ${_SED_SH_STRIP_} $< >$*.out
	${CHMOD} ${_SED_CHMOD_} $*.out
	${MV} -f $*.out $*

.txt:
	@echo "txt-compile $< >$*.out"
	@${SED}\
	 ${_SED_SOURCEWARNING_}\
	 dep_var(SED_OPTIONS)\
	 $< >$*.out
	${MV} -f $*.out $*

.y:
	${YACC} ${_YFLAGS_} $*.y
	${_CC_} ${_LDFLAGS_} ${_CCFLAGS_} -o $*.out y.tab.c
	${MV} -f $*.out $@
	${RM} -f y.tab.c

.uu:
	uudecode $*.uu

#
#  Default double suffix compilation rules
#
${_PROGRAM_C_O_}:
	${_CC_} -c ${_CCFLAGS_} $*.c

${_LIBRARY_C_O_}:
	${_CC_} -c ${_CCFLAGS_} $*.c
	${LD} ${_LDFLAGS_} -x -r $*.o
	${MV} -f a.out $*.o

${_NORMAL_LIBRARY_P_C_O_}: %.c
	${_CC_} -c ${_CCFLAGS_} -o %_p.o -p %.c
	${LD} ${_LDFLAGS_} -x -r %_p.o
	${MV} -f a.out %_p.o

${_DEFICIENT_LIBRARY_P_C_O_}: %.c
	-${RM} -f %_p.c
	ln -s %.c %_p.c
	${_CC_} -c ${_CCFLAGS_} -p %_p.c
	-${RM} -f %_p.c
	${LD} ${_LDFLAGS_} -x -r %_p.o
	${MV} -f a.out %_p.o

${_XSTR_C_O_}: %.c
	${_CC_} -E ${_CCFLAGS_} %.c | ${XSTR} -c -
	${_CC_} -c ${_CCFLAGS_} x.c
	${MV} -f x.o %_x.o
	${RM} -f x.c

.p.o:
	${PC} -c ${_PCFLAGS_} $*.p

#
#  Special rules
#

#
#  Use this as a dependency for any rule which should always be triggered
#  (e.g. recursive makes).
#
${ALWAYS}:;@

#
#  Default update rule
#
.INOBJECTDIR: ${_INOBJECTDIR_}
	@echo "copy-here <$@ >$@.out"
	@${SED} dep_var(SED_OPTIONS) ${_INOBJECTDIR_OPTIONS_} <$@ >./$@.out
	${MV} -f ./$@.out ./$@

#
#  Change into a directory and make recursively
#
__%1_%2__: `@-${MAKEPATH} %1/%2; echo ${ALWAYS}`
	@cd %1 &&\
	 echo "cd %1 && ${MAKE} %2_all" &&\
	 echo "[ ${IDIR}%1 ]" &&\
	 exec ${MAKE} %2_all

#
#  Special include version compilation rules
#
%.iv%2: %
	${CP} -p $> $@

PERCENTD=%d

%_iv.c:
	@echo '#include <sys/version.h>' >%_iv.c
	@echo '#include <stdio.h>' >>%_iv.c
	@echo 'main() {' >>%_iv.c
	@echo '  fprintf(stderr,"INCLUDE VERSION: ${PERCENTD}\n",INCLUDE_VERSION);' >>%_iv.c
	@echo '  printf("${PERCENTD}\n", INCLUDE_VERSION);' >>%_iv.c
	@echo '}' >>%_iv.c

%.iv:	%_iv.c
	${_CC_} ${_LDFLAGS_} -DKERNEL_FEATURES ${_CCFLAGS_NO_IV_} -o %.iv.out %_iv.c
	./%.iv.out >%.iv
	@${RM} -f ./%.iv.out %_iv.c

%.iv%2.iv: %.iv
	echo %2 | cmp -s - %.iv

#
#  Generate files here for target
#
_%_files_here_: $${_FILES_HERE_DEPS_};@

#
#  Build rules
#
build_all: make_targets(BUILD,build);@

all:	${ALWAYS}
	@echo "$@ is no longer an acceptable target for build..."; exit 1

_build_prefix_%: $${_BUILD_IV_} $${_BUILD_TARGET_};@

_build_prefix_%.iv%2: %.iv%2.iv %.iv%2;@

_build_prefix_${NOTSITE}%:
	@echo % is not used at ${SITE}

${_PROGRAMS_}: $${_OFILES_}
	${_CC_} ${_LDFLAGS_} -o $@.out ${_OFILES_} ${_LIBS_}
	${MV} $@.out $@

${_LIBRARIES_}: ${_LIBRARIES_}($${_LIB_OFILES_})
	${AR} ${DEF_ARFLAGS} $@ $?
	${RANLIB} $@
	${RM} -f $?

${_LIBRARIES_P_}: ${_LIBRARIES_P_}($${_LIB_P_OFILES_})
	${AR} ${DEF_ARFLAGS} $@ $?
	${RANLIB} $@
	${RM} -f $?

${_ALL_OFILES_}: ${HFILES}

%_BUILD_IV: `@echo -n %.iv | cat - %.iv`;@

#
#  Clean up rules
#
clean_all: make_targets(CLEAN,clean)
	@${RM} ${DEF_RMFLAGS} core

clean:	${ALWAYS}
	@echo "$@ is no longer an acceptable target for build..."; exit 1

_clean_prefix_%:
	@${RM} ${DEF_RMFLAGS} ${_CLEANSOMEFILES_}
	@${RM} ${DEF_RMFLAGS} ${_CLEANMOREFILES_}

_clean_prefix_${NOTSITE}%:;@

#
#  Lint rules
#
lint_all: make_targets(LINT,lint);@

_lint_prefix_%: $${_LINTFILES_}
	${LINT} ${_LINTFLAGS_} $>

_lint_prefix_${NOTSITE}%:;@

#
#  Tags rules
#
tags_all: make_targets(TAGS,tags);@

_tags_prefix_%: $${_TAGSFILES_}
	${TAGS} ${_TAGSFLAGS_} $>

#
#  Installation/release rules
#
install_all: make_targets(INSTALL,install);@

_install_prefix_%: $${_INSTALL_TARGET_};@

_install_prefix_%.iv%2: $${_ITARGET_}%.iv%2;@

_install_prefix_${NOTSITE}%:;@

_need_iv_prefix_%: ${ALWAYS}
	@echo "To release %, you must use %.iv#"; exit 1

%_INSTALL_IV: `@echo -n $${_ITARGET_}%.iv | cat - %.iv`;@

${_STAGEFILES_}: $${_INSTALL_BUILD_} ${ALWAYS}
	${_RELEASE_} -o pct_var(IOWNER)\
		-g pct_var(IGROUP)\
		-m pct_var(IMODE)\
		-tostage ${TOSTAGE}\
		-fromfile %\
		pct_var(IDIR)%\
		pct_var(ILINKS)

${_STAGEBASES_}: $${_INSTALL_BUILD_} ${ALWAYS}
	${_RELEASE_} -o pct_var(IOWNER)\
		-g pct_var(IGROUP)\
		-m pct_var(IMODE)\
		-tostage ${TOSTAGE}\
		-fromstage ${FROMSTAGE}\
		pct_var(IDIR)%\
		pct_var(ILINKS)

${_STAGEFILES_IV_}: ${ALWAYS}
	${_RELEASE_} -o pct_var(IOWNER)\
		-g pct_var(IGROUP)\
		-m pct_var(IMODE)\
		-tostage ${TOSTAGE}\
		-fromfile %.iv%2\
		pct_var(IDIR)%.iv%2\
		pct_var(ILINKS)

${_STAGEBASES_IV_}: ${ALWAYS}
	${_RELEASE_} -o pct_var(IOWNER)\
		-g pct_var(IGROUP)\
		-m pct_var(IMODE)\
		-tostage ${TOSTAGE}\
		-fromstage ${FROMSTAGE}\
		pct_var(IDIR)%.iv%2\
		pct_var(ILINKS)
