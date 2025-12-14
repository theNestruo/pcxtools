
#
# file lists
#

TOOLS=\
	$(PCX2MSX) \
	$(PCX2SPR) \
	$(PCX2MSXPLUS) \
	$(PCX2SPRPLUS) \
	$(PNG2MSX) \
	$(PNG2SPR) \
	$(PNG2SPRPLUS) \
	$(TMX2BIN)

TESTS=\
	test/charset.pcx.chr \
	test/charset.pcx.clr \
	test/charset.png.chr \
	test/charset.png.clr \
	test/bigsprites.pcx.spat.asm \
	test/bigsprites.pcx.spr.asm \
	test/bigsprites.png.spat.asm \
	test/bigsprites.png.spr.asm \
	test/screen.tmx.bin \
	test/metatiles.tmx.bin

COMMON_DEPS_C=\
	src/args.c

COMMON_DEPS_PCX_C=\
	src/bitmap.c \
	src/readpcx.c

COMMON_DEPS_PNG_C=\
	src/bitmap.c \
	src/readpng.c \
	src/lodepng/lodepng.c

COMMON_DEPS_TMX_C=\
	src/tiled.c \
	src/readtmx.c

COMMON_DEPS_H=\
	src/args.h

COMMON_DEPS_PCX_H=\
	src/bitmap.h \
	src/readpcx.h

COMMON_DEPS_PNG_H=\
	src/bitmap.h \
	src/readpng.h \
	src/lodepng/lodepng.h

COMMON_DEPS_TMX_H=\
	src/tiled.h \
	src/readtmx.h

#
# default target
#

default: compile

clean:
	$(REMOVE) $(TOOLS)

compile: $(TOOLS)

test: compile $(TESTS)

demos: demopcx demopng

#
# main targets
#

$(PCX2MSX): src/pcx2msx.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/charset.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src/charset.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/charset.c -o $@

$(PCX2SPR): src/pcx2spr.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/sprite.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src/sprite.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/sprite.c -o $@

$(PCX2MSXPLUS): src/pcx2msx+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/charset.c src/nametable.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src/charset.h src/nametable.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/charset.c src/nametable.c -o $@

$(PCX2SPRPLUS): src/pcx2spr+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/sprite+.c src/output.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src/sprite.h src/sprite+.h src/output.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src/sprite+.c src/output.c -o $@

$(PNG2MSX): src/png2msx.c $(COMMON_DEPS_C) $(COMMON_DEPS_PNG_C) src/charset.c src/nametable.c $(COMMON_DEPS_H) $(COMMON_DEPS_PNG_H) src/charset.h src/nametable.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PNG_C) src/charset.c src/nametable.c -o $@ $(CCOMPILER_LINKER_OPTIONS)

$(PNG2SPR): src/png2spr.c $(COMMON_DEPS_C) $(COMMON_DEPS_PNG_C) src/sprite.c $(COMMON_DEPS_H) $(COMMON_DEPS_PNG_H) src/sprite.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PNG_C) src/sprite.c -o $@ $(CCOMPILER_LINKER_OPTIONS)

$(PNG2SPRPLUS): src/png2spr+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PNG_C) src/sprite+.c src/output.c $(COMMON_DEPS_H) $(COMMON_DEPS_PNG_H) src/sprite.h src/sprite+.h src/output.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PNG_C) src/sprite+.c src/output.c -o $@ $(CCOMPILER_LINKER_OPTIONS)

$(TMX2BIN): src/tmx2bin.c $(COMMON_DEPS_C) $(COMMON_DEPS_TMX_C) $(COMMON_DEPS_H) $(COMMON_DEPS_TMX_H)
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_TMX_C) -o $@

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

demopcx: $(PCX2SPRPLUS) pcx2sprplus_demo/demo.pcx pcx2sprplus_demo/demo.asm
	$(PCX2SPRPLUS) -v -vv -w24 -h32 pcx2sprplus_demo/demo.pcx

demopng: $(PNG2SPRPLUS) png2sprplus_demo/demo.png png2sprplus_demo/demo.asm
	$(PNG2SPRPLUS) -v -vv -w24 -h32 png2sprplus_demo/demo.png
