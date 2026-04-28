/*
 * Support routines for managing sprites in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#pragma once

#include <stdint.h>

#include "bitmap.h"

/* Data structures --------------------------------------------------------- */

typedef struct {
	uint8_t *pattern;
} Sprite;

typedef struct {
	Sprite *sprites;
	int spriteCount;
} SpriteGroup;

typedef struct {
	// Data container
	SpriteGroup *groups;
	int groupCount;

	// Arguments
	int spriteWidth;
	int spriteHeight;
	int colorMode;
	int traverseHorizontally;
} SprWriter;

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -8	generate 8x8px sprites
// -h	generate half sprites (8x16px, 16b per sprite)
// -lh  higher colors will have higher priority planes (default)
// -hl  lower colors will have higher priority planes
// -ld	lighter colors will have higher priority planes
// -dl	darker colors will have higher priority planes
// -th  traverse spritesheet horizontally, then vertically (default)
// -tv  traverse spritesheet vertically, then horizontally
void sprWriterOptions();

void sprWriterInit(SprWriter *instance, int argc, char **argv);

void sprWriterReadSprites(SprWriter *instance, Bitmap *bitmap);

int sprWriterWrite(SprWriter *instance, FILE *sprFile);

void sprWriterDone(SprWriter *instance);

