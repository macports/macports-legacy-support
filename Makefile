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
SOEXT            = .dylib
LIBNAME          = MacportsLegacySupport
LIBFILE          = lib$(LIBNAME)$(SOEXT)
LIBPATH          = $(LIBDIR)/$(LIBFILE)
BUILDLIBDIR      = lib
BUILDLIBPATH     = $(BUILDLIBDIR)/$(LIBFILE)
BUILDLIBFLAGS    = -dynamiclib -headerpad_max_install_names \
                   -install_name @executable_path/../$(BUILDLIBPATH) \
                   -current_version 1.0 -compatibility_version 1.0
POSTINSTALL      = install_name_tool
POSTINSTALLFLAGS = -id $(LIBPATH)

ARCHFLAGS       ?=
CC              ?= cc $(ARCHFLAGS)
CFLAGS          ?= -Os -Wall
CXX             ?= c++ $(ARCHFLAGS)
CXXFLAGS        ?= -Os -Wall
LDFLAGS         ?=

MKINSTALLDIRS    = install -d -m 755
INSTALL_PROGRAM  = install -c -m 755
INSTALL_DATA     = install -c -m 644
RM               = rm -f
RMDIR            = sh -c 'for d; do test ! -d "$$d" || rmdir -p "$$d"; done' rmdir

SRCDIR           = src
SRCINCDIR        = include
ALLHEADERS      := $(wildcard $(SRCINCDIR)/*.h $(SRCINCDIR)/*/*.h $(SRCDIR)/*.h)
LIBOBJECTS      := $(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c))

TESTDIR          = test
TESTNAMEPREFIX   = $(TESTDIR)/test_
TESTRUNPREFIX    = run_
TESTLDFLAGS      = -L$(BUILDLIBDIR) $(LDFLAGS)
TESTLIBS         = -l$(LIBNAME)
TESTSRCS_C      := $(wildcard $(TESTNAMEPREFIX)*.c)
TESTSRCS_CPP    := $(wildcard $(TESTNAMEPREFIX)*.cpp)
TESTOBJS_C      := $(patsubst %.c,%.o,$(TESTSRCS_C))
TESTOBJS_CPP    := $(patsubst %.cpp,%.o,$(TESTSRCS_CPP))
TESTPRGS_C      := $(patsubst %.c,%,$(TESTSRCS_C))
TESTPRGS_CPP    := $(patsubst %.cpp,%,$(TESTSRCS_CPP))
TESTPRGS         = $(TESTPRGS_C) $(TESTPRGS_CPP)
TESTRUNS        := $(patsubst $(TESTNAMEPREFIX)%,$(TESTRUNPREFIX)%,$(TESTPRGS))

all: $(BUILDLIBPATH)

# Generously marking all header files as potential dependencies
$(LIBOBJECTS) $(TESTOBJS_C): %.o: %.c $(ALLHEADERS)
	$(CC) -c -I$(SRCINCDIR) $(CFLAGS) $< -o $@

$(TESTOBJS_CPP): %.o: %.cpp $(ALLHEADERS)
	$(CXX) -c -I$(SRCINCDIR) $(CXXFLAGS) $< -o $@

$(BUILDLIBPATH): $(LIBOBJECTS)
	$(MKINSTALLDIRS) $(BUILDLIBDIR)
	$(CC) $(BUILDLIBFLAGS) $(LDFLAGS) $^ -o $@

$(TESTPRGS_C): %: %.o $(BUILDLIBPATH)
	$(CC) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

$(TESTPRGS_CPP): %: %.o $(BUILDLIBPATH)
	$(CXX) $(TESTLDFLAGS) $< $(TESTLIBS) -o $@

$(TESTRUNS): $(TESTRUNPREFIX)%: $(TESTNAMEPREFIX)%
	$<

install: install-headers install-lib

install-headers:
	$(MKINSTALLDIRS) $(DESTDIR)$(PKGINCDIR)/sys
	$(MKINSTALLDIRS) $(DESTDIR)$(PKGINCDIR)/xlocale
	$(INSTALL_DATA) $(wildcard include/*.h include/c*) $(DESTDIR)$(PKGINCDIR)
	$(INSTALL_DATA) $(wildcard include/sys/*)     $(DESTDIR)$(PKGINCDIR)/sys
	$(INSTALL_DATA) $(wildcard include/xlocale/*) $(DESTDIR)$(PKGINCDIR)/xlocale

install-lib: $(BUILDLIBPATH)
	$(MKINSTALLDIRS) $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROGRAM) $(BUILDLIBPATH) $(DESTDIR)$(LIBDIR)
	$(POSTINSTALL) $(POSTINSTALLFLAGS) $(DESTDIR)$(LIBPATH)

test check: $(TESTRUNS)

clean:
	$(RM) $(foreach D,$(SRCDIR) $(TESTDIR),$D/*.o $D/*.d)
	$(RM) $(BUILDLIBPATH) $(TESTPRGS)
	@$(RMDIR) $(BUILDLIBDIR)

.PHONY: all clean install install-headers install-lib test check $(TESTRUNS)
