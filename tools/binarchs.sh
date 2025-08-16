#! /bin/bash

# Get architectures for system programs.
# NOTE: Currently needed to build 'which' on 10.4.

REFPROG="${1:-/usr/bin/true}"

ARCHS="$(file $REFPROG | grep ' executable ' | sed 's|.* executable ||')"
ARCHFLAGS="$(for a in $ARCHS; do echo -n ' -arch' $a; done)"

echo $ARCHFLAGS
