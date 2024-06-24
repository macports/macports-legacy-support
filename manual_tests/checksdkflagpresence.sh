#! /bin/sh

# Script to verify that all SDK version flags used are included in the
# setup in sdkversion.h and the reports in headerinfo.c
#
# The optional argument is a git commit to apply the test to.

ID="$1"

HEADER="include/_macports_extras/sdkversion.h"
REPORT="manual_tests/headerinfo.c"
IGNORE="manual_tests/checksdkflagvalues.c"

VARS="$(git grep -Ew '__MPLS_PRE_[0-9]+_[0-9]+_SDK' $ID | grep -v ^$IGNORE | sed -E 's/.*(__MPLS_PRE_.+_SDK).*/\1/' | sort | uniq)"

for v in $VARS; do
  if ! git grep -q $v $ID -- "$HEADER"; then
    echo $v is missing from $HEADER
    ! break
  fi
  if ! git grep -q $v $ID -- "$REPORT"; then
    echo $v is missing from $REPORT
    ! break
  fi
done
