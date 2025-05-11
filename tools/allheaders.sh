#! /bin/bash

# Script to generate list of #includes for all headers (with exceptions).
# Requires git, and hence can't be used in a Makefile rule.
#
# This creates forward and reverse-ordered "kitchen sink" headers.
#
# Because the builds of some headers may be unexpectedly influenced by the
# prior inclusion of other headers, this generates the list in both forward and
# reverse order.  By testing both, any given pair of headers is tested in
# both orders of inclusion.  More complex order sensitivities are not tested.

DIR=$(dirname "$0")

INCDIR="$DIR/../include"
TSTDIR="../xtest"  # Relative to INCDIR
FWDFILE="$TSTDIR/allheaders.h"
REVFILE="$TSTDIR/revheaders.h"

# Headers not included:

# Everything under _macports_extras, which are legacy-support private
FILTERS='^_macports_extras'
# AvailabilityInternal.h (the name says it)
FILTERS+='|AvailabilityInternal.h'
# Headers with a leading underscore, which are private to specific uses
FILTERS+='|/_'
# MacportsLegacySupport.h, which is our "configuration" header
FILTERS+='|MacportsLegacySupport.h'
# Framework headers under CoreFoundation, IOKit, and OpenGL
FILTERS+='|CoreFoundation/|IOKit/|OpenGL/'
# The 10.5-internal-only available.h
FILTERS+='|available.h'

# Headers without .h are C++-only, and not legal in basic-C builds.
CPPFILTER='[.]h$'

FWDLIST=""
REVLIST=""

cd "$INCDIR"
for f in $(git ls-files | egrep -v "$FILTERS" ); do
  FWDLIST="$FWDLIST $f"
  REVLIST="$f $REVLIST"
done

makelist () {
  for f in $1; do
    if echo $f | grep -v "$CPPFILTER" >/dev/null; then
      echo "#ifdef __cplusplus"
      echo "  #include <$f>"
      echo "#endif"
    else
      echo "#include <$f>"
    fi
  done
}

makelist "$FWDLIST" >"$FWDFILE"
makelist "$REVLIST" >"$REVFILE"
