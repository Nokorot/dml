#!/bin/sh

IN_FILE="$HOME/.dml/main"

PRGNAME="_dml_composer"

GDB_DEBUG_FILE="debug.gdb"

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

gen_gdb_commands() {
  < $GDB_DEBUG_FILE sed 's,%IN_FILE,'"$IN_FILE,g"
}

_gdb() {
  cmdf=$(mktemp)
  gen_gdb_commands > $cmdf
  gdb -q -x "$cmdf" $PRGNAME
  rm $cmdf
}

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

  cp bin/dml $DST/bin/dml
  chmod 755 $DST/bin/dml
}

run() {
    set -x 
    # [ -z "$1" ] || { IN_FILE="$1"; shift; }
    ./$PRGNAME "$IN_FILE" $@
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
    gdb)      _gdb ;;
    release)  release ;;
    run)      debug && { shift; run $@; } ;;
    install)  release && install ;;
    clean)    clean ;;
    *) echo "Unknown sub-command '$1'" ;;
  esac
fi
