/*
 * Support routines for output in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "charset.h"

/* Symbolic constants ------------------------------------------------------ */

#define PATTERN_MODE_UNSET (0x00)
#define PATTERN_MODE_FOREGROUND (0x10)
#define PATTERN_MODE_BACKGROUND (0x20)
#define PATTERN_MODE_HIGH_LOW (0x40)
#define PATTERN_MODE_LOW_HIGH (0x80)
#define PATTERN_MODE_MASK (PATTERN_MODE_UNSET | PATTERN_MODE_FOREGROUND | PATTERN_MODE_BACKGROUND | PATTERN_MODE_HIGH_LOW | PATTERN_MODE_LOW_HIGH)

/* Private function prototypes --------------------------------------------- */

int patternMode(int isForeground, char bit);
int readLine(struct stCharsetProcessor *instance, struct stLine *line, struct stBitmap *bitmap, int x, int y);
void negateAndSwap(struct stLine *line);
int isLineEquals(struct stLine *line, struct stLine *other);
byte colorAtBit(struct stLine *line, char bit);

void postProcessPattern(struct stCharsetProcessor *instance, struct stBlock *block);
void postProcessOptimize(struct stCharsetProcessor *instance, struct stBlock *block);
int isOptimizable(struct stLine *line);
int applyDirectOptimization(struct stLine *line, struct stLine *reference);
int applySwappedOptimization(struct stLine *line, struct stLine *reference);

/* Function bodies --------------------------------------------------------- */

// Supported arguments:
// -il		ignore line on color collision
// -hl		force higher color to be foreground
// -lh		force lower color to be foreground
// -f<0..7>	force bit <n> to be foreground (set) on patterns
// -b<0..7>	force bit <n> to be background (reset) on patterns
void charsetProcessorOptions() {

	printf("\t-il\tignore line on color collision\n");
	printf("\t-hl\tforce higher color to be foreground\n");
	printf("\t-lh\tforce lower color to be foreground\n");
	printf("\t-f<0..7>\tforce bit <n> to be foreground (set) on patterns\n");
	printf("\t-b<0..7>\tforce bit <n> to be background (reset) on patterns\n");
	printf("\t-o\tattempt to optimize CLRTBL for compression\n");
}

void charsetProcessorInit(struct stCharsetProcessor *this, int argc, char **argv) {

	// Init
	this->ignoreCollision = 0;
	this->patternMode = PATTERN_MODE_UNSET;

	// Read arguments
	int i;
	this->ignoreCollision = (argEquals(argc, argv, "-il") != -1);
	this->patternMode =
		  ((i = argStartsWith(argc, argv, "-f", 3)) != -1) ? patternMode(1, argv[i][2])
		: ((i = argStartsWith(argc, argv, "-b", 3)) != -1) ? patternMode(0, argv[i][2])
		: (argEquals(argc, argv, "-hl") != -1) ? PATTERN_MODE_HIGH_LOW
		: (argEquals(argc, argv, "-lh") != -1) ? PATTERN_MODE_LOW_HIGH
		: PATTERN_MODE_UNSET;
	this->optimize = (argEquals(argc, argv, "-o") != -1);
}

int charsetProcessorRead(struct stCharsetProcessor *this, struct stCharset *charset, struct stBitmap *bitmap) {

	// Allocate space for the blocks
	charset->blockCount = ((int) (bitmap->width / TILE_WIDTH)) * ((int) (bitmap->height / TILE_HEIGHT));
	charset->blocks = (struct stBlock*) calloc(charset->blockCount, sizeof(struct stBlock));

	// For each block...
	int x, y;
	struct stBlock *itBlock;
	for (y = 0, itBlock = charset->blocks; (y + TILE_HEIGHT) <= bitmap->height; y += TILE_HEIGHT) {
		for (x = 0; (x + TILE_WIDTH) <= bitmap->width; x += TILE_WIDTH, itBlock++) {

			// For each line...
			int i;
			struct stLine *itLine;
			for (i = 0, itLine = itBlock->line; i < TILE_HEIGHT; i++, itLine++) {
				if (readLine(this, itLine, bitmap, x, y + i) && !this->ignoreCollision)
					return 1;
			}
		}
	}

	return 0;
}

