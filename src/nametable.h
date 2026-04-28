/*
 * Support routines for output in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#pragma once

#include "charset.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

typedef struct {
	// Data container
	int *namtbl;
	int namtblSize;
} NameTable;

typedef struct {
	// Arguments
	int namtblOffset;
	int generateBlank;
	int blankAt;
	int removeRepeated;
	int removeEmpty;
	byte emptyColorRemoved;
	int traverseHorizontally; // (see CharsetProcessor)
} NameTableProcessor;

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -n[<00..ff>]	generate NAMTBL [starting at value <n>]
// -b<00..ff>	blank block at position <nn>
// -rr		    remove repeated tiles
// -rm<0..f>	remove solid tiles of <n> color
void nameTableProcessorOptions();

void nameTableProcessorInit(NameTableProcessor *instance, int argc, char **argv);

void nameTableProcessorGenerate(NameTableProcessor *instance,
	NameTable *nametable, Charset *charset);

void nameTableProcessorGenerateUsing(NameTableProcessor *instance,
	NameTable *nametable, Charset *charset, Charset *screen);

void nameTableProcessorApplyTo(NameTable *nametable, Charset *charset);

void nameTableProcessorPostProcess(NameTableProcessor *instance, NameTable *nametable);

int nameTableProcessorWrite(NameTableProcessor *instance, NameTable *nametable, FILE *namFile);

void nameTableProcessorDone(NameTableProcessor *instance);

void nameTableDone(NameTable *instance);

