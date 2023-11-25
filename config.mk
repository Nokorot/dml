
VERSION = 0.2

IN_FILE = $(HOME)/.dml/main
PRGNAME = _dml_composer

GDB_DEBUG_FILE = debug.gdb

### Instalation destination
DST="/usr"

### Source files
# SRC="$(find src -type f \( -name '*.cpp' -o -name "*.c" \))"
CC = g++

### Compile definions
# DEFS="-D_WIN32_WINNT=0x0602"

### Compililation flags
STD = --std=c++20
LIBS = -lstdc++fs -lcrypto

CFLAGS = $(STD) -g3 -O0 -Wall -pedantic
LDFLAGS = $(LIBS)

# DFLAGS = $(STD) $(LIBS) -g3 -O0 -Wall -pedantic
RFLAGS = $(STD) $(LIBS) -O3