void charsetProcessorPostProcess(struct stCharsetProcessor *this, struct stCharset *charset) {

	if (this->patternMode != PATTERN_MODE_UNSET) {
		// For each block...
		int i;
		struct stBlock *itBlock;
		for (i = 0, itBlock = charset->blocks; i < charset->blockCount; i++, itBlock++) {
			postProcessPattern(this, itBlock);
		}
	}

	if (this->optimize) {
		// For each block...
		int i;
		struct stBlock *itBlock;
		for (i = 0, itBlock = charset->blocks; i < charset->blockCount; i++, itBlock++) {
			postProcessOptimize(this, itBlock);
		}
	}
}

void postProcessPattern(struct stCharsetProcessor *this, struct stBlock *block) {

	// For each line...
	int i;
	struct stLine *itLine;
	for (i = 0, itLine = block->line; i < TILE_HEIGHT; i++, itLine++) {

		// Apply current pattern mode
		switch (this->patternMode & PATTERN_MODE_MASK) {
		case PATTERN_MODE_FOREGROUND:
			// Force foreground bit
			if (!(itLine->pattern & (1 << (this->patternMode & 0x07))))
				negateAndSwap(itLine);
			break;

		case PATTERN_MODE_BACKGROUND:
			// Force background bit
			if (itLine->pattern & (1 << (this->patternMode & 0x07)))
				negateAndSwap(itLine);
			break;

		case PATTERN_MODE_HIGH_LOW:
			// Force higher color foreground
			if ((itLine->color >> 4) < (itLine->color & 0x0f))
				negateAndSwap(itLine);
			if ((itLine->pattern == 0xff) && ((itLine->color >> 4) < 2))
				negateAndSwap(itLine); // WORKAROUND: color 0 or 1 always background
			break;

		case PATTERN_MODE_LOW_HIGH:
			// Force higher color background
			if ((itLine->color & 0x0f) < (itLine->color >> 4))
				negateAndSwap(itLine);
			if ((itLine->pattern == 0x00) && ((itLine->color & 0x0f) == 15))
				negateAndSwap(itLine); // WORKAROUND: color 15 always foreground
			break;
		}
	}
}

void postProcessOptimize(struct stCharsetProcessor *this, struct stBlock *block) {

	int i;
	for (i = 0; i < TILE_HEIGHT; i++) {
		struct stLine *line = &(block->line[i]);
		if (!isOptimizable(line))
			continue;

		int j;
		for (j = 1; j < TILE_HEIGHT; j++) {

			// Try to optimize using the following lines
			if (i + j < TILE_HEIGHT) {
				struct stLine *ref = &(block->line[i + j]);
				if (applyDirectOptimization(line, ref)
						|| applySwappedOptimization(line, ref))
					break;
			}

			// Try to optimize using the previous lines
			if (i - j >= 0) {
				struct stLine *ref = &(block->line[i - j]);
				if (applyDirectOptimization(line, ref)
						|| applySwappedOptimization(line, ref))
					break;
			}
		}
	}
}

int isOptimizable(struct stLine *line) {

	return (line->pattern == 0x00) || (line->pattern == 0xff);
}

int applyDirectOptimization(struct stLine *line, struct stLine *reference) {

	// (sanity check)
	if (!isOptimizable(line) || isOptimizable(reference))
		return 0;

	int valid = line->pattern == 0x00
		? (line->color & 0x0f) == (reference->color & 0x0f)
		: (line->color >> 4) == (reference->color >> 4);

	if (valid)
		line->color = reference->color;

	return valid;
}

int applySwappedOptimization(struct stLine *line, struct stLine *reference) {

	// (sanity check)
	if (!isOptimizable(line) || isOptimizable(reference))
		return 0;

	int valid = line->pattern == 0x00
		? (line->color & 0x0f) == (reference->color >> 4)
		: (line->color >> 4) == (reference->color & 0x0f);

	if (valid) {
		line->pattern ^= 0xff;
		line->color = reference->color;
	}

	return valid;
}

