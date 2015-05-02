/*
 * Support routines for output in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#ifndef NAMETABLE_H_INCLUDED
#define NAMETABLE_H_INCLUDED

#include "charset.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

struct stNameTable {
	// Data container
	int *namtbl;
	int namtblSize;
};

struct stNameTableProcessor {
	// Arguments
	int namtblOffset;
	int generateBlank;
	int blankAt;
	int removeRepeated;
	int removeEmpty;
	byte emptyColorRemoved;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -n[<00..ff>]	generate NAMTBL [starting at value <n>]
// -b<00..ff>	blank block at position <nn>
// -rr		remove repeated tiles
// -rm<0..f>	remove solid tiles of <n> color
void nameTableProcessorOptions();

void nameTableProcessorInit(struct stNameTableProcessor *instance, int argc, char **argv);

void nameTableProcessorGenerate(struct stNameTableProcessor *instance,
	struct stNameTable *nametable, struct stCharset *charset);

void nameTableProcessorGenerateUsing(struct stNameTableProcessor *instance,
	struct stNameTable *nametable, struct stCharset *charset, struct stCharset *screen);

void nameTableProcessorApplyTo(struct stNameTable *nametable, struct stCharset *charset);

void nameTableProcessorPostProcess(struct stNameTableProcessor *instance, struct stNameTable *nametable);

int nameTableProcessorWrite(struct stNameTableProcessor *instance, struct stNameTable *nametable, FILE *namFile);

void nameTableProcessorDone(struct stNameTableProcessor *instance);

void nameTableDone(struct stNameTable *instance);

#endif // NAMETABLE_H_INCLUDED
