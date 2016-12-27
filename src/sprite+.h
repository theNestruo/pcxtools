/*
 * Additional support routines for managing sprites
 * Coded by theNestruo
 */

#ifndef SPRITE_PLUS_H_INCLUDED
#define SPRITE_PLUS_H_INCLUDED

#include "bitmap.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

struct stScanlineCount {
	int value;
	int scanlineCount; // number of scanlines with the value
};

struct stRect {
	int x, y;
};

struct stColorSolver {
	struct stSpriteSolver *spriteSolver; // reference
	byte color; // reference
	
	// Solutions
	int rectsPerSolution;
	struct stRect *rects;
	int rectCount;
};

struct stSpriteSolver {
	// Sprite to solve
	struct stSprWriterPlus *cfg; // reference
	struct stBitmap *bitmap; // reference
	int x0, y0, width, height;

	// Solution
	struct stColorSolver colorSolver[16];
	int solutionIndexes[16]; // best
	struct stScanlineCount bestScanlineCount;
};

struct stSprWriterPlus {
	// Data container
	struct stSpriteSolver *sprites;
	int spriteCount;
	
	// Arguments
	int spriteWidth;
	int spriteHeight;
	int offsetX;
	int offsetY;
	int attributePadding;
	byte terminator;
	int binaryOutput;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
void sprWriterPlusOptions();

void sprWriterPlusInit(struct stSprWriterPlus *instance, int argc, char **argv);

void sprWriterPlusReadSprites(struct stSprWriterPlus *instance, struct stBitmap *bitmap);

int sprWriterPlusWrite(struct stSprWriterPlus *instance, FILE *sprFile, FILE *spatFile);

void sprWriterPlusDone(struct stSprWriterPlus *instance);

#endif // SPRITE_PLUS_H_INCLUDED