int charsetProcessorWrite(struct stCharsetProcessor *this, struct stCharset *charset, FILE *chrFile, FILE *clrFile) {

	int i, j;
	struct stBlock *it;
	struct stLine *line;
	for (i = 0, it = charset->blocks; i < charset->blockCount; i++, it++) {
		for (j = 0, line = it->line; j < TILE_HEIGHT; j++, line++) {
			// Write value in output files
			if (fwrite(&(line->pattern), sizeof(byte), 1, chrFile) != 1)
				return 2;
			if (fwrite(&(line->color), sizeof(byte), 1, clrFile) != 1)
				return 3;
		}
	}

	return 0;
}

void charsetProcessorDone(struct stCharsetProcessor *this) {

	// nothing to do here
}

void charsetDone(struct stCharset *this) {

	if (this->blocks) free(this->blocks);
	this->blocks = NULL;
}

int isSolidBlock(struct stBlock *block, byte color) {

	int i;
	struct stLine *line;
	for (i = 0, line = block->line; i < TILE_HEIGHT; i++, line++) {
		if (((line->pattern) != 0xff) && ((line->color & 0x0f) != color))
			return 0;
		if (((line->pattern) != 0x00) && ((line->color & 0xf0) != (color << 4)))
			return 0;
	}
	return 1;
}

int isBlockEquals(struct stBlock *this, struct stBlock *that) {

	int i;
	struct stLine *thisLine, *thatLine;
	for (i = 0, thisLine = this->line, thatLine = that->line; i < TILE_HEIGHT; i++, thisLine++, thatLine++)
		if (!isLineEquals(thisLine, thatLine))
			return 0;

	return 1;
}

int blockIndex(struct stCharset *charset, struct stBlock *block, int stopBefore) {

	int i;
	struct stBlock *it;
	for (i = 0, it = charset->blocks; (i < charset->blockCount) && (i < stopBefore); i++, it++)
		if (isBlockEquals(block, it))
			return i;

	return -1;
}

/* Private function bodies ------------------------------------------------- */

int patternMode(int isForeground, char bit) {

	if ((bit < '0') || (bit > '7'))
		return PATTERN_MODE_UNSET;

	return (isForeground ? PATTERN_MODE_FOREGROUND : PATTERN_MODE_BACKGROUND)
		| (bit - '0');
}

int readLine(struct stCharsetProcessor *this, struct stLine *line, struct stBitmap *bitmap, int x, int y) {

	int i, j;
	byte colors[TILE_WIDTH];
	byte pattern = 0x00, fgcolor = 0xff, bgcolor = 0xff;

	// Read the colors
	for (i = 0; i < TILE_WIDTH; i++) {
		colors[i] = bitmapGet(bitmap, x + i, y);
	}

	// Create the pattern
	for (i = 0, j = TILE_WIDTH - 1; i < TILE_WIDTH; i++, j--) {
		byte color = colors[j];
		if (bgcolor == 0xff) {
			bgcolor = color;
			// pattern |= 0<<i; // (unnecesary)
		} else if (bgcolor == color) {
			// pattern |= 0<<i; // (unnecesary)
		} else if (fgcolor == 0xff) {
			fgcolor = color;
			pattern |= 1<<i;
		} else if (fgcolor == color) {
			pattern |= 1<<i;
		} else {
			// More than two colors
			printf("%s: Collision at %d,%d: %x%x%x%x%x%x%x%x\n",
				this->ignoreCollision ? "WARN" : "ERROR",
				x, y,
				colors[0], colors[1], colors[2], colors[3],
				colors[4], colors[5], colors[6], colors[7]);
			line->pattern = 0x00;
			line->color = 0x00;
			return 1;
		}
	}

	// Line read
	if (fgcolor == 0xff) fgcolor = 0;
	if (bgcolor == 0xff) bgcolor = 0;
	line->pattern = pattern;
	line->color = fgcolor << 4 | bgcolor;
	return 0;
}

void negateAndSwap(struct stLine *line) {

	line->pattern ^= 0xff;
	line->color = ((line->color & 0x0f) << 4) | (line->color >> 4);
}

int isLineEquals(struct stLine *thisLine, struct stLine *thatLine) {

	int i;
	for (i = 0; i < TILE_WIDTH; i++)
		if (colorAtBit(thisLine, i) != colorAtBit(thatLine, i))
			return 0;

	return 1;
}

byte colorAtBit(struct stLine *line, char bit) {

	return (line->pattern & (0x01 << bit))
		? (line->color >> 4)
		: (line->color & 0x0f);
}
