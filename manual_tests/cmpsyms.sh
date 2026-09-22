#! /bin/sh

FILE1="$1"
FILE2="$2"
SRC="$3"
shift 3
ARCHS="$@"

DEFTMPROOT="/tmp/getlibsyms"
TMPROOT="${TMPROOT:-$DEFTMPROOT}"
TMPEXT=".tmp"
BASE1=$(basename "$FILE1")
BASE2=$(basename "$FILE2")
TMP1="$TMPROOT/$BASE1$TMPEXT"
TMP2="$TMPROOT/$BASE2$TMPEXT"

NM="${NM:-/usr/bin/nm}"

# Avoid 'nm' errors when lib is empty
if ! [ -s "$SRC" ]; then
  exit 0
fi

# Avoid -U for compatibility with /usr/bin/nm on <10.6.
# Filter lines for leading hex.
get_syms() {
  $NM -g $2 $1 | grep '^[0-9a-fA-F]' | awk '{print $2 " " $3}' | egrep '^(T|S)'
}

mkdir -p "$TMPROOT" || exit $?

if [ "$ARCHS" == "" ]; then
  get_syms "$FILE1" >"$TMP1" || exit $?
  get_syms "$FILE2" >"$TMP2" || exit $?
  diff "$TMP1" "$TMP2"
else
  for ARCH in $ARCHS; do
    get_syms "$FILE1" "-arch $ARCH" >"$TMP1" || exit $?
    get_syms "$FILE2" "-arch $ARCH" >"$TMP2" || exit $?
    diff "$TMP1" "$TMP2"
  done
fi

if [ "$TMPROOT" == "$DEFTMPROOT" ]; then
 rm -rf "$TMPROOT"
fi
