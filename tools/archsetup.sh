#! /bin/bash

# Tool to create temporary simple "C" Makefile for misc tools, based on
# the architecture settings in macports.conf.
#
# Usage is: archsetup.sh [-m32|-m64|-unv] [<output dir>]
# Options modify the architecture specification.

DIR=$(dirname "$0")

OPT=""
case "$1" in
  "-m32" | "-m64" | "-unv" )
    OPT="$1"
    shift
    ;;
esac

if [ "$1" != "" ]; then
  DEST="$1"
else
  DEST="$DIR"
fi

PREFIX="/opt/local/"
CONF="$PREFIX/etc/macports/macports.conf"
OUT="Makefile"

BUILD_ARCH="$(grep '^build_arch' $CONF | awk '{print $2}')"
UNV_ARCHS="$(grep '^universal_archs' $CONF \
             | sed 's|universal_archs[[:blank:]]*||')"

case "$OPT" in
  "-m32" )
    case "$BUILD_ARCH" in
      "ppc64" )
        ARCHS="ppc"
        ;;
      "x86_64" )
        ARCHS="i386"
        ;;
      * )
        ARCHS="$BUILD_ARCH"
    esac
    ;;
  "-m64" )
    case "$BUILD_ARCH" in
      "ppc" | "ppc7400" )
        ARCHS="ppc64"
        ;;
      "i386" )
        ARCHS="x86_64"
        ;;
      * )
        ARCHS="$BUILD_ARCH"
    esac
    ;;
  "-unv" )
    ARCHS="$UNV_ARCHS"
    ;;
  * )
    ARCHS="$BUILD_ARCH"
    ;;
esac

if [ "$CC" == "" ]; then
  CC="cc"
fi

ARCHFLAGS="$(for a in $ARCHS; do echo -n ' -arch' $a; done)"
CCCMD=$'\t'"$CC$ARCHFLAGS \$^ -o \$@"

cat >"$DEST/$OUT" <<EOF
%: %.c
$CCCMD
EOF
