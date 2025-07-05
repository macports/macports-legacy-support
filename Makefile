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
MAN3DIR          = $(MANDIR)/man3
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
POSTINSTALL     ?= /usr/bin/install_name_tool

# The defaults for C[XX]FLAGS are defined as XC[XX]FLAGS, so that supplied
# definitions of C[XX]FLAGS don't override them.  If overriding these defaults
# is wanted, then override XC[XX]FLAGS.
#
# Note: Overriding CC or CXX with ?= doesn't work, since they're "defined".
ARCHS           ?=
ARCHFLAGS       ?= $(patsubst %,-arch %,$(ARCHS))
DEBUG           ?=
OPT             ?= -Os
XCFLAGS         ?= $(DEBUG) $(OPT) -Wall -Wno-deprecated-declarations -Wundef
ALLCFLAGS       := $(ARCHFLAGS) $(XCFLAGS) $(CFLAGS)
TOOLCFLAGS      ?= $(ARCHFLAGS) $(DEBUG) $(OPT) $(CFLAGS)
DLIBCFLAGS      ?= -fPIC
SLIBCFLAGS      ?=
XCXXFLAGS       ?= $(DEBUG) $(OPT) -Wall
ALLCXXFLAGS     := $(ARCHFLAGS) $(XCXXFLAGS) $(CXXFLAGS)
XLDFLAGS        ?= $(DEBUG)
ALLLDFLAGS      := $(ARCHFLAGS) $(XLDFLAGS) $(LDFLAGS)
TEST_ARGS       ?=

ARX             ?= /usr/bin/ar
UNAME           ?= uname
SED             ?= /usr/bin/sed
GREP            ?= /usr/bin/grep
CP              ?= /bin/cp

# Directory for temporary test files
TEST_TEMP       ?= tst_data
TESTCFLAGS       = -Wshadow $(ALLCFLAGS) '-DTEST_TEMP="$(TEST_TEMP)"'

MKINSTALLDIRS    = install -d -m 755
INSTALL_PROGRAM  = install -c -m 755
INSTALL_DATA     = install -c -m 644
INSTALL_MAN      = install -c -m 444
RM               = rm -f
RMDIR            = rm -rf

SRCDIR           = src
SRCINCDIR        = include
BUILDDIR         = bin
TESTBINDIR       = tbin
# Use VAR := $(shell CMD) instead of VAR != CMD to support old make versions
FIND_LIBHEADERS := find $(SRCINCDIR) -type f \( -name '*.h' -o \
                                             \( -name 'c*' ! -name '*.*' \) \)
