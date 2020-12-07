#
# commands
#

CCOMPILER=gcc
CCOMPILER_OPTIONS=-Wall -O3
PCX2MSX=bin/pcx2msx
PCX2SPR=bin/pcx2spr
PCX2MSXPLUS=bin/pcx2msx+
PCX2SPRPLUS=bin/pcx2spr+
PNG2MSX=bin/png2msx
TMX2BIN=bin/tmx2bin
REMOVE=rm

#
# file lists
#

TESTS=\
	test/charset.pcx.chr \
	test/charset.pcx.clr \
	test/charset.png.chr \
	test/charset.png.clr \
	test/bigsprites.pcx.spat.asm \
	test/bigsprites.pcx.spr.asm \
	test/screen.tmx.bin \
	test/metatiles.tmx.bin

#
# main makefile
#

include makefile.common
