# Simple Makefile for building copyfile as standalone program.

copyfile: copyfile.c
	$(CC) -D_COPYFILE_TEST -I../include $^ -o $@

# No-quarantine version which works on 10.4
copyfile-nq: copyfile.c
	$(CC) -D_COPYFILE_TEST -D_NO_QUARANTINE -I../include $^ -o $@

# Versions for debugging arch-related issues (10.4-compatible).
DEBUG_FLAGS  = -g3 -O0 -D_COPYFILE_TEST -D_COPYFILE_DEBUG -D_NO_QUARANTINE
copyfile-ppc: copyfile.c
	$(CC) -arch ppc $(DEBUG_FLAGS) -I../include $^ -o $@
copyfile-i386: copyfile.c
	$(CC) -arch i386 $(DEBUG_FLAGS) -I../include $^ -o $@

copyfile-dbg: copyfile-ppc copyfile-i386

.PHONY: copyfile-dbg
