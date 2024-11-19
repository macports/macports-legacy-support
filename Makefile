# GNU Makefile for MacportsLegacySupport
# Copyright (c) 2018 Chris Jones <jonesc@macports.org>
# Copyright (c) 2019 Michael Dickens <michaelld@macports.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

DESTDIR         ?= 
PREFIX          ?= /usr/local
INCSUBDIR        = LegacySupport
PKGINCDIR        = $(PREFIX)/include/$(INCSUBDIR)
LIBDIR           = $(PREFIX)/lib
BINDIR           = $(PREFIX)/bin
MANDIR           = $(PREFIX)/share/man
MAN1DIR          = $(MANDIR)/man1
AREXT            = .a
SOEXT            = .dylib
LIBNAME          = MacportsLegacySupport
SYSLIBNAME       = MacportsLegacySystem.B
DLIBFILE         = lib$(LIBNAME)$(SOEXT)
SLIBFILE         = lib$(LIBNAME)$(AREXT)
SYSLIBFILE       = lib$(SYSLIBNAME)$(SOEXT)
DLIBPATH         = $(LIBDIR)/$(DLIBFILE)
SLIBPATH         = $(LIBDIR)/$(SLIBFILE)
SYSLIBPATH       = $(LIBDIR)/$(SYSLIBFILE)
BUILDLIBDIR      = lib
BUILDDLIBPATH    = $(BUILDLIBDIR)/$(DLIBFILE)
BUILDSLIBPATH    = $(BUILDLIBDIR)/$(SLIBFILE)
BUILDSYSLIBPATH  = $(BUILDLIBDIR)/$(SYSLIBFILE)
SOCURVERSION    ?= 1.0
SOCOMPATVERSION ?= 1.0
BUILDDLIBFLAGS   = -dynamiclib -headerpad_max_install_names \
                   -install_name @executable_path/../$(BUILDDLIBPATH) \
                   -current_version $(SOCURVERSION) \
                   -compatibility_version $(SOCOMPATVERSION)
BUILDSYSLIBFLAGS = -dynamiclib -headerpad_max_install_names \
                   -install_name @executable_path/../$(BUILDSYSLIBPATH) \
                   -current_version $(SOCURVERSION) \
                   -compatibility_version $(SOCOMPATVERSION)
OSLIBDIR         = /usr/lib
OSLIBNAME        = System.B
OSLIBLINK        = System
XLIBDIR          = xlib
XLIBPATH         = $(XLIBDIR)/lib$(OSLIBLINK)$(SOEXT)
SYSREEXPORTFLAG  = -Wl,-reexport_library,$(OSLIBDIR)/lib$(OSLIBNAME)$(SOEXT)
BUILDSLIBFLAGS   = -qs
POSTINSTALL      = install_name_tool

# The defaults for C[XX]FLAGS are defined as XC[XX]FLAGS, so that supplied
# definitions of C[XX]FLAGS don't override them.  If overriding these defaults
# is wanted, then override XC[XX]FLAGS.
#
# Note: Overriding CC or CXX with ?= doesn't work, since they're "defined".
FORCE_ARCH      ?=
ARCHFLAGS       ?=
LIPO            ?= lipo
DEBUG           ?=
OPT             ?= -Os
XCFLAGS         ?= $(DEBUG) $(OPT) -Wall -Wno-deprecated-declarations
ALLCFLAGS       := $(ARCHFLAGS) $(XCFLAGS) $(CFLAGS)
DLIBCFLAGS      ?= -fPIC
SLIBCFLAGS      ?=
XCXXFLAGS       ?= $(DEBUG) $(OPT) -Wall
ALLCXXFLAGS     := $(ARCHFLAGS) $(XCXXFLAGS) $(CXXFLAGS)
LD              ?= /usr/bin/ld
LDFLAGS         ?= $(DEBUG)
ALLLDFLAGS      := $(ARCHFLAGS) $(LDFLAGS)
TEST_ARGS       ?=

AR              ?= ar
UNAME           ?= uname
SED             ?= /usr/bin/sed
GREP            ?= /usr/bin/grep
CP              ?= /bin/cp

