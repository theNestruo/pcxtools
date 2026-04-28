/*
 * Support routines for managing graphic blocks in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#pragma once

#include <stdint.h>

#include "bitmap.h"

/* Symbolic constants ------------------------------------------------------ */

#define TILE_WIDTH (8)
#define TILE_HEIGHT (8)

/* Data structures --------------------------------------------------------- */

typedef struct {
    uint8_t pattern;
    uint8_t color;
} Line;

typedef struct {
    Line line[TILE_HEIGHT];
} Block;

typedef struct {
    // Data container
    Block *blocks;
    int blockCount;

    unsigned int width, height; // (in chars)
} Charset;

typedef struct {
    // Arguments
    int ignoreCollision;
    int forceStrippedImage;
    int patternMode;
    int postProcessRangeFrom;
    int postProcessRangeTo;
    int traverseHorizontally;

    // State
    uint8_t preferredBackground;
    int isStrippedImage;
} CharsetProcessor;

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

void charsetProcessorInit(CharsetProcessor *instance, int argc, char **argv);

int charsetProcessorRead(CharsetProcessor *instance, Charset *charset, Bitmap *bitmap);

void charsetProcessorPostProcess(CharsetProcessor *instance, Charset *charset);

int charsetProcessorWrite(CharsetProcessor *instance, Charset *charset, FILE *chrFile, FILE *clrFile);

void charsetProcessorDone(CharsetProcessor *instance);

void charsetDone(Charset *instance);

int isSolidBlock(Block *block, uint8_t color);

int isBlockEquals(Block *block, Block *other);

int blockIndex(Charset *charset, Block *block, int stopBefore);