LIBHEADERS      := $(shell $(FIND_LIBHEADERS))
ALLHEADERS      := $(LIBHEADERS) $(wildcard $(SRCDIR)/*.h)

ALLLIBSRCS      := $(patsubst $(SRCDIR)/%.c,%,$(wildcard $(SRCDIR)/*.c))
ADDSRCS         := add_symbols
LIBSRCS         := $(filter-out $(ADDSRCS),$(ALLLIBSRCS))

DLIBOBJEXT       = .dl.o
SLIBOBJEXT       = .o
DLIBOBJS        := $(patsubst %,$(BUILDDIR)/%$(DLIBOBJEXT),$(LIBSRCS))
SLIBOBJS        := $(patsubst %,$(BUILDDIR)/%$(SLIBOBJEXT),$(LIBSRCS))
ADDOBJS         := $(patsubst %,$(BUILDDIR)/%$(SLIBOBJEXT),$(ADDSRCS))
SYSLIBOBJS      := $(DLIBOBJS) $(ADDOBJS)

# Man pages
SRCMAN3S        := $(wildcard $(SRCDIR)/*.3)

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
# A complication is that a completely empty static library is illegal,
# so we provide a dummy object to be used when the library is logically
# empty.
#
# This treatment is only applicable to the static library.
EMPTY            = empty_source_content
EMPTYSOBJ        = $(BUILDDIR)/$(EMPTY)$(SLIBOBJEXT)
SOBJLIST         = $(BUILDDIR)/slibobjs.tmp
DUMMYSRC         = $(SRCDIR)/dummylib.xxc
DUMMYOBJ         = $(BUILDDIR)/dummylib.o

# Automatic tests that don't use the library, and are OK with -fno-builtin
XTESTDIR          = xtest
XTESTNAMEPREFIX   = $(XTESTDIR)/test_
XTESTBINPREFIX    = $(TESTBINDIR)/test_
XTESTRUNPREFIX    = run_
XTESTLDFLAGS      = $(ALLLDFLAGS)
XTESTSRCS_C      := $(wildcard $(XTESTNAMEPREFIX)*.c)
XTESTPRGS_C      := $(patsubst $(XTESTDIR)/%.c,%,$(XTESTSRCS_C))
XTESTOBJS_C      := $(patsubst %,$(TESTBINDIR)/%.o,$(XTESTPRGS_C))
XTESTPRGS         = $(patsubst %,$(TESTBINDIR)/%,$(XTESTPRGS_C))
XTESTRUNS        := \
    $(patsubst $(XTESTBINPREFIX)%,$(XTESTRUNPREFIX)%,$(XTESTPRGS))
DARWINSRCS_C    := $(wildcard $(XTESTNAMEPREFIX)darwin_c*.c)
DARWINRUNS      := $(patsubst \
                     $(XTESTNAMEPREFIX)%.c,$(XTESTRUNPREFIX)%,$(DARWINSRCS_C))
SCANDIRSRCS_C   := $(wildcard $(XTESTNAMEPREFIX)scandir*.c)
SCANDIRRUNS     := $(patsubst \
                     $(XTESTNAMEPREFIX)%.c,$(XTESTRUNPREFIX)%,$(SCANDIRSRCS_C))
ALLHDRSRCS_C    := $(wildcard $(XTESTNAMEPREFIX)allheaders*.c)
ALLHDRRUNS      := $(patsubst \
                     $(XTESTNAMEPREFIX)%.c,$(XTESTRUNPREFIX)%,$(ALLHDRSRCS_C)) \
                     $(XTESTRUNPREFIX)revheaders

# Normal automatic tests
TESTDIR          = test
TESTNAMEPREFIX   = $(TESTDIR)/test_
TESTBINPREFIX    = $(TESTBINDIR)/test_
TESTRUNPREFIX    = run_
TESTLDFLAGS      = -L$(BUILDLIBDIR) $(ALLLDFLAGS)
TESTLIBS         = -l$(LIBNAME)
TESTSYSLDFLAGS   = -L$(XLIBDIR) $(ALLLDFLAGS)
TESTSRCS_C      := $(wildcard $(TESTNAMEPREFIX)*.c)
TESTPRGS_C      := $(patsubst $(TESTDIR)/%.c,%,$(TESTSRCS_C))
TESTOBJS_C      := $(patsubst %,$(TESTBINDIR)/%.o,$(TESTPRGS_C))
TESTPRGS        := $(patsubst %,$(TESTBINDIR)/%,$(TESTPRGS_C))
TESTSPRGS       := $(patsubst %,%_static,$(TESTPRGS))
TESTSYSPRGS     := $(patsubst %,%_syslib,$(TESTPRGS))
ALLTESTPRGS     := $(TESTPRGS) $(TESTSPRGS) $(TESTSYSPRGS)
TESTRUNS        := $(patsubst \
                     $(TESTBINPREFIX)%,$(TESTRUNPREFIX)%,$(TESTPRGS))
TESTSRUNS       := $(patsubst \
                     $(TESTBINPREFIX)%,$(TESTRUNPREFIX)%,$(TESTSPRGS))
TESTSYSRUNS     := $(patsubst \
                     $(TESTBINPREFIX)%,$(TESTRUNPREFIX)%,$(TESTSYSPRGS))
REALPATHSRCS_C  := $(wildcard $(TESTNAMEPREFIX)realpath*.c)
REALPATHRUNS    := $(patsubst \
                     $(TESTNAMEPREFIX)%.c,$(TESTRUNPREFIX)%,$(REALPATHSRCS_C))
FDOPENDIRSRCS_C := $(wildcard $(TESTNAMEPREFIX)fdopendir*.c)
FDOPENDIRRUNS   := $(patsubst \
                     $(TESTNAMEPREFIX)%.c,$(TESTRUNPREFIX)%,$(FDOPENDIRSRCS_C))
STATXXSRCS_C    := $(wildcard $(TESTNAMEPREFIX)stat*.c)
STATXXRUNS      := $(patsubst \
                     $(TESTNAMEPREFIX)%.c,$(TESTRUNPREFIX)%,$(STATXXSRCS_C))
PACKETSRCS_C    := $(wildcard $(TESTNAMEPREFIX)packet*.c)
PACKETRUNS      := $(patsubst \
                     $(TESTNAMEPREFIX)%.c,$(TESTRUNPREFIX)%,$(PACKETSRCS_C))
STPNCHKSRCS_C    := $(wildcard $(TESTNAMEPREFIX)stpncpy_chk*.c)
STPNCHKRUNS      := $(patsubst \
                     $(TESTNAMEPREFIX)%.c,$(TESTRUNPREFIX)%,$(STPNCHKSRCS_C))

# Tests that are only run manually
MANTESTDIR       = manual_tests
MANTESTPREFIX    = $(MANTESTDIR)/
MANLIBTESTPFX    = $(MANTESTDIR)/libtest_
MANTESTBINPREFIX = $(TESTBINDIR)/
MANRUNPREFIX     = mantest_
MANTESTLDFLAGS   = $(ALLLDFLAGS)
MANALLTESTSRCS_C := $(wildcard $(MANTESTPREFIX)*.c)
MANLIBTESTSRCS_C := $(wildcard $(MANLIBTESTPFX)*.c)
MANTESTSRCS_C    := $(filter-out $(MANLIBTESTSRCS_C),$(MANALLTESTSRCS_C))
MANTESTSRCS_CPP  := $(wildcard $(MANTESTPREFIX)*.cpp)
MANTESTPRGS_C    := $(patsubst $(MANTESTPREFIX)%.c,%,$(MANTESTSRCS_C))
MANLIBTESTPRGS_C := $(patsubst $(MANLIBTESTPFX)%.c,%,$(MANLIBTESTSRCS_C))
MANTESTPRGS_CPP  := $(patsubst $(MANTESTPREFIX)%.cpp,%,$(MANTESTSRCS_CPP))
MANTESTOBJS_C    := $(patsubst %,$(TESTBINDIR)/%.o,$(MANTESTPRGS_C))
MANLIBTESTOBJS_C := $(patsubst %,$(TESTBINDIR)/%.o,$(MANLIBTESTPRGS_C))
MANTESTOBJS_CPP  := $(patsubst %,$(TESTBINDIR)/%.o,$(MANTESTPRGS_CPP))
MANTESTOBJS      := $(MANTESTOBJS_C) $(MANLIBTESTOBJS_C) $(MANTESTOBJS_CPP)
MANTESTBINS_C    := $(patsubst %,$(TESTBINDIR)/%,$(MANTESTPRGS_C))
MANTESTBINS_CPP  := $(patsubst %,$(TESTBINDIR)/%,$(MANTESTPRGS_CPP))
MANTESTPRGS      := $(MANTESTBINS_C) $(MANTESTBINS_CPP)
MANTESTRUNS      := $(patsubst \
                     $(TESTBINDIR)/%,$(MANRUNPREFIX)%,$(MANTESTPRGS))
MANLIBTESTBINS   := $(patsubst %,$(TESTBINDIR)/%,$(MANLIBTESTPRGS_C))
MANLIBTESTRUNS  := $(patsubst \
                     $(TESTBINDIR)/%,$(MANRUNPREFIX)%,$(MANLIBTESTBINS))
STRNCHKSRCS_C    := $(wildcard $(MANLIBTESTPFX)strncpy_chk*.c)
STRNCHKRUNS      := $(patsubst \
                     $(MANLIBTESTPFX)%.c,$(MANRUNPREFIX)%,$(STRNCHKSRCS_C))
MANPACKETSRCS_C  := $(wildcard $(MANLIBTESTPFX)packet*.c)
MANPACKETRUNS    := $(patsubst \
                     $(MANLIBTESTPFX)%.c,$(MANRUNPREFIX)%,$(MANPACKETSRCS_C))

# C standard for tests
TESTCSTD         := c99

TIGERROOT        = tiger_only
TIGERSRCDIR      = $(TIGERROOT)/src
TIGERBINDIR      = $(TIGERROOT)/bin
TIGERSRCS       := \
    $(patsubst $(TIGERSRCDIR)/%.c,%,$(wildcard $(TIGERSRCDIR)/*.c))
TIGERPRGS       := $(patsubst %,$(TIGERBINDIR)/%,$(TIGERSRCS))
TIGERMAN1S      := $(wildcard $(TIGERSRCDIR)/*.1)

# Miscellaneous tools
TOOLPREFIX       = tool_
TOOLDIR          = tools
TOOLBINDIR       = tlbin
TOOLSRCS_C      := $(wildcard $(TOOLDIR)/*.c)
TOOLBINS        := $(patsubst $(TOOLDIR)/%.c,$(TOOLBINDIR)/%,$(TOOLSRCS_C))
TOOLTARGS       := $(patsubst $(TOOLBINDIR)/%, $(TOOLPREFIX)%, $(TOOLBINS))
TOOL_ARGS       ?=

ARCHTOOL         = $(TOOLDIR)/binarchs.sh

all: dlib slib syslib
dlib: $(BUILDDLIBPATH)
slib: $(BUILDSLIBPATH)
syslib: $(BUILDSYSLIBPATH)

# Generously marking all header files as potential dependencies
$(DLIBOBJS): $(BUILDDIR)/%$(DLIBOBJEXT): $(SRCDIR)/%.c $(ALLHEADERS) \
    | $(BUILDDIR)
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(DLIBCFLAGS) $< -o $@

$(SLIBOBJS): $(BUILDDIR)/%$(SLIBOBJEXT): $(SRCDIR)/%.c $(ALLHEADERS) \
    | $(BUILDDIR)
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) $< -o $@

$(ADDOBJS): $(BUILDDIR)/%$(SLIBOBJEXT): $(SRCDIR)/%.c $(ALLHEADERS) \
    | $(BUILDDIR)
	$(CC) -c -I$(SRCINCDIR) $(ALLCFLAGS) $(SLIBCFLAGS) $< -o $@

dlibobjs: $(DLIBOBJS)

syslibobjs: $(SYSLIBOBJS)

slibobjs: $(SLIBOBJS)

allobjs: dlibobjs slibobjs syslibobjs

# Rule to make assembler source for inspection (not used in normal builds)
%.S: %.c
	$(CC) -S -I$(SRCINCDIR) $(ALLCFLAGS) $< -o $@

# Create a list of nonempty static object files.
# Since completely empty archives are illegal, we use our dummy if there
# would otherwise be no objects.
$(SOBJLIST): $(SLIBOBJS)
	$(CC) -c $(ALLCFLAGS) $(SLIBCFLAGS) -xc /dev/null -o $(EMPTYSOBJ)
	$(CC) -c $(ALLCFLAGS) $(SLIBCFLAGS) -xc $(DUMMYSRC) -o $(DUMMYOBJ)
	for f in $^; do cmp -s $(EMPTYSOBJ) $$f || echo $$f; done > $@
	if [ ! -s $@ ]; then echo $(DUMMYOBJ) > $@; fi

# Make the directories separate targets to avoid collisions in parallel builds.
$(BUILDDIR) $(TIGERBINDIR) $(BUILDLIBDIR) $(TESTBINDIR) \
    $(DESTDIR)$(LIBDIR) $(DESTDIR)$(BINDIR) \
    $(DESTDIR)$(MAN1DIR) $(DESTDIR)$(MAN3DIR) \
    $(TEST_TEMP) $(TOOLBINDIR):
	$(MKINSTALLDIRS) $@

$(BUILDDLIBPATH): $(DLIBOBJS) | $(BUILDLIBDIR)
	$(CC) $(BUILDDLIBFLAGS) $(ALLLDFLAGS) $(DLIBOBJS) -o $@

$(BUILDSYSLIBPATH): $(SYSLIBOBJS) | $(BUILDLIBDIR)
	$(CC) $(BUILDSYSLIBFLAGS) $(ALLLDFLAGS) $(SYSREEXPORTFLAG) \
	      $(SYSLIBOBJS) -o $@

$(BUILDSLIBPATH): $(SOBJLIST) | $(BUILDLIBDIR)
	$(RM) $@
	$(ARX) $(BUILDSLIBFLAGS) $@ $$(cat $<)

# To run tests with our syslib, we want to suppress linking with the OS syslib,
# just to be certain that our replacement is an adequate substitute.
# But there doesn't seem to be any compiler option that does that correctly
# and without unwanted side effects.  So instead we create a special lib
# directory containing a symlink to our replacement library, but with the
# normal OS syslib name, to fake out the implied '-lSystem'.
$(XLIBPATH): $(BUILDSYSLIBPATH)
	$(MKINSTALLDIRS) $(XLIBDIR)
	cd $(XLIBDIR) && ln -sf ../$< ../$@

$(TESTOBJS_C): $(TESTBINDIR)/%.o: $(TESTDIR)/%.c $(ALLHEADERS) | $(TESTBINDIR)
	$(CC) -c -std=$(TESTCSTD) -I$(SRCINCDIR) $(TESTCFLAGS) $< -o $@

$(MANTESTOBJS_C): \
    $(TESTBINDIR)/%.o: $(MANTESTDIR)/%.c $(ALLHEADERS) | $(TESTBINDIR)
	$(CC) -c -std=$(TESTCSTD) -I$(SRCINCDIR) $(TESTCFLAGS) $< -o $@

$(MANLIBTESTOBJS_C): \
    $(TESTBINDIR)/%.o: $(MANLIBTESTPFX)%.c $(ALLHEADERS) | $(TESTBINDIR)
	$(CC) -c -std=$(TESTCSTD) -I$(SRCINCDIR) $(TESTCFLAGS) $< -o $@

$(MANTESTOBJS_CPP): $(TESTBINDIR)/%.o: \
    $(MANTESTDIR)/%.cpp $(ALLHEADERS) | $(TESTBINDIR)
	$(CXX) -c -I$(SRCINCDIR) $(ALLCXXFLAGS) $< -o $@

$(TESTPRGS): %: %.o $(BUILDDLIBPATH)
	$(CC) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

# Build tests with *only* our replacement syslib.
$(TESTSYSPRGS): %_syslib: %.o $(XLIBPATH)
	$(CC) $(TESTSYSLDFLAGS) $< -o $@

$(TESTSPRGS): %_static: %.o $(BUILDSLIBPATH)
	$(CC) $(ALLLDFLAGS) $< $(BUILDSLIBPATH) -o $@

# The "darwin_c" tests need the -fno-builtin option with some compilers.
$(XTESTOBJS_C): $(TESTBINDIR)/%.o: $(XTESTDIR)/%.c $(ALLHEADERS) | $(TESTBINDIR)
	$(CC) -c -std=$(TESTCSTD) -fno-builtin -I$(SRCINCDIR) $(TESTCFLAGS) $< -o $@

# The xtests don't require the library
$(XTESTPRGS): %: %.o
	$(CC) $(XTESTLDFLAGS) $< -o $@

# Currently, the manual C tests don't require the library
$(MANTESTBINS): %: %.o
	$(CC) $(MANTESTLDFLAGS) $< -o $@

# Except for the ones that do
$(MANLIBTESTBINS): %: %.o $(BUILDDLIBPATH)
	$(CC) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

# And the manual C++ tests *do* require the library
$(MANTESTBINS_CPP): %: %.o $(BUILDDLIBPATH)
	$(CXX) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

alltestobjs: $(TESTOBJS_C) $(XTESTOBJS_C) $(MANTESTOBJS_C) $(MANLIBTESTOBJS_C)

$(TIGERPRGS): $(TIGERBINDIR)/%: $(TIGERSRCDIR)/%.c | $(TIGERBINDIR)
	$(CC) $$($(ARCHTOOL)) $< -o $@

tiger-bins: $(TIGERPRGS)

# Dummy Leopard build target, so the Portfile can reference it in case
# we need it later.
leopard-bins:

# Special clause for testing the cmath fix: Just need to verify that
# building succeeds or fails, not that the executable runs or what it
# produces.  Note that for some reason all Clang compilers tested
# (Apple and MP) successfully compile and link this code regardless of
# the c++ standard chosen, which seems to be a build issue since the
# functions being tested were not introduced until c++11. GCC
# correctly fails the compile and link using c++03 or older, but
# succeeds using c++11 -- as desired.
test_cmath: test/test_cmath.cc $(ALLHEADERS) | $(TESTBINDIR)
	$(info 1: testing compiler '$(CXX)' for non-legacy cmath using c++03; the build should fail, regardless of the compiler or OS)
	$(info 1: $(CXX) $(ALLCXXFLAGS) -std=c++03 $< -o $(TESTBINDIR)/$@_cxx03)
	@-$(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++03 $< -o $(TESTBINDIR)/$@_cxx03 &> /dev/null && echo "1: c++03 no legacy cmath build success (test failed)!" || echo "1: c++03 no legacy cmath build failure (test succeeded)!"
	$(info 2: testing compiler '$(CXX)' for non-legacy cmath using c++03; the build should fail, regardless of the compiler or OS)
	$(info 2: $(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++03 $< -o $(TESTBINDIR)/$@_cxx03)
	@-$(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++03 $< -o $(TESTBINDIR)/$@_cxx03 &> /dev/null && echo "2: c++03 legacy cmath build success (test failed)!" || echo "2: c++03 legacy cmath build failure (test succeeded)!"
	$(info 3: testing compiler '$(CXX)' for non-legacy cmath using c++11; if the compiler supports this standard, then the build should succeed regardless of OS)
	$(info 3: $(CXX) $(ALLCXXFLAGS) -std=c++11 $< -o $(TESTBINDIR)/$@_cxx11)
	@-$(CXX) $(ALLCXXFLAGS) -std=c++11 $< -o $(TESTBINDIR)/$@_cxx11 &> /dev/null && echo "3: c++11 no legacy cmath build success (test failed)!" || echo "3: c++11 no legacy cmath build failure (test succeeded)!"
	$(info 4: testing compiler '$(CXX)' for legacy cmath using c++11; if the compiler supports this standard, then the build should succeed regardless of OS)
	$(info 4: $(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++11 $< -o $(TESTBINDIR)/$@_cxx11)
	@-$(CXX) -I$(SRCINCDIR) $(ALLCXXFLAGS) -std=c++11 $< -o $(TESTBINDIR)/$@_cxx11 &> /dev/null && echo "4: c++11 legacy cmath build success (test succeeded)!" || echo "4: c++11 legacy cmath build failure (test failed)!"

# Special clause for testing faccessat in a setuid program.
# Must be run by root.
# Assumes there is a _uucp user.
# Tests setuid _uucp, setuid root, and setgid tty.
mantest_faccessat_setuid: $(TESTBINDIR)/test_faccessat_static | $(TEST_TEMP)
	@manual_tests/do_test_faccessat_setuid.sh

$(TESTRUNS): $(TESTRUNPREFIX)%: $(TESTBINPREFIX)% | $(TEST_TEMP)
	$< $(TEST_ARGS)

$(TESTSRUNS): $(TESTRUNPREFIX)%: $(TESTBINPREFIX)% | $(TEST_TEMP)
	$< $(TEST_ARGS)

$(TESTSYSRUNS): $(TESTRUNPREFIX)%: $(TESTBINPREFIX)% | $(TEST_TEMP)
	$< $(TEST_ARGS)

$(XTESTRUNS): $(XTESTRUNPREFIX)%: $(XTESTBINPREFIX)% | $(TEST_TEMP)
	$< $(TEST_ARGS)

$(MANTESTRUNS) $(MANLIBTESTRUNS): \
    $(MANRUNPREFIX)%: $(MANTESTBINPREFIX)% | $(TEST_TEMP)
	$< $(TEST_ARGS)

# Rule for tools - no include or library
$(TOOLBINS): $(TOOLBINDIR)/%: $(TOOLDIR)/%.c | $(TOOLBINDIR)
	$(CC) -std=$(TESTCSTD) $(TOOLCFLAGS) $< -o $@

$(TOOLTARGS): $(TOOLPREFIX)%: $(TOOLBINDIR)/%
	$< $(TOOL_ARGS)

# The "dirfuncs_compat" test includes the fdopendir test source
$(TESTNAMEPREFIX)dirfuncs_compat.o: $(TESTNAMEPREFIX)fdopendir.c

# The "forced" tests include the unforced source
$(TESTBINPREFIX)stpncpy_chk_forced.o: $(TESTNAMEPREFIX)stpncpy_chk.c
$(TESTBINPREFIX)stpncpy_chk_force0.o: $(TESTNAMEPREFIX)stpncpy_chk.c
$(TESTBINPREFIX)stpncpy_chk_force1.o: $(TESTNAMEPREFIX)stpncpy_chk.c
$(MANTESTBINPREFIX)strncpy_chk_forced.o: $(MANLIBTESTPFX)strncpy_chk.c
$(MANTESTBINPREFIX)strncpy_chk_force0.o: $(MANLIBTESTPFX)strncpy_chk.c
$(MANTESTBINPREFIX)strncpy_chk_force1.o: $(MANLIBTESTPFX)strncpy_chk.c

# The "darwin_c" tests include the basic "darwin_c" source
$(XTESTBINPREFIX)darwin_c_199309.o: $(XTESTNAMEPREFIX)darwin_c.c
$(XTESTBINPREFIX)darwin_c_200809.o: $(XTESTNAMEPREFIX)darwin_c.c
$(XTESTBINPREFIX)darwin_c_full.o: $(XTESTNAMEPREFIX)darwin_c.c

# The "scandir_*" tests include the basic "scandir" source
$(XTESTBINPREFIX)scandir_old.o: $(XTESTNAMEPREFIX)scandir.c
$(XTESTBINPREFIX)scandir_ino32.o: $(XTESTNAMEPREFIX)scandir.c
$(XTESTBINPREFIX)scandir_ino64.o: $(XTESTNAMEPREFIX)scandir.c

# The nonstandard realpath tests include the realpath source
$(TESTBINPREFIX)realpath_nonext.o: $(TESTNAMEPREFIX)realpath.c
$(TESTBINPREFIX)realpath_nonposix.o: $(TESTNAMEPREFIX)realpath.c
$(TESTBINPREFIX)realpath_compat.o: $(TESTNAMEPREFIX)realpath.c

# The fdopendir_ino?? tests include the fdopendir source
$(TESTBINPREFIX)fdopendir_ino32.o: $(TESTNAMEPREFIX)fdopendir.c
$(TESTBINPREFIX)fdopendir_ino64.o: $(TESTNAMEPREFIX)fdopendir.c

# The stat_ino?? tests include the stat source
$(TESTBINPREFIX)stat_darwin.o: $(TESTNAMEPREFIX)stat.c
$(TESTBINPREFIX)stat_ino32.o: $(TESTNAMEPREFIX)stat.c
$(TESTBINPREFIX)stat_ino64.o: $(TESTNAMEPREFIX)stat.c
$(TESTBINPREFIX)stat_ino64_darwin.o: $(TESTNAMEPREFIX)stat.c

# The packet_* tests include the packet source
$(TESTBINPREFIX)packet_nocancel.o: $(TESTNAMEPREFIX)packet.c
$(TESTBINPREFIX)packet_nonposix.o: $(TESTNAMEPREFIX)packet.c

# The manual packet tests include the packet source
$(MANTESTBINPREFIX)packet_cont.o: $(TESTNAMEPREFIX)packet.c
$(MANTESTBINPREFIX)packet_nofix.o: $(TESTNAMEPREFIX)packet.c
$(MANTESTBINPREFIX)packet_nofix_nocancel.o: $(TESTNAMEPREFIX)packet.c
$(MANTESTBINPREFIX)packet_nofix_nonposix.o: $(TESTNAMEPREFIX)packet.c

# The "allheaders" tests include the basic "allheaders" source
$(XTESTBINPREFIX)allheaders_199309.o: $(XTESTNAMEPREFIX)allheaders.c
$(XTESTBINPREFIX)allheaders_200809.o: $(XTESTNAMEPREFIX)allheaders.c
$(XTESTBINPREFIX)allheaders_full.o: $(XTESTNAMEPREFIX)allheaders.c
$(XTESTBINPREFIX)allheaders_199309_ds.o: $(XTESTNAMEPREFIX)allheaders.c
$(XTESTBINPREFIX)allheaders_200809_ds.o: $(XTESTNAMEPREFIX)allheaders.c
$(XTESTBINPREFIX)allheaders_full_ds.o: $(XTESTNAMEPREFIX)allheaders.c

# Provide a target for all "darwin_c" tests
$(XTESTRUNPREFIX)darwin_c_all: $(DARWINRUNS)

# Provide a target for all "scandir" tests
$(XTESTRUNPREFIX)scandir_all: $(SCANDIRRUNS)

# Provide a target for all "realpath" tests
$(TESTRUNPREFIX)realpath_all: $(REALPATHRUNS)

# Provide a target for all "fdopendir" tests
$(TESTRUNPREFIX)fdopendir_all: $(FDOPENDIRRUNS)

# Provide a target for all "stat" tests
$(TESTRUNPREFIX)stat_all: $(STATXXRUNS)

# Provide a target for all "stpncpy_chk" tests
$(TESTRUNPREFIX)stpncpy_chk_all: $(STPNCHKRUNS)

# Provide a target for all "strncpy_chk" tests
$(MANRUNPREFIX)strncpy_chk_all: $(STRNCHKRUNS)

# Provide a target for all non-manual "packet" tests
$(TESTRUNPREFIX)packet_all: $(PACKETRUNS)

# Provide a target for all manual "packet" tests
$(MANRUNPREFIX)packet_all: $(MANPACKETRUNS)

# Provide a target for all "allheaders" tests
$(XTESTRUNPREFIX)allheaders_all: $(ALLHDRRUNS)

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

# We need a better way to handle OS-dependent manpage installs.
# Currently, the only cases are the Tiger-specific which.1,
# and copyfile.3 for Tiger and Leopard, so we add the Tiger
# case to the existing Tiger install, and add a new Leopard install.

install-tiger: $(TIGERPRGS) | $(DESTDIR)$(BINDIR) \
    $(DESTDIR)$(MAN1DIR) $(DESTDIR)$(MAN3DIR)
	$(INSTALL_PROGRAM) $(TIGERPRGS) $(DESTDIR)$(BINDIR)
	$(INSTALL_MAN) $(TIGERMAN1S) $(DESTDIR)$(MAN1DIR)
	$(INSTALL_MAN) $(SRCMAN3S) $(DESTDIR)$(MAN3DIR)

install-leopard: | $(DESTDIR)$(MAN3DIR)
	$(INSTALL_MAN) $(SRCMAN3S) $(DESTDIR)$(MAN3DIR)

test check: $(TESTRUNS) $(XTESTRUNS) test_cmath

test_static: $(TESTSRUNS)

test_syslib: $(TESTSYSRUNS)

test_all: test test_static test_syslib

xtest: $(XTESTRUNS)

# Targets to build tests without running them

build_tests: $(TESTPRGS) $(XTESTPRGS)

build_tests_static: $(TESTSPRGS)

build_tests_syslib: $(TESTSYSPRGS)

build_tests_all: build_tests build_tests_static build_tests_syslib

# Dummy target for placeholder in scripts
dummy:

xtest_clean:
	$(RM) $(XTESTOBJS_C) $(XTESTPRGS)

$(MANRUNPREFIX)clean:
	$(RM) $(MANTESTOBJS) $(MANTESTPRGS) $(MANLIBTESTBINS)

test_clean:
	$(RMDIR) $(TESTBINDIR) $(XLIBDIR) $(TEST_TEMP)

tools_clean:
	$(RMDIR) $(TOOLBINDIR)

clean: test_clean tools_clean
	$(RMDIR) $(BUILDDIR) $(BUILDLIBDIR) $(TIGERBINDIR)

.PHONY: all dlib syslib slib clean check test test_cmath xtest
.PHONY: test_static test_syslib test_all
.PHONY: $(TESTRUNS) $(XTESTRUNS) $(MANTESTRUNS)
.PHONY: $(MANRUNPREFIX)clean test_clean xtest_clean
.PHONY: $(XTESTRUNPREFIX)darwin_c_all
.PHONY: $(XTESTRUNPREFIX)scandir_all
.PHONY: $(TESTRUNPREFIX)realpath_all
.PHONY: $(TESTRUNPREFIX)fdopendir_all
.PHONY: $(TESTRUNPREFIX)stat_all
.PHONY: $(MANRUNPREFIX)stpncpy_chk_all
.PHONY: $(MANRUNPREFIX)strncpy_chk_all
.PHONY: $(TESTRUNPREFIX)packet_all
.PHONY: $(MANRUNPREFIX)packet_all
.PHONY: $(XTESTRUNPREFIX)allheaders_all
.PHONY: install install-headers install-lib install-dlib install-slib
.PHONY: tiger-bins install-tiger
.PHONY: leopard-bins install-leopard
.PHONY: allobjs dlibobjs slibobjs syslibobjs alltestobjs
.PHONY: build_tests build_tests_static build_tests_syslib build_tests_all dummy
