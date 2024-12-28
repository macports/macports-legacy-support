# Simple Makefile for building copyfile as standalone program.

copyfile: copyfile.c
	cc -D_COPYFILE_TEST -I../include $^ -o $@

# No-quarantine version which works on 10.4
copyfile-nq: copyfile.c
	cc -D_COPYFILE_TEST -D_NO_QUARANTINE -I../include $^ -o $@
