
#   Copyright (C) 1994, 1996, 1997, 1998, 2001, 2003, 2005, 2006 Free
#   Software Foundation, Inc.

#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2, or (at
#   your option) any later version.

#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
#   02110-1301, USA. 

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
