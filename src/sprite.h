/*
 * Support routines for managing sprites in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED

#include "bitmap.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

struct stSprite {
	byte *pattern;
};

struct stSpriteGroup {
	struct stSprite *sprites;
	int spriteCount;
};

struct stSprWriter {
	// Data container
	struct stSpriteGroup *groups;
	int groupCount;
	
	// Arguments
	int spriteWidth;
	int spriteHeight;
	int colorOrder;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -8	generate 8x8px sprites
// -h	generate half sprites (8x16px, 16b per sprite
void sprWriterOptions();

void sprWriterInit(struct stSprWriter *instance, int argc, char **argv);

void sprWriterReadSprites(struct stSprWriter *instance, struct stBitmap *bitmap);

int sprWriterWrite(struct stSprWriter *instance, FILE *sprFile);

void sprWriterDone(struct stSprWriter *instance);

#endif // SPRITE_H_INCLUDED
