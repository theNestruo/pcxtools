#
# commands
#

CCOMPILER=gcc
CCOMPILER_OPTIONS=-Wall -O3
# CCOMPILER_OPTIONS=-Wall -ggdb
CCOMPILER_LINKER_OPTIONS=
PCX2MSX=bin\pcx2msx.exe
PCX2SPR=bin\pcx2spr.exe
PCX2MSXPLUS=bin\pcx2msx+.exe
PCX2SPRPLUS=bin\pcx2spr+.exe
PNG2MSX=bin\png2msx.exe
PNG2SPR=bin\png2spr.exe
PNG2SPRPLUS=bin\png2spr+.exe
TMX2BIN=bin\tmx2bin.exe
REMOVE=cmd /c del

#
# main makefile
#

include shared.makefile
