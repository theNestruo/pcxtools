/*
 * Support routines for managing graphic blocks in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#ifndef CHARSETS_H_INCLUDED
#define CHARSETS_H_INCLUDED

#include "bitmap.h"

/* Symbolic constants ------------------------------------------------------ */

#define TILE_WIDTH (8)
#define TILE_HEIGHT (8)

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

struct stLine {
	byte pattern;
	byte color;
};

struct stBlock {
	struct stLine line[TILE_HEIGHT];
};

struct stCharset {
	// Data container
	struct stBlock *blocks;
	int blockCount;
};

struct stCharsetProcessor {
	// Arguments
	int ignoreCollision;
	int patternMode;
	int optimize;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -il		ignore line on color collision
// -hl		force higher color to be foreground
// -lh		force lower color to be foreground
// -f<0..7>	force bit <n> to be foreground (set) on patterns
// -b<0..7>	force bit <n> to be background (reset) on patterns
void charsetProcessorOptions();

void charsetProcessorInit(struct stCharsetProcessor *instance, int argc, char **argv);

int charsetProcessorRead(struct stCharsetProcessor *instance, struct stCharset *charset, struct stBitmap *bitmap);

void charsetProcessorPostProcess(struct stCharsetProcessor *instance, struct stCharset *charset);

int charsetProcessorWrite(struct stCharsetProcessor *instance, struct stCharset *charset, FILE *chrFile, FILE *clrFile);

void charsetProcessorDone(struct stCharsetProcessor *instance);

void charsetDone(struct stCharset *instance);

int isSolidBlock(struct stBlock *block, byte color);

int isBlockEquals(struct stBlock *block, struct stBlock *other);

int blockIndex(struct stCharset *charset, struct stBlock *block, int stopBefore);

#endif // CHARSETS_H_INCLUDED
