#
# targets
#
PCX2MSX=bin/pcx2msx
PCX2SPR=bin/pcx2spr
PCX2MSXPLUS=bin/pcx2msx+
PCX2SPRPLUS=bin/pcx2spr+
PNG2MSX=bin/png2msx
PNG2SPR=bin/png2spr
PNG2SPRPLUS=bin/png2spr+
TMX2BIN=bin/tmx2bin

#
# options
# (see https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags)
#
CC=gcc
CFLAGS=-O3 -Wall -Wextra -Wshadow -Wdouble-promotion -Wundef
LDFLAGS=-lm

#
# commands
#
REMOVE=rm

#
# main makefile
#
include shared.makefile
