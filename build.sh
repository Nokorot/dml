#!/bin/sh

IN_FILE="$HOME/.dml/main"

PRGNAME="_dml_composer"

### Source files
SRC="$(find src -type f \( -name '*.cpp' -o -name "*.c" \))"
# CC="g++"
CC="clang++"

### Compile definions
# DEFS="-D_WIN32_WINNT=0x0602"

### Compililation flags
STD="--std=c++20"
LIBS="-lstdc++fs -lcrypto"

DFLAGS="$STD $LIBS -g3 -O0 -Wall -pedantic"
RFLAGS="$STD $LIBS -O3"  

### Instalation destination
DST="/usr"

debug() {
    set -x
    $CC $DFLAGS -o $PRGNAME $SRC || exit 1
    echo "Build debug complete!"
    set +x
}

release() {
  $CC $RFLAGS -o $PRGNAME $SRC || exit 1
  echo "Build debug complete!"
}

install() {
  cp $PRGNAME $DST/bin/$PRGNAME
  chmod 755 $DST/bin/$PRGNAME
}

run() {
    set -x 
    [ -z "$1" ] || IN_FILE="$1"
    ./$PRGNAME "$IN_FILE"
    set +x 
}

clean() {
  echo cleaning
  set -x
  rm -rf $PRGNAME
}

usage() {
    echo "Usage: $0 [subcmd]\n"
    echo ""
    echo "Subcmds:"
    echo "    debug         debug build"
    echo "    run           debug build and run the executable if the compilation succeeds"
    echo "    install       release build and install the resulting executable"
    echo "    clean         delete all build files"
}

set -e
if [ -z "$1" ]; then 
    debug
else
  case "$1" in
    debug)    debug ;;
    run)      debug && { shift; run $@; } ;;
    install)  release && install ;;
    clean)    clean ;;
  esac
fi
