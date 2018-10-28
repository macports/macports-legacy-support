

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

PREFIX  ?= /usr/local
DESTDIR ?= 

CC       ?= cc      # The C compiler.
CFLAGS   ?= -Os     # C compilation options which relate to
                    # optimization or debugging (usually
                    # just -g or -O).  Usually this wouldn't
                    # include -I options to specify the
                    # include directories, because then you
                    # couldn't override it on the command line
                    # easily as in the above example.
CXX      ?= c++     # The C++ compiler.  (Sometimes "CPP" instead
                    # of CXX.)
CXXFLAGS ?= -Os     # C++ compilation options related to 
                    # optimization or debugging (-O or -g).
F77      ?= f77     # The fortran compiler.
FFLAGS   ?=         # Optimization flags for fortran.

MPLEGACYSUPPNAME = MacportsLegacySupport
MPLEGACYSUPPLIB  = lib$(MPLEGACYSUPPNAME).dylib
INSTALLINCDIR    = $(PREFIX)/include/LegacySupport

INCDIR           = ${PWD}/include
SRCDIR           = ${PWD}/src/

LIBOBJECTS      := $(patsubst %.c,%.o,$(wildcard $(SRCDIR)*.c))
-include $(LIBOBJECTS:.o=.d)

%.o: %.c
	${CC} -c -I$(INCDIR) $(CFLAGS) -MP -MMD -MT $*.o -MT $*.d -MF $*.d -o $*.o $*.c

$(MPLEGACYSUPPLIB): $(LIBOBJECTS)
	$(CC) $(LDFLAGS) -dynamiclib $(LIBOBJECTS) -install_name $(PREFIX)/lib/$(MPLEGACYSUPPLIB) -current_version 1.0 -compatibility_version 1.0 -o $(MPLEGACYSUPPLIB)

all: $(MPLEGACYSUPPLIB)

install: all
	@mkdir -p $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(INSTALLINCDIR) $(DESTDIR)$(INSTALLINCDIR)/sys
	install    -m 0755 $(MPLEGACYSUPPLIB)      $(DESTDIR)$(PREFIX)/lib
	install -m 0755 $(wildcard include/*.h)    $(DESTDIR)$(INSTALLINCDIR)
	install -m 0755 $(wildcard include/c*)     $(DESTDIR)$(INSTALLINCDIR)
	install -m 0755 $(wildcard include/sys/*)  $(DESTDIR)$(INSTALLINCDIR)/sys

clean:
	@rm -f $(SRCDIR)*.o $(SRCDIR)*.d $(MPLEGACYSUPPLIB)
