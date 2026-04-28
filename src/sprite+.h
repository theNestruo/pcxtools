/*
 * Additional support routines for managing sprites
 * Coded by theNestruo
 */

#pragma once

#include "bitmap.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

// (forward declarations)
typedef struct stSpriteSolver SpriteSolver;
typedef struct stSprWriterPlus SprWriterPlus;

typedef struct {
	int value;
	int scanlineCount; // number of scanlines with the value
} ScanlineCount;

typedef struct {
	int x, y;
} Rect;

typedef struct {
	SpriteSolver *spriteSolver; // reference
	byte color; // reference

	// Solutions
	int rectsPerSolution;
	Rect *rects;
	int rectCount;
} ColorSolver;

struct stSpriteSolver {
	// Sprite to solve
	SprWriterPlus *cfg; // reference
	Bitmap *bitmap; // reference
	int x0, y0, width, height;

	// Solution
	ColorSolver colorSolver[16];
	int solutionIndexes[16]; // best
	ScanlineCount bestScanlineCount;
};

struct stSprWriterPlus {
	// Data container
	SpriteSolver *sprites;
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

void sprWriterPlusInit(SprWriterPlus *instance, int argc, char **argv);

void sprWriterPlusReadSprites(SprWriterPlus *instance, Bitmap *bitmap);

int sprWriterPlusWrite(SprWriterPlus *instance, FILE *sprFile, FILE *spatFile);

void sprWriterPlusDone(SprWriterPlus *instance);

