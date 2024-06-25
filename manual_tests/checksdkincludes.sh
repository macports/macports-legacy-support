#! /bin/sh

# Script to verify that files referencing __MPLS_SDK_* are including
# _macports_extras/sdkversion.h.
#
# The optional argument is a git commit to apply the test to.

ID="$1"

HEADER="sdkversion.h"
HEADERINC="_macports_extras/$HEADER"
HEADERPATH="include/$HEADERINC"

FILES="$(git grep -l '__MPLS_SDK_' $ID | grep -v ^$HEADERPATH)"

for f in $FILES; do
  if ! git grep -q "$HEADERINC" $ID -- "$f"; then
    echo "\"#include <$HEADERINC>\" is missing from $f"
    ! break
  fi
done
