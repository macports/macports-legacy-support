#! /bin/sh

# Script to derive C source from arch-specific symbol lists

INROOT="$1"
OUT="$2"
shift 2
ARCHS="$@"

INEXT=".tmp"
TMPEXT=".c"
QUAL="only"
INSUF="$QUAL$INEXT"
TMPSUF="$QUAL$TMPEXT"

DEFTMPROOT="/tmp/getlibsyms"
TMPROOT="${TMPROOT:-$DEFTMPROOT}"
TMPIN="$TMPROOT/$(basename $INROOT)"
OUTBASE="$(basename $OUT)"
OUTROOT="$(basename -s.c $OUT)"
TMPOUT="$TMPROOT/$OUTBASE"

do_format() {
  cat $1 | while read TYPE SYM; do
    if ! echo $SYM | grep -q '^_'; then continue; fi
    SSYM=$(echo $SYM | sed 's|^_||')
    case $TYPE in
      "T" )
        echo "${2}void $SSYM(void){};"
        ;;
      "S" )
        echo "${2}void *$SSYM;"
        ;;
    esac
  done
}

mkdir -p "$TMPROOT" || exit $?

# First write the arch-independent portion
if [ -s "$INROOT-all-$INSUF" ]; then
  echo >"$TMPIN-all-$TMPSUF" || exit $?
  do_format "$INROOT-all-$INSUF" "" >>"$TMPIN-all-$TMPSUF" || exit $?
else
  cat /dev/null >"$TMPIN-all-$TMPSUF" || exit $?
fi

# Now write the arch-dependent portions
INDENT=" "
for ARCH in $ARCHS; do
  INFILE="$INROOT-$ARCH-$INSUF"
  OUTFILE="$TMPIN-$ARCH-$TMPSUF"
  if [ -s "$INFILE" ]; then
    echo >"$OUTFILE" || exit $?
    echo "#ifdef __${ARCH}__" >>"$OUTFILE" || exit $?
    do_format "$INFILE" "  " >>"$OUTFILE" || exit $?
    echo "#endif" >>"$OUTFILE" || exit $?
  else
    cat /dev/null >"$OUTFILE" || exit $?
  fi
done

# Now combine the results
cat >"$OUT" <<EOD
/*
 * $OUTBASE -- symbol-only source for $OUTROOT
 *
 * Compile with -fno-builtin
 * Linking with -nostdlib is desirable but causes errors on macOS 11+
 *
 * This version supports architectures: $ARCHS
 */
EOD
for ARCH in all $ARCHS; do
  cat "$TMPIN-$ARCH-$TMPSUF" || exit $?
done >"$TMPOUT"

# If no real content, make the file completely empty
if [ -s "$TMPOUT" ]; then
  cat "$TMPOUT" >>"$OUT" || exit $?
else
  cat /dev/null >"$OUT"
fi

if [ "$TMPROOT" == "$DEFTMPROOT" ]; then
 rm -rf "$TMPROOT"
fi
