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

	unsigned int width, height; // (in chars)
};

struct stCharsetProcessor {
	// Arguments
	int ignoreCollision;
	int forceStrippedImage;
	int patternMode;
	int postProcessRangeFrom;
	int postProcessRangeTo;
	int traverseHorizontally;

	// State
	byte preferredBackground;
	int isStrippedImage;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -il		 ignore line on color collision
// -sa		 auto-detect stripped images (default)
// -sy		 force image to be detected as stripped (simplified algorithm)
// -sn		 force image to be detected as non-stripped (default algorithm)
// -hl		 force higher color to be foreground
// -lh		 force lower color to be foreground
// -ld		 force lighter foreground, darker background
// -dl		 force darker foreground, lighter background
// -f<0..7>	 force bit <n> to be foreground (set) on patterns
// -b<0..7>	 force bit <n> to be background (reset) on patterns
// -pf<0000> post-process only the specified address range (from)
// -pt<ffff> post-process only the specified address range (to)
// -th       traverse image horizontally, then vertically (default)
// -tv       traverse image vertically, then horizontally
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
