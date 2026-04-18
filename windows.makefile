#
# targets
#
PCX2MSX=bin/pcx2msx.exe
PCX2SPR=bin/pcx2spr.exe
PCX2MSXPLUS=bin/pcx2msx+.exe
PCX2SPRPLUS=bin/pcx2spr+.exe
PNG2MSX=bin/png2msx.exe
PNG2SPR=bin/png2spr.exe
PNG2SPRPLUS=bin/png2spr+.exe
TMX2BIN=bin/tmx2bin.exe

#
# options
# (see https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags)
#
CC=gcc
CFLAGS=-O3 -Wall -Wextra -Wshadow -Wdouble-promotion -Wundef
LDFLAGS=

#
# main makefile
#
include shared.makefile

#
# demo targets
#

.PHONY: demo
demo: demo_namtbl_png demo_sprplus_pcx demo_sprplus_png

demo_namtbl_png: $(PNG2MSX) demo/namtbl/png/screen-th.png demo/namtbl/png/screen-tv.png demo/namtbl/png/demo.asm
	$(PNG2MSX) -v -rm0 -rr -n -th demo/namtbl/png/screen-th.png
	$(PNG2MSX) -v -rm0 -rr -n -tv demo/namtbl/png/screen-tv.png
	asmsx demo/namtbl/png/demo.asm

demo_sprplus_pcx: $(PCX2SPRPLUS) demo/sprplus/pcx/demo.pcx demo/sprplus/pcx/demo.asm
	$(PCX2SPRPLUS) -v -vv -w24 -h32 demo/sprplus/pcx/demo.pcx
	asmsx demo/sprplus/pcx/demo.asm

demo_sprplus_png: $(PNG2SPRPLUS) demo/sprplus/png/demo.png demo/sprplus/png/demo.asm
	$(PNG2SPRPLUS) -v -vv -w24 -h32 demo/sprplus/png/demo.png
	asmsx demo/sprplus/png/demo.asm
