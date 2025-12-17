
#
# file lists
#

TARGETS=\
	$(PCX2MSX) \
	$(PCX2SPR) \
	$(PCX2MSXPLUS) \
	$(PCX2SPRPLUS) \
	$(PNG2MSX) \
	$(PNG2SPR) \
	$(PNG2SPRPLUS) \
	$(TMX2BIN)

OBJS_PCX=\
	src/args.o \
	src/bitmap.o \
	src/readpcx.o

OBJS_PNG=\
	src/args.o \
	src/bitmap.o \
	src/readpng.o \
	src/lodepng/lodepng.o

OBJS_TMX=\
	src/args.o \
	src/tiled.o \
	src/readtmx.o

DEPS_PCX=\
	src/args.h \
	src/bitmap.h \
	src/readpcx.h

DEPS_PNG=\
	src/args.h \
	src/bitmap.h \
	src/readpng.h \
	src/lodepng/lodepng.h

DEPS_TMX=\
	src/args.h \
	src/tiled.h \
	src/readtmx.h

TESTS=\
	test/charset.pcx.chr \
	test/charset.pcx.clr \
	test/bigsprites.pcx.spat.asm \
	test/bigsprites.pcx.spr.asm \
	test/charset.png.chr \
	test/charset.png.clr \
	test/bigsprites.png.spat.asm \
	test/bigsprites.png.spr.asm \
	test/screen.tmx.bin \
	test/metatiles.tmx.bin

#
# default target
#

default: all

clean:
	$(REMOVE) $(TARGETS)

all: $(TARGETS)

test: $(TOOLS) $(TESTS)

demo: demo_sprplus_pcx demo_sprplus_png

.PHONY: default clean all test demo

#
# main targets
#

$(PCX2MSX): src/pcx2msx.c $(OBJS_PCX) src/charset.o $(DEPS_PCX) src/charset.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PCX) src/charset.o

$(PCX2MSXPLUS): src/pcx2msx+.c $(OBJS_PCX) src/charset.c src/nametable.c $(DEPS_PCX) src/charset.h src/nametable.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PCX) src/charset.c src/nametable.c

$(PCX2SPR): src/pcx2spr.c $(OBJS_PCX) src/sprite.c $(DEPS_PCX) src/sprite.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PCX) src/sprite.c

$(PCX2SPRPLUS): src/pcx2spr+.c $(OBJS_PCX) src/sprite+.c src/output.c $(DEPS_PCX) src/sprite.h src/sprite+.h src/output.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PCX) src/sprite+.c src/output.c

#

$(PNG2MSX): src/png2msx.c $(OBJS_PNG) src/charset.o src/nametable.o $(DEPS_PNG) src/charset.h src/nametable.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PNG) src/charset.o src/nametable.o $(LDFLAGS)

$(PNG2SPR): src/png2spr.c $(OBJS_PNG) src/sprite.o $(DEPS_PNG) src/sprite.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PNG) src/sprite.o $(LDFLAGS)

$(PNG2SPRPLUS): src/png2spr+.c $(OBJS_PNG) src/sprite+.o src/output.o $(DEPS_PNG) src/sprite.h src/sprite+.h src/output.h
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_PNG) src/sprite+.o src/output.o $(LDFLAGS)

#

$(TMX2BIN): src/tmx2bin.c $(OBJS_TMX) $(DEPS_TMX)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_TMX)

#
# test targets
#

test/charset.pcx.chr test/charset.pcx.clr &: $(PCX2MSX) test/charset.pcx
	$(PCX2MSX) -v test/charset.pcx

test/charset.png.chr test/charset.png.clr &: $(PNG2MSX) test/charset.png
	$(PNG2MSX) -v test/charset.png

test/bigsprites.pcx.spat.asm test/bigsprites.pcx.spr.asm &: $(PCX2SPRPLUS) test/bigsprites.pcx
	$(PCX2SPRPLUS) -v -vv -w24 -h32 test/bigsprites.pcx

test/bigsprites.png.spat.asm test/bigsprites.png.spr.asm &: $(PNG2SPRPLUS) test/bigsprites.png
	$(PNG2SPRPLUS) -v -vv -w24 -h32 test/bigsprites.png

test/screen.tmx.bin: $(TMX2BIN) test/screen.tmx
	$(TMX2BIN) -v test/screen.tmx

test/metatiles.tmx.bin: $(TMX2BIN) test/metatiles.tmx
	$(TMX2BIN) -v -t2 test/metatiles.tmx

test/test-pal.png.chr test/test-pal.png.clr &: $(PNG2MSX) test/test-pal.png
	$(PNG2MSX) -v test/test-pal.png

#
# demo targets
#

demo_sprplus_pcx: $(PCX2SPRPLUS) demo/sprplus/pcx/demo.pcx demo/sprplus/pcx/demo.asm
	$(PCX2SPRPLUS) -v -vv -w24 -h32 demo/sprplus/pcx/demo.pcx
	$(ASSEMBLER) demo/sprplus/pcx/demo.asm

demo_sprplus_png: $(PNG2SPRPLUS) demo/sprplus/png/demo.png demo/sprplus/png/demo.asm
	$(PNG2SPRPLUS) -v -vv -w24 -h32 demo/sprplus/png/demo.png
	$(ASSEMBLER) demo/sprplus/png/demo.asm
