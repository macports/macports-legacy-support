#! /bin/sh
# Script to extract list of version macros from SDK, and convert to #undefs

SDKROOT="$1"

grep MAC_OS_VERSION_ $SDKROOT/usr/include/AvailabilityVersions.h | sed -E 's/^#define +MAC_OS_VERSION_([0-9_]+).*/#undef MAC_OS_VERSION_\1/'
