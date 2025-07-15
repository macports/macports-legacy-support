#! /bin/sh

# Check to see whether the 'arch' program accepts an arch argument, i.e.
# whether it's a 10.5+ version of 'arch'.
#
# Note that the arch returned by 'arch' is 'i386' on 'x86_64' machines,
# and may not be supported by some executables.  So instead we use
# the hw.machine sysctl parameter, which may also be 'i386' on 'x86_64'
# machines, but is always a valid arch for system programs.  Additionally,
# it may return "Power Macintosh", which we have to replace with "ppc".
#
# The basic approach is to try using the obtained arch as an argument
# to the 'arch' program targeting the 'true' program, which should succeed
# unless the 'arch' program fails to accept the argument.  We return either
# the full path of the successful arch program, or nothing.

ARCHPROG="$(which arch)"
TESTARCH="$(sysctl -n hw.machine)"
TESTPROG="$(which true)"

if [ "$TESTARCH" == "Power Macintosh" ]; then TESTARCH=ppc; fi

if "$ARCHPROG" "-$TESTARCH" "$TESTPROG" 2>/dev/null; then echo "$ARCHPROG"; fi
