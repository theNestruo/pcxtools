#
# commands
#

CCOMPILER=gcc
CCOMPILER_OPTIONS=-Wall -O3
# CCOMPILER_OPTIONS=-Wall -ggdb
COPY=cmd /c copy
PCX2MSX=pcx2msx.exe
PCX2SPR=pcx2spr.exe
PCX2MSXPLUS=pcx2msx+.exe
PCX2SPRPLUS=pcx2spr+.exe
TMX2BIN=tmx2bin.exe
REMOVE=cmd /c del

#
# file lists
#

TEST_PATH=test

INSTALLATION_PATH=D:\MSX\bin\tools

TOOLS=\
	bin\$(PCX2MSX) \
	bin\$(PCX2SPR) \
	bin\$(PCX2MSXPLUS) \
	bin\$(PCX2SPRPLUS) \
	bin\$(TMX2BIN)

TESTS=\
	test\charset.pcx.chr \
	test\charset.pcx.clr \
	test\bigsprites.pcx.spat.asm \
	test\bigsprites.pcx.spr.asm \
	test\screen.tmx.bin \
	test\metatiles.tmx.bin
	
COMMON_DEPS_C=\
	src\args.c

COMMON_DEPS_PCX_C=\
	src\bitmap.c \
	src\readpcx.c

COMMON_DEPS_TMX_C=\
	src\tiled.c \
	src\readtmx.c
	
COMMON_DEPS_H=\
	src\args.h

COMMON_DEPS_PCX_H=\
	src\bitmap.h \
	src\readpcx.h

COMMON_DEPS_TMX_H=\
	src\tiled.h \
	src\readtmx.h

#
# default target
#

default: compile

clean:
	$(REMOVE) $(TOOLS)

cleanTest:
	$(REMOVE) $(TESTS)	

compile: $(TOOLS)

test: compile $(TESTS)

install: compile test
	$(COPY) "bin\$(PCX2MSX)" $(INSTALLATION_PATH)
	$(COPY) "bin\$(PCX2SPR)" $(INSTALLATION_PATH)
	$(COPY) "bin\$(PCX2MSXPLUS)" $(INSTALLATION_PATH)
	$(COPY) "bin\$(PCX2SPRPLUS)" $(INSTALLATION_PATH)
	$(COPY) "bin\$(TMX2BIN)" $(INSTALLATION_PATH)

#
# main targets
#

bin\$(PCX2MSX): src\pcx2msx.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\charset.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c -o $@

bin\$(PCX2SPR): src\pcx2spr.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\sprite.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite.c -o $@

bin\$(PCX2MSXPLUS): src\pcx2msx+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c src\nametable.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\charset.h src\nametable.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c src\nametable.c -o $@

bin\$(PCX2SPRPLUS): src\pcx2spr+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite+.c src\output.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\sprite.h src\sprite+.h src\output.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite+.c src\output.c -o $@

bin\$(TMX2BIN): src\tmx2bin.c $(COMMON_DEPS_C) $(COMMON_DEPS_TMX_C) $(COMMON_DEPS_H) $(COMMON_DEPS_TMX_H)
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_TMX_C) -o $@

#
# test targets
#

test\charset.pcx.chr test\charset.pcx.clr: bin\$(PCX2MSX) test\charset.pcx
	bin\$(PCX2MSX) -v test\charset.pcx
	
# %.pcx.spr: %.pcx
#	$(PCX2SPR) -v $<

test\bigsprites.pcx.spat.asm test\bigsprites.pcx.spr.asm: bin\$(PCX2SPRPLUS) test\bigsprites.pcx
	bin\$(PCX2SPRPLUS) -v -vv -w24 -h32 test\bigsprites.pcx

test\screen.tmx.bin: bin\$(TMX2BIN) test\screen.tmx
	bin\$(TMX2BIN) -v test\screen.tmx

test\metatiles.tmx.bin: bin\$(TMX2BIN) test\metatiles.tmx
	bin\$(TMX2BIN) -v -t2 test\metatiles.tmx

demo: bin\$(PCX2SPRPLUS) pcx2sprplus_demo\demo.pcx pcx2sprplus_demo\demo.asm
	bin\$(PCX2SPRPLUS) -v -vv -w24 -h32 pcx2sprplus_demo\demo.pcx
	asmsx pcx2sprplus_demo\demo.asm
	cmd /c start pcx2sprplus_demo\demo.rom