MKINSTALLDIRS    = install -d -m 755
INSTALL_PROGRAM  = install -c -m 755
INSTALL_DATA     = install -c -m 644
INSTALL_MAN      = install -c -m 444
RM               = rm -f
RMDIR            = sh -c 'for d; do test ! -d "$$d" || rmdir -p "$$d"; done' rmdir

PLATFORM        ?= $(shell $(UNAME) -r | $(SED) -ne 's/\([0-9][0-9]*\)\..*/\1/p')

SRCDIR           = src
SRCINCDIR        = include
# Use VAR := $(shell CMD) instead of VAR != CMD to support old make versions
FIND_LIBHEADERS := find $(SRCINCDIR) -type f \( -name '*.h' -o \
                                             \( -name 'c*' ! -name '*.*' \) \)
LIBHEADERS      := $(shell $(FIND_LIBHEADERS))
ALLHEADERS      := $(LIBHEADERS) $(wildcard $(SRCDIR)/*.h)

MULTISRCS       := $(SRCDIR)/fdopendir.c
ADDSRCS         := $(SRCDIR)/add_symbols.c
LIBSRCS         := $(filter-out $(MULTISRCS) $(ADDSRCS),$(wildcard $(SRCDIR)/*.c))

DLIBOBJEXT       = .dl.o
SLIBOBJEXT       = .o
DLIBOBJS        := $(patsubst %.c,%$(DLIBOBJEXT),$(LIBSRCS))
MULTIDLIBOBJS   := $(patsubst %.c,%$(DLIBOBJEXT),$(MULTISRCS))
ALLDLIBOBJS     := $(DLIBOBJS) $(MULTIDLIBOBJS)
SLIBOBJS        := $(patsubst %.c,%$(SLIBOBJEXT),$(LIBSRCS))
MULTISLIBOBJS   := $(patsubst %.c,%$(SLIBOBJEXT),$(MULTISRCS))
ALLSLIBOBJS     := $(SLIBOBJS) $(MULTISLIBOBJS)
ADDOBJS         := $(patsubst %.c,%$(SLIBOBJEXT),$(ADDSRCS))
ALLSYSLIBOBJS   := $(ALLDLIBOBJS) $(ADDOBJS)

# Defs for filtering out empty object files
#
# Some object files are logically empty, due to their content being
# conditionaled out for the current target.  We provide a mechanism
# for filtering out logically empty object files from the list used
# for linking the static library.  This is done by creating a reference
# empty object file, and excluding any object files with identical contents.
#
# This not only reduces the size of the static library a bit, but also
# avoids the "no symbols" warnings when creating it.
#
# This treatment is only applicable to the static library.
EMPTY            = empty_source_content
EMPTYSOBJ        = $(SRCDIR)/$(EMPTY)$(SLIBOBJEXT)
SOBJLIST         = $(SRCDIR)/slibobjs.tmp

# Automatic tests that don't use the library, and are OK with -fno-builtin
XTESTDIR          = xtest
XTESTNAMEPREFIX   = $(XTESTDIR)/test_
XTESTRUNPREFIX    = run_
XTESTLDFLAGS      = $(ALLLDFLAGS)
XTESTSRCS_C      := $(wildcard $(XTESTNAMEPREFIX)*.c)
XTESTOBJS_C      := $(patsubst %.c,%.o,$(XTESTSRCS_C))
XTESTPRGS_C      := $(patsubst %.c,%,$(XTESTSRCS_C))
XTESTPRGS         = $(XTESTPRGS_C)
XTESTRUNS        := $(patsubst $(XTESTNAMEPREFIX)%,$(XTESTRUNPREFIX)%,$(XTESTPRGS))
DARWINSRCS_C    := $(wildcard $(XTESTNAMEPREFIX)darwin_c*.c)
DARWINRUNS      := $(patsubst \
                     $(XTESTNAMEPREFIX)%.c,$(XTESTRUNPREFIX)%,$(DARWINSRCS_C))
SCANDIRSRCS_C   := $(wildcard $(XTESTNAMEPREFIX)scandir*.c)
SCANDIRRUNS     := $(patsubst \
                     $(XTESTNAMEPREFIX)%.c,$(XTESTRUNPREFIX)%,$(SCANDIRSRCS_C))

# Normal automatic tests
TESTDIR          = test
TESTNAMEPREFIX   = $(TESTDIR)/test_
TESTRUNPREFIX    = run_
TESTLDFLAGS      = -L$(BUILDLIBDIR) $(ALLLDFLAGS)
TESTLIBS         = -l$(LIBNAME)
TESTSYSLDFLAGS   = -L$(XLIBDIR) $(ALLLDFLAGS)
TESTSRCS_C      := $(wildcard $(TESTNAMEPREFIX)*.c)
TESTOBJS_C      := $(patsubst %.c,%.o,$(TESTSRCS_C))
TESTPRGS_C      := $(patsubst %.c,%,$(TESTSRCS_C))
TESTSPRGS_C     := $(patsubst %.c,%_static,$(TESTSRCS_C))
TESTSYSPRGS_C   := $(patsubst %.c,%_syslib,$(TESTSRCS_C))
ALLTESTPRGS     := $(TESTPRGS_C) $(TESTSPRGS_C) $(TESTSYSPRGS_C)
TESTRUNS        := $(patsubst \
                     $(TESTNAMEPREFIX)%,$(TESTRUNPREFIX)%,$(TESTPRGS_C))
TESTSRUNS       := $(patsubst \
                     $(TESTNAMEPREFIX)%,$(TESTRUNPREFIX)%,$(TESTSPRGS_C))
TESTSYSRUNS     := $(patsubst \
                     $(TESTNAMEPREFIX)%,$(TESTRUNPREFIX)%,$(TESTSYSPRGS_C))
REALPATHSRCS_C  := $(wildcard $(TESTNAMEPREFIX)realpath*.c)
REALPATHRUNS    := $(patsubst \
                     $(TESTNAMEPREFIX)%.c,$(TESTRUNPREFIX)%,$(REALPATHSRCS_C))

# Tests that are only run manually
MANTESTDIR       = manual_tests
MANTESTPREFIX    = $(MANTESTDIR)/
MANRUNPREFIX     = mantest_
MANTESTLDFLAGS   = $(ALLLDFLAGS)
MANTESTSRCS_C   := $(wildcard $(MANTESTPREFIX)*.c)
MANTESTSRCS_CPP := $(wildcard $(MANTESTPREFIX)*.cpp)
MANTESTOBJS_C   := $(patsubst %.c,%.o,$(MANTESTSRCS_C))
MANTESTOBJS_CPP := $(patsubst %.cpp,%.o,$(MANTESTSRCS_CPP))
MANTESTPRGS_C   := $(patsubst %.c,%,$(MANTESTSRCS_C))
MANTESTPRGS_CPP := $(patsubst %.cpp,%,$(MANTESTSRCS_CPP))
MANTESTPRGS      = $(MANTESTPRGS_C) $(MANTESTPRGS_CPP)
MANTESTRUNS     := $(patsubst \
                     $(MANTESTPREFIX)%,$(MANRUNPREFIX)%,$(MANTESTPRGS))

TIGERSRCDIR      = tiger_only/src
TIGERSRCS       := $(wildcard $(TIGERSRCDIR)/*.c)
TIGERPRGS       := $(patsubst %.c,%,$(TIGERSRCS))
TIGERMAN1S      := $(wildcard $(TIGERSRCDIR)/*.1)

TOOLDIR          = tools
ARCHTOOL         = $(TOOLDIR)/binarchs.sh

define splitandfilterandmergemultiarch
	output='$(1)' && \
	lipo='$(2)' && \
	rm='$(3)' && \
	cp='$(4)' && \
	ld='$(5)' && \
	grep='$(6)' && \
	platform='$(7)' && \
	force_arch='$(8)' && \
	objectlist="$${output}".* && \
	archlist='' && \
	fatness='' && \
	for object in $${objectlist}; do \
		if [ -z "$${force_arch}" ]; then \
			archlist_new="$$($${lipo} -archs "$${object}")"; \
		else \
			archlist_new="$${force_arch}"; \
		fi && \
		if [ -n "$${archlist}" ] && [ "$${archlist}" != "$${archlist_new}" ]; then \
			printf 'Old/previous architecture list "%s" does not match new one "%s", this is unsupported.\n' "$${archlist}" "$${archlist_new}" >&2 && \
			exit '1'; \
		else \
			archlist="$${archlist_new}"; \
		fi && \
		( $${lipo} -info "$${object}" | grep -qs '^Non-fat file:' ); \
		fatness_new="$${?}" && \
		if [ -n "$${fatness}" ] && [ "$${fatness}" != "$${fatness_new}" ]; then \
			printf 'Old/previous fatness value "%d" does not match new one "%d", this is unsupported.\n' "$${fatness}" "$${fatness_new}" >&2 && \
			exit '2'; \
		else \
			fatness="$${fatness_new}"; \
		fi && \
		if [ -n "$${force_arch}" ] && [ '0' -ne "$${fatness}" ]; then \
			printf 'Architecture forced to "%s", but object file "%s" is a multi-architecture (fat) object file, this is unsupported.\n' "$${force_arch}" "$${object}" >&2 && \
			exit '3'; \
		fi && \
		$$(: 'Check for unknown architectures.') && \
		for arch in $${archlist}; do \
			case "$${arch}" in \
				(unknown*) \
					printf 'Unknown architecture "%s" encountered, this is unsupported.\n' "$${arch}" >&2 && \
					exit '4'; \
					;; \
				(*) \
					;; \
			esac && \
			if [ '0' -eq "$${fatness}" ]; then \
				$${cp} "$${object}" "$${object}.$${arch}" && \
				$$(: 'A non-fat file cannot have more than one architecture, but breaking out sounds weird.'); \
			else \
				$${lipo} "$${object}" -thin "$${arch}" -output "$${object}.$${arch}"; \
			fi; \
		done && \
		$${rm} "$${object}"; \
	done && \
	$$(: '... and use ld to merge each variant into a single-architecture object file ...') && \
	for arch in $${archlist}; do \
		$$(: 'Filter out variants not applicable to certain architectures.') && \
		$$(: 'For instance, the x86_64 architecture is fully UNIX2003-compliant and thus does not have $$UNIX2003-compat functons.') && \
		$$(: 'On the contrary, the i386 architecture has only $$UNIX2003-compat functions for the $$INODE64 feature set.') && \
		$$(: '10.4 is so old that it does not even have the $$INODE64 feature.') && \
		case "$${arch}" in \
			('x86_64'|'ppc64') \
				if [ '9' -gt "$${platform}" ]; then \
					$${ld} -r "$${output}.inode32.$${arch}" -o "$${output}.$${arch}"; \
				else \
					$${ld} -r "$${output}.inode32.$${arch}" "$${output}.inode64.$${arch}" -o "$${output}.$${arch}"; \
				fi; \
				;; \
			('i386'|'ppc'|'ppc7400') \
				if [ '9' -gt "$${platform}" ]; then \
					$${ld} -r "$${output}.inode32.$${arch}" "$${output}.inode32unix2003.$${arch}" -o "$${output}.$${arch}"; \
				else \
					$${ld} -r "$${output}.inode32.$${arch}" "$${output}.inode32unix2003.$${arch}" "$${output}.inode64unix2003.$${arch}" -o "$${output}.$${arch}"; \
				fi; \
				;; \
			(*) \
				$${ld} -r "$${output}.inode32.$${arch}" "$${output}.inode32unix2003.$${arch}" "$${output}.inode64.$${arch}" "$${output}.inode64unix2003.$${arch}" -o "$${output}.$${arch}"; \
				;; \
		esac; \
	done && \
	$$(: '... build list of single-architecture merged object files ...') && \
	objectarchlist='' && \
	for arch in $${archlist}; do \
		objectarchlist="$${objectarchlist} $${output}.$${arch}"; \
	done && \
	if [ '0' -eq "$${fatness}" ]; then \
		$$(: 'Thin files can just be copied directly, assuming that the list will only contain one element.') && \
		$${cp} $${objectarchlist} "$${output}"; \
	else \
		$$(: '... and eventually use lipo to merge them all together!') && \
		$${lipo} $${objectarchlist} -create -output "$${output}"; \
	fi
endef

all: dlib slib syslib
dlib: $(BUILDDLIBPATH)
slib: $(BUILDSLIBPATH)
syslib: $(BUILDSYSLIBPATH)

# Special rules for special implementations.
# For instance, functions using struct stat need to be implemented multiple
# times with different stat structs - a 32-bit-inode based one and a 64-bit-
# inode-based one.
$(MULTIDLIBOBJS): %$(DLIBOBJEXT): %.c $(ALLHEADERS)
	# Generate possibly multi-architecture object files ...
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(DLIBCFLAGS) -D__DARWIN_UNIX03=0 -D__DARWIN_64_BIT_INO_T=0 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode32
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(DLIBCFLAGS) -D__DARWIN_UNIX03=1 -D__DARWIN_ONLY_UNIX_CONFORMANCE=0 -D__DARWIN_64_BIT_INO_T=0 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode32unix2003
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(DLIBCFLAGS) -D__DARWIN_UNIX03=1 -D__DARWIN_ONLY_UNIX_CONFORMANCE=1 -D__DARWIN_64_BIT_INO_T=1 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode64
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(DLIBCFLAGS) -D__DARWIN_UNIX03=1 -D__DARWIN_ONLY_UNIX_CONFORMANCE=0 -D__DARWIN_64_BIT_INO_T=1 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode64unix2003
	# ... and split them up, because ld can only generate single-architecture files ...
	$(call splitandfilterandmergemultiarch,$@,$(LIPO),$(RM),$(CP),$(LD),$(GREP),$(PLATFORM),$(FORCE_ARCH))

$(MULTISLIBOBJS): %$(SLIBOBJEXT): %.c $(ALLHEADERS)
	# Generate possibly multi-architecture object files ...
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) -D__DARWIN_UNIX03=0 -D__DARWIN_64_BIT_INO_T=0 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode32
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) -D__DARWIN_UNIX03=1 -D__DARWIN_ONLY_UNIX_CONFORMANCE=0 -D__DARWIN_64_BIT_INO_T=0 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode32unix2003
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) -D__DARWIN_UNIX03=1 -D__DARWIN_ONLY_UNIX_CONFORMANCE=1 -D__DARWIN_64_BIT_INO_T=1 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode64
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) -D__DARWIN_UNIX03=1 -D__DARWIN_ONLY_UNIX_CONFORMANCE=0 -D__DARWIN_64_BIT_INO_T=1 -D__DARWIN_ONLY_64_BIT_INO_T=0 $< -o $@.inode64unix2003
	# ... and split them up, because ld can only generate single-architecture files ...
	$(call splitandfilterandmergemultiarch,$@,$(LIPO),$(RM),$(CP),$(LD),$(GREP),$(PLATFORM),$(FORCE_ARCH))

# Generously marking all header files as potential dependencies
$(DLIBOBJS): %$(DLIBOBJEXT): %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(DLIBCFLAGS) $< -o $@

$(SLIBOBJS): %$(SLIBOBJEXT): %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) $< -o $@

$(ADDOBJS): %$(SLIBOBJEXT): %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) $< -o $@

dlibobjs: $(ALLDLIBOBJS)

syslibobjs: $(ALLSYSLIBOBJS)

slibobjs: $(ALLSLIBOBJS)

allobjs: dlibobjs slibobjs syslibobjs

# Create a list of nonempty static object files.
$(SOBJLIST): $(ALLSLIBOBJS)
	$(CC) -c $(ALLCFLAGS) $(SLIBCFLAGS) -xc /dev/null -o $(EMPTYSOBJ)
	for f in $^; do cmp -s $(EMPTYSOBJ) $$f || echo $$f; done > $@

# Make the directories separate targets to avoid collisions in parallel builds.
$(BUILDLIBDIR) $(DESTDIR)$(LIBDIR):
	$(MKINSTALLDIRS) $@

$(BUILDDLIBPATH): $(ALLDLIBOBJS) | $(BUILDLIBDIR)
	$(CC) $(BUILDDLIBFLAGS) $(ALLLDFLAGS) $(ALLDLIBOBJS) -o $@

$(BUILDSYSLIBPATH): $(ALLSYSLIBOBJS) | $(BUILDLIBDIR)
	$(CC) $(BUILDSYSLIBFLAGS) $(ALLLDFLAGS) $(SYSREEXPORTFLAG) \
	      $(ALLSYSLIBOBJS) -o $@

$(BUILDSLIBPATH): $(SOBJLIST) | $(BUILDLIBDIR)
	$(RM) $@
	$(AR) $(BUILDSLIBFLAGS) $@ $$(cat $<)

# To run tests with our syslib, we want to suppress linking with the OS syslib,
# just to be certain that our replacement is an adequate substitute.
# But there doesn't seem to be any compiler option that does that correctly
# and without unwanted side effects.  So instead we create a special lib
# directory containing a symlink to our replacement library, but with the
# normal OS syslib name, to fake out the implied '-lSystem'.
$(XLIBPATH): $(BUILDSYSLIBPATH)
	$(MKINSTALLDIRS) $(XLIBDIR)
	cd $(XLIBDIR) && ln -sf ../$< ../$@

$(TESTOBJS_C): %.o: %.c $(ALLHEADERS)
	$(CC) -c -std=c99 -I$(SRCINCDIR) $(ALLCFLAGS) $< -o $@

$(TESTPRGS_C): %: %.o $(BUILDDLIBPATH)
	$(CC) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

# Build tests with *only* our replacement syslib.
$(TESTSYSPRGS_C): %_syslib: %.o $(XLIBPATH)
	$(CC) $(TESTSYSLDFLAGS) $< -o $@

$(TESTSPRGS_C): %_static: %.o $(BUILDSLIBPATH)
	$(CC) $(ALLLDFLAGS) $< $(BUILDSLIBPATH) -o $@

# The "darwin_c" tests need the -fno-builtin option with some compilers.
$(XTESTOBJS_C): %.o: %.c $(ALLHEADERS)
	$(CC) -c -std=c99 -fno-builtin -I$(SRCINCDIR) $(ALLCFLAGS) $< -o $@

# The xtests don't require the library
$(XTESTPRGS_C): %: %.o
	$(CC) $(XTESTLDFLAGS) $< -o $@

# The "darwin_c" tests need the -fno-builtin option with some compilers.
# It shouldn't hurt the other manual tests.
$(MANTESTOBJS_C): %.o: %.c $(ALLHEADERS)
	$(CC) -c -std=c99 -fno-builtin -I$(SRCINCDIR) $(ALLCFLAGS) $< -o $@

# Currently, the manual C tests don't require the library
$(MANTESTPRGS_C): %: %.o
	$(CC) $(MANTESTLDFLAGS) $< -o $@

# But the manual C++ tests *do* require the library
$(MANTESTOBJS_CPP): %.o: %.cpp $(ALLHEADERS)
	$(CXX) -c -I$(SRCINCDIR) $(ALLCXXFLAGS) $< -o $@

$(MANTESTPRGS_CPP): %: %.o $(BUILDDLIBPATH)
	$(CXX) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

$(TIGERPRGS): %: %.c
	$(CC) $$($(ARCHTOOL)) $< -o $@

tiger-bins: $(TIGERPRGS)

# Special clause for testing the cmath fix: Just need to verify that
# building succeeds or fails, not that the executable runs or what it
# produces.  Note that for some reason all Clang compilers tested
# (Apple and MP) successfully compile and link this code regardless of
# the c++ standard chosen, which seems to be a build issue since the
# functions being tested were not introduced until c++11. GCC
# correctly fails the compile and link using c++03 or older, but
# succeeds using c++11 -- as desired.
test_cmath: test/test_cmath.cc $(ALLHEADERS)
	$(info 1: testing compiler '$(CXX)' for non-legacy cmath using c++03; the build should fail, regardless of the compiler or OS)
	$(info 1: $(CXX) $(ALLCXXFLAGS) -std=c++03 $< -o test/$@_cxx03)
	@-$(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++03 $< -o test/$@_cxx03 &> /dev/null && echo "1: c++03 no legacy cmath build success (test failed)!" || echo "1: c++03 no legacy cmath build failure (test succeeded)!"
	$(info 2: testing compiler '$(CXX)' for non-legacy cmath using c++03; the build should fail, regardless of the compiler or OS)
	$(info 2: $(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++03 $< -o test/$@_cxx03)
	@-$(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++03 $< -o test/$@_cxx03 &> /dev/null && echo "2: c++03 legacy cmath build success (test failed)!" || echo "2: c++03 legacy cmath build failure (test succeeded)!"
	$(info 3: testing compiler '$(CXX)' for non-legacy cmath using c++11; if the compiler supports this standard, then the build should succeed regardless of OS)
	$(info 3: $(CXX) $(ALLCXXFLAGS) -std=c++11 $< -o test/$@_cxx11)
	@-$(CXX) $(ALLCXXFLAGS) -std=c++11 $< -o test/$@_cxx11 &> /dev/null && echo "3: c++11 no legacy cmath build success (test failed)!" || echo "3: c++11 no legacy cmath build failure (test succeeded)!"
	$(info 4: testing compiler '$(CXX)' for legacy cmath using c++11; if the compiler supports this standard, then the build should succeed regardless of OS)
	$(info 4: $(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++11 $< -o test/$@_cxx11)
	@-$(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++11 $< -o test/$@_cxx11 &> /dev/null && echo "4: c++11 legacy cmath build success (test succeeded)!" || echo "4: c++11 legacy cmath build failure (test failed)!"

# Special clause for testing faccessat in a setuid program.
# Must be run by root.
# Assumes there is a _uucp user.
# Tests setuid _uucp, setuid root, and setgid tty.
test_faccessat_setuid: test/test_faccessat
	@test/do_test_faccessat_setuid "$(BUILDDLIBPATH)"

test_faccessat_setuid_msg:
	@echo 'Run "sudo make test_faccessat_setuid" to test faccessat properly (Not on 10.4)'

$(TESTRUNS): $(TESTRUNPREFIX)%: $(TESTNAMEPREFIX)%
	$< $(TEST_ARGS)

$(TESTSRUNS): $(TESTRUNPREFIX)%: $(TESTNAMEPREFIX)%
	$< $(TEST_ARGS)

$(TESTSYSRUNS): $(TESTRUNPREFIX)%: $(TESTNAMEPREFIX)%
	$< $(TEST_ARGS)

$(XTESTRUNS): $(XTESTRUNPREFIX)%: $(XTESTNAMEPREFIX)%
	$< $(TEST_ARGS)

# The "dirfuncs_compat" test includes the fdopendir test source
$(TESTNAMEPREFIX)dirfuncs_compat.o: $(TESTNAMEPREFIX)fdopendir.c

# The "forced" tests include the unforced source
$(TESTNAMEPREFIX)stpncpy_chk_forced.o: $(TESTNAMEPREFIX)stpncpy_chk.c
$(TESTNAMEPREFIX)stpncpy_chk_force0.o: $(TESTNAMEPREFIX)stpncpy_chk.c
$(TESTNAMEPREFIX)stpncpy_chk_force1.o: $(TESTNAMEPREFIX)stpncpy_chk.c
$(TESTNAMEPREFIX)strncpy_chk_forced.o: $(TESTNAMEPREFIX)strncpy_chk.c
$(TESTNAMEPREFIX)strncpy_chk_force0.o: $(TESTNAMEPREFIX)strncpy_chk.c
$(TESTNAMEPREFIX)strncpy_chk_force1.o: $(TESTNAMEPREFIX)strncpy_chk.c

# The "darwin_c" tests include the basic "darwin_c" source
$(XTESTNAMEPREFIX)darwin_c_199309.o: $(XTESTNAMEPREFIX)darwin_c.c
$(XTESTNAMEPREFIX)darwin_c_200809.o: $(XTESTNAMEPREFIX)darwin_c.c
$(XTESTNAMEPREFIX)darwin_c_full.o: $(XTESTNAMEPREFIX)darwin_c.c

# The "scandir_*" tests include the basic "scandir" source
$(XTESTNAMEPREFIX)scandir_old.o: $(XTESTNAMEPREFIX)scandir.c
$(XTESTNAMEPREFIX)scandir_ino32.o: $(XTESTNAMEPREFIX)scandir.c
$(XTESTNAMEPREFIX)scandir_ino64.o: $(XTESTNAMEPREFIX)scandir.c

# The nonstandard realpath tests include the realpath source
$(TESTNAMEPREFIX)realpath_nonext.o: $(TESTNAMEPREFIX)realpath.c
$(TESTNAMEPREFIX)realpath_nonposix.o: $(TESTNAMEPREFIX)realpath.c
$(TESTNAMEPREFIX)realpath_compat.o: $(TESTNAMEPREFIX)realpath.c

# Provide a target for all "darwin_c" tests
$(XTESTRUNPREFIX)darwin_c_all: $(DARWINRUNS)

# Provide a target for all "scandir" tests
$(XTESTRUNPREFIX)scandir_all: $(SCANDIRRUNS)

# Provide a target for all "realpath" tests
$(TESTRUNPREFIX)realpath_all: $(REALPATHRUNS)

$(MANTESTRUNS): $(MANRUNPREFIX)%: $(MANTESTPREFIX)%
	$< $(TEST_ARGS)

install: install-headers install-lib

install-headers:
	$(MKINSTALLDIRS) $(patsubst $(SRCINCDIR)/%,$(DESTDIR)$(PKGINCDIR)/%,\
	                            $(sort $(dir $(LIBHEADERS))))
	for h in $(patsubst $(SRCINCDIR)/%,%,$(LIBHEADERS)); do \
	  $(INSTALL_DATA) $(SRCINCDIR)/"$$h" $(DESTDIR)$(PKGINCDIR)/"$$h"; \
	done

install-lib: install-dlib install-slib install-syslib

install-dlib: $(BUILDDLIBPATH) | $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROGRAM) $(BUILDDLIBPATH) $(DESTDIR)$(LIBDIR)
	$(POSTINSTALL) -id $(DLIBPATH) $(DESTDIR)$(DLIBPATH)

install-syslib: $(BUILDSYSLIBPATH) | $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROGRAM) $(BUILDSYSLIBPATH) $(DESTDIR)$(LIBDIR)
	$(POSTINSTALL) -id $(SYSLIBPATH) $(DESTDIR)$(SYSLIBPATH)

install-slib: $(BUILDSLIBPATH) | $(DESTDIR)$(LIBDIR)
	$(INSTALL_DATA) $(BUILDSLIBPATH) $(DESTDIR)$(LIBDIR)

install-tiger: $(TIGERPRGS)
	$(INSTALL_PROGRAM) $(TIGERPRGS) $(DESTDIR)$(BINDIR)
	$(INSTALL_MAN) $(TIGERMAN1S) $(DESTDIR)$(MAN1DIR)

test check: $(TESTRUNS) $(XTESTRUNS) test_cmath test_faccessat_setuid_msg

test_static: $(TESTSRUNS)

test_syslib: $(TESTSYSRUNS)

test_all: test test_static test_syslib

xtest: $(XTESTRUNS)

xtest_clean:
	$(RM) $(XTESTDIR)/*.o $(XTESTPRGS)

$(MANRUNPREFIX)clean:
	$(RM) $(MANTESTDIR)/*.o $(MANTESTPRGS)

test_clean: xtest_clean $(MANRUNPREFIX)clean
	$(RM) $(TESTDIR)/*.o $(ALLTESTPRGS) $(XLIBPATH)
	$(RM) test/test_cmath_* test/test_faccessat_setuid
	@$(RMDIR) $(XLIBDIR)

clean: $(MANRUNPREFIX)clean test_clean
	$(RM) $(foreach D,$(SRCDIR),$D/*.o $D/*.o.* $D/*.d)
	$(RM) $(BUILDDLIBPATH) $(BUILDSLIBPATH) $(BUILDSYSLIBPATH)
	@$(RMDIR) $(BUILDLIBDIR)

.PHONY: all dlib syslib slib clean check test test_cmath xtest
.PHONY: test_static test_syslib test_all
.PHONY: $(TESTRUNS) $(XTESTRUNS) $(MANTESTRUNS)
.PHONY: $(MANRUNPREFIX)clean test_clean xtest_clean
.PHONY: $(XTESTRUNPREFIX)scandir_all
.PHONY: install install-headers install-lib install-dlib install-slib
.PHONY: tiger-bins install-tiger
.PHONY: allobjs dlibobjs slibobjs syslibobjs
