# GNU Makefile for MacportsLegacySupport
# Copyright (c) 2010 Chris Jones <jonesc@macports.org>
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
AREXT            = .a
SOEXT            = .dylib
LIBNAME          = MacportsLegacySupport
DLIBFILE         = lib$(LIBNAME)$(SOEXT)
SLIBFILE         = lib$(LIBNAME)$(AREXT)
DLIBPATH         = $(LIBDIR)/$(DLIBFILE)
SLIBPATH         = $(LIBDIR)/$(SLIBFILE)
BUILDDLIBDIR     = lib
BUILDSLIBDIR     = lib
BUILDDLIBPATH    = $(BUILDDLIBDIR)/$(DLIBFILE)
BUILDSLIBPATH    = $(BUILDSLIBDIR)/$(SLIBFILE)
BUILDDLIBFLAGS   = -dynamiclib -headerpad_max_install_names \
                   -install_name @executable_path/../$(BUILDDLIBPATH) \
                   -current_version 1.0 -compatibility_version 1.0
BUILDSLIBFLAGS   = -qs
POSTINSTALL      = install_name_tool
POSTINSTALLFLAGS = -id $(DLIBPATH)

ARCHFLAGS       ?=
CC              ?= cc $(ARCHFLAGS)
CFLAGS          ?= -Os -Wall
DLIBCFLAGS      ?= -fPIC
SLIBCFLAGS      ?=
CXX             ?= c++ $(ARCHFLAGS)
CXXFLAGS        ?= -Os -Wall
LDFLAGS         ?=
AR              ?= ar

MKINSTALLDIRS    = install -d -m 755
INSTALL_PROGRAM  = install -c -m 755
INSTALL_DATA     = install -c -m 644
RM               = rm -f
RMDIR            = sh -c 'for d; do test ! -d "$$d" || rmdir -p "$$d"; done' rmdir

SRCDIR           = src
SRCINCDIR        = include
# Use VAR := $(shell CMD) instead of VAR != CMD to support old make versions
FIND_LIBHEADERS := find $(SRCINCDIR) -type f \( -name '*.h' -o \
                                             \( -name 'c*' ! -name '*.*' \) \)
LIBHEADERS      := $(shell $(FIND_LIBHEADERS))
ALLHEADERS      := $(LIBHEADERS) $(wildcard $(SRCDIR)/*.h)
LIBSRCS         := $(wildcard $(SRCDIR)/*.c)
DLIBOBJEXT       = .dl.o
SLIBOBJEXT       = .o
DLIBOBJS        := $(patsubst %.c,%$(DLIBOBJEXT),$(LIBSRCS))
SLIBOBJS        := $(patsubst %.c,%$(SLIBOBJEXT),$(LIBSRCS))

TESTDIR          = test
TESTNAMEPREFIX   = $(TESTDIR)/test_
TESTRUNPREFIX    = run_
TESTLDFLAGS      = -L$(BUILDDLIBDIR) $(LDFLAGS)
TESTLIBS         = -l$(LIBNAME)
TESTSRCS_C      := $(wildcard $(TESTNAMEPREFIX)*.c)
TESTSRCS_CPP    := $(wildcard $(TESTNAMEPREFIX)*.cpp)
TESTOBJS_C      := $(patsubst %.c,%.o,$(TESTSRCS_C))
TESTOBJS_CPP    := $(patsubst %.cpp,%.o,$(TESTSRCS_CPP))
TESTPRGS_C      := $(patsubst %.c,%,$(TESTSRCS_C))
TESTPRGS_CPP    := $(patsubst %.cpp,%,$(TESTSRCS_CPP))
TESTPRGS         = $(TESTPRGS_C) $(TESTPRGS_CPP)
TESTRUNS        := $(patsubst $(TESTNAMEPREFIX)%,$(TESTRUNPREFIX)%,$(TESTPRGS))

all: dlib slib
dlib: $(BUILDDLIBPATH)
slib: $(BUILDSLIBPATH)

# Generously marking all header files as potential dependencies
$(DLIBOBJS): %$(DLIBOBJEXT): %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(CFLAGS) $(DLIBCFLAGS) $< -o $@

$(SLIBOBJS): %$(SLIBOBJEXT): %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(CFLAGS) $(SLIBCFLAGS) $< -o $@

$(TESTOBJS_C): %.o: %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(CFLAGS) $< -o $@

$(TESTOBJS_CPP): %.o: %.cpp $(ALLHEADERS)
	$(CXX) -c -I$(SRCINCDIR) $(CXXFLAGS) $< -o $@

$(BUILDDLIBPATH): $(DLIBOBJS)
	$(MKINSTALLDIRS) $(BUILDDLIBDIR)
	$(CC) $(BUILDDLIBFLAGS) $(LDFLAGS) $^ -o $@

$(BUILDSLIBPATH): $(SLIBOBJS)
	$(MKINSTALLDIRS) $(BUILDSLIBDIR)
	$(RM) $@
	$(AR) $(BUILDSLIBFLAGS) $@ $^

$(TESTPRGS_C): %: %.o $(BUILDDLIBPATH)
	$(CC) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

$(TESTPRGS_CPP): %: %.o $(BUILDDLIBPATH)
	$(CXX) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

$(TESTRUNS): $(TESTRUNPREFIX)%: $(TESTNAMEPREFIX)%
	$<

install: install-headers install-lib

install-headers:
	$(MKINSTALLDIRS) $(patsubst $(SRCINCDIR)/%,$(DESTDIR)$(PKGINCDIR)/%,\
	                            $(sort $(dir $(LIBHEADERS))))
	for h in $(patsubst $(SRCINCDIR)/%,%,$(LIBHEADERS)); do \
	  $(INSTALL_DATA) $(SRCINCDIR)/"$$h" $(DESTDIR)$(PKGINCDIR)/"$$h"; \
	done

install-lib: install-dlib install-slib

install-dlib: $(BUILDDLIBPATH)
	$(MKINSTALLDIRS) $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROGRAM) $(BUILDDLIBPATH) $(DESTDIR)$(LIBDIR)
	$(POSTINSTALL) $(POSTINSTALLFLAGS) $(DESTDIR)$(DLIBPATH)

install-slib: $(BUILDSLIBPATH)
	$(MKINSTALLDIRS) $(DESTDIR)$(LIBDIR)
	$(INSTALL_DATA) $(BUILDSLIBPATH) $(DESTDIR)$(LIBDIR)

test check: $(TESTRUNS)

clean:
	$(RM) $(foreach D,$(SRCDIR) $(TESTDIR),$D/*.o $D/*.d)
	$(RM) $(BUILDDLIBPATH) $(BUILDSLIBPATH) $(TESTPRGS)
	@$(RMDIR) $(BUILDDLIBDIR) $(BUILDSLIBDIR)

.PHONY: all dlib slib clean check test $(TESTRUNS)
.PHONY: install install-headers install-lib install-dlib install-slib
