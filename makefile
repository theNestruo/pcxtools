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
REMOVE=cmd /c del

#
# file lists
#

TARGET_DIR=D:\MSX\tools

TOOLS=\
	$(PCX2MSX) \
	$(PCX2SPR) \
	$(PCX2MSXPLUS) \
	$(PCX2SPRPLUS)

COMMON_DEPS_C=\
	src\args.c \
	src\bitmap.c \
	src\readpcx.c

COMMON_DEPS_H=\
	src\args.h \
	src\bitmap.h \
	src\readpcx.h

#
# main targets
#

all: $(TOOLS)

install: $(TOOLS)
	$(COPY) $(PCX2MSX) $(TARGET_DIR)
	$(COPY) $(PCX2SPR) $(TARGET_DIR)
	$(COPY) "$(PCX2MSXPLUS)" $(TARGET_DIR)
	$(COPY) "$(PCX2SPRPLUS)" $(TARGET_DIR)

clean:
	$(REMOVE) $(TOOLS)

demo: $(PCX2SPRPLUS) pcx2sprplus_demo\demo.pcx pcx2sprplus_demo\demo.asm
	$(PCX2SPRPLUS) -v -vv -w24 -h32 pcx2sprplus_demo\demo.pcx
	asmsx pcx2sprplus_demo\demo.asm
	cmd /c start pcx2sprplus_demo\demo.rom

$(PCX2MSX): src\pcx2msx.c $(COMMON_DEPS_C) src\charset.c $(COMMON_DEPS_H) src\charset.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) src\charset.c -o $@

$(PCX2SPR): src\pcx2spr.c $(COMMON_DEPS_C) src\sprite.c $(COMMON_DEPS_H) src\sprite.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) src\sprite.c -o $@

$(PCX2MSXPLUS): src\pcx2msx+.c $(COMMON_DEPS_C) src\charset.c src\nametable.c $(COMMON_DEPS_H) src\charset.h src\nametable.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) src\charset.c src\nametable.c -o $@

$(PCX2SPRPLUS): src\pcx2spr+.c $(COMMON_DEPS_C) src\sprite+.c src\output.c $(COMMON_DEPS_H) src\sprite.h src\sprite+.h src\output.h
	$(CCOMPILER) $(CCOMPILER_OPTIONS) $< $(COMMON_DEPS_C) src\sprite+.c src\output.c -o $@
