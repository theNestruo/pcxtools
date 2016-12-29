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
	$(PCX2MSX) \
	$(PCX2SPR) \
	$(PCX2MSXPLUS) \
	$(PCX2SPRPLUS) \
	$(TMX2BIN)

TESTS=\
	$(TEST_PATH)\charset.pcx.chr \
	$(TEST_PATH)\charset.pcx.clr \
	$(TEST_PATH)\screen.tmx.bin \
	$(TEST_PATH)\metatiles.tmx.bin
	
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
	$(COPY) $(PCX2MSX) $(INSTALLATION_PATH)
	$(COPY) $(PCX2SPR) $(INSTALLATION_PATH)
	$(COPY) "$(PCX2MSXPLUS)" $(INSTALLATION_PATH)
	$(COPY) "$(PCX2SPRPLUS)" $(INSTALLATION_PATH)
	$(COPY) $(TMX2BIN) $(INSTALLATION_PATH)

#
# main targets
#

$(PCX2MSX): src\pcx2msx.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\charset.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c -o $@

$(PCX2SPR): src\pcx2spr.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\sprite.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite.c -o $@

$(PCX2MSXPLUS): src\pcx2msx+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c src\nametable.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\charset.h src\nametable.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\charset.c src\nametable.c -o $@

$(PCX2SPRPLUS): src\pcx2spr+.c $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite+.c src\output.c $(COMMON_DEPS_H) $(COMMON_DEPS_PCX_H) src\sprite.h src\sprite+.h src\output.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_PCX_C) src\sprite+.c src\output.c -o $@

$(TMX2BIN): src\tmx2bin.c $(COMMON_DEPS_C) $(COMMON_DEPS_TMX_C) $(COMMON_DEPS_H) $(COMMON_DEPS_TMX_H)
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) $(COMMON_DEPS_TMX_C) -o $@

#
# test targets
#

$(TEST_PATH)\charset.pcx.chr $(TEST_PATH)\charset.pcx.clr: $(PCX2MSX) $(TEST_PATH)\charset.pcx
	.\$(PCX2MSX) -v $(TEST_PATH)\charset.pcx
	
# %.pcx.spr: %.pcx
#	$(PCX2SPR) -v $<

$(TEST_PATH)\screen.tmx.bin: $(TMX2BIN) $(TEST_PATH)\screen.tmx
	.\$(TMX2BIN) -v $(TEST_PATH)\screen.tmx

$(TEST_PATH)\metatiles.tmx.bin: $(TMX2BIN) $(TEST_PATH)\metatiles.tmx
	.\$(TMX2BIN) -v -t2 $(TEST_PATH)\metatiles.tmx

demo: $(PCX2SPRPLUS) pcx2sprplus_demo\demo.pcx pcx2sprplus_demo\demo.asm
	.\$(PCX2SPRPLUS) -v -vv -w24 -h32 pcx2sprplus_demo\demo.pcx
	asmsx pcx2sprplus_demo\demo.asm
	cmd /c start pcx2sprplus_demo\demo.rom
