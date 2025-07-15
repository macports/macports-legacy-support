#! /bin/sh

# This is a replacement for the 'arch' program, intended to be used on 10.4
# systems where 'arch' can't be used to specify an execution architecture.
#
# This works by using lipo to create a single-architecture copy of the target
# executable, and then running that.
#
# A complication in principle (though it doesn't happen on 10.4, which is
# the only OS where we expect to need this tool) is that on 10.5+, 'ppc'
# slices are coded as 'ppc7400', which lipo doesn't recognize when asked
# for 'ppc'.  So we include a fallback invocation for 'ppc'->'ppc7400'.
# We suppress the possible error message from the 'ppc' invocation, but if
# the 'ppc7400' invocation also fails, we'll get an error message from that.

ARCH="${1##-}"
PROG="$2"
shift 2
ARGS="$@"

DEST="$PROG-$ARCH"

LIPO=lipo

if [ "$ARCH" != 'ppc' ]; then
  $LIPO "$PROG" -thin "$ARCH" -output "$DEST" && exec "$DEST" "$ARGS"
else
  $LIPO "$PROG" -thin ppc -output "$DEST" 2>/dev/null \
  || $LIPO "$PROG" -thin ppc7400 -output "$DEST" && exec "$DEST" "$ARGS"
fi
