#
# commands
#

PCX2MSX=bin\pcx2msx.exe
PCX2SPR=bin\pcx2spr.exe
PCX2MSXPLUS=bin\pcx2msx+.exe
PCX2SPRPLUS=bin\pcx2spr+.exe
PNG2MSX=bin\png2msx.exe
PNG2SPR=bin\png2spr.exe
PNG2SPRPLUS=bin\png2spr+.exe
TMX2BIN=bin\tmx2bin.exe

CCOMPILER=gcc
# (see https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags)
CCOMPILER_OPTIONS=-Wall -Wextra -Wshadow -Wdouble-promotion -Wundef -O3
# CCOMPILER_OPTIONS=-Wall -Wextra -Wshadow -Wdouble-promotion -Wundef -ggdb
CCOMPILER_LINKER_OPTIONS=

REMOVE=cmd /c del

ASSEMBLER=asmsx

#
# main makefile
#

include shared.makefile
