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
	@mkdir -p $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
	install -m 0755 $(MPLEGACYSUPPLIB)    $(DESTDIR)$(PREFIX)/lib
	install -m 0755 $(wildcard include/*) $(DESTDIR)$(PREFIX)/include

clean:
	@rm -f $(SRCDIR)*.o $(SRCDIR)*.d $(MPLEGACYSUPPLIB)
