/*
 * Support routines for managing sprites in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "args.h"
#include "sprite.h"

/* Private function prototypes --------------------------------------------- */

void processSpriteGroup(struct stSprWriter *instance, struct stSpriteGroup *group, struct stBitmap *bitmap, int x, int y);

/* Function bodies --------------------------------------------------------- */

void sprWriterOptions() {

	printf("\t-8\tgenerate 8x8px sprites\n");
	printf("\t-h\tgenerate half sprites (8x16px, 16b per sprite)\n");
}

void sprWriterInit(struct stSprWriter *this, int argc, char **argv) {

	// Init
	this->groups = NULL;
	this->groupCount = 0;
	this->spriteWidth = 16;
	this->spriteHeight = 16;

	// Read arguments
	if (argEquals(argc, argv, "-8") != -1) {
		this->spriteWidth = 8;
		this->spriteHeight = 8;
	} else if (argEquals(argc, argv, "-h") != -1) {
		this->spriteWidth = 8;
	}
}

void sprWriterReadSprites(struct stSprWriter *this, struct stBitmap *bitmap) {

	this->groupCount = ((int) (bitmap->width / this->spriteWidth)) * ((int) (bitmap->height / this->spriteHeight));
	this->groups = (struct stSpriteGroup*) calloc(this->groupCount, sizeof(struct stSpriteGroup));

	int y, x;
	struct stSpriteGroup *it;
	for (y = 0, it = this->groups; (y + this->spriteHeight) <= bitmap->height; y += this->spriteHeight) {
		for (x = 0; (x + this->spriteWidth) <= bitmap->width; x += this->spriteWidth, it++) {
			processSpriteGroup(this, it, bitmap, x, y);
		}
	}
}

int sprWriterWrite(struct stSprWriter *this, FILE *sprFile) {

	int spriteSize = (this->spriteWidth / 8) * this->spriteHeight;

	// For each group...
	int i;
	struct stSpriteGroup *it;
	for (i = 0, it = this->groups; i < this->groupCount; i++, it++) {
	
		// For each sprite...
		int j;
		struct stSprite *sprite;
		for (j = 0, sprite = it->sprites; j < it->spriteCount; j++, sprite++) {
			if (fwrite(sprite->pattern, sizeof(byte), spriteSize, sprFile) != spriteSize)
				return 2;
		}
	}
	return 0;
}

void sprWriterDone(struct stSprWriter *this) {

	if (!this->groups)
		return;
	
	// For each group...
	int i;
	struct stSpriteGroup *it;
	for (i = 0, it = this->groups; i < this->groupCount; i++, it++) {
		if (!it->sprites) continue;
	
		// For each sprite...
		int j;
		struct stSprite *sprite;
		for (j = 0, sprite = it->sprites; j < it->spriteCount; j++, sprite++) {
			if (sprite->pattern)
				free(sprite->pattern);
		}
		free(it->sprites);
	}
	free(this->groups);
	this->groups = NULL;
}

/* Private function bodies ------------------------------------------------- */

void processSpriteGroup(struct stSprWriter *this, struct stSpriteGroup *group, struct stBitmap *bitmap, int x0, int y0) {

	int spriteSize = (this->spriteWidth / 8) * this->spriteHeight;
	
	// Create buffer and reset values
	int i, j;
	struct stSprite *buffer = (struct stSprite*) calloc(15, sizeof(struct stSprite));
	for(i = 0; i < 15; i++) {
		buffer[i].pattern = (byte*) calloc(spriteSize, sizeof(byte));
	}
	
	// Create the pattern
	int col, y, x;
	for (col = 0, j = 0; col < this->spriteWidth; col += 8) { // for each 8px width column
		for (y = 0; y < this->spriteHeight; y++, j++) { // for each line
			for (x = 0; x < 8; x++) { // for each pixel
				byte color = bitmapGet(bitmap, x0 + col + x, y0 + y);
				if (color)
					buffer[color - 1].pattern[j] |= (0x80 >> x);
			}
		}
	}
	
	// Count patterns
	group->spriteCount = 0;
	for (i = 0; i < 15; i++) {
		for (j = 0; j < spriteSize; j++) {
			if (buffer[i].pattern[j]) {
				group->spriteCount++;
				break;
			}
		}
	}
	
	// Blit patterns
	if (!group->spriteCount) {
		group->sprites = NULL;
		
	} else {
		group->sprites = (struct stSprite*) calloc(group->spriteCount, sizeof(struct stSprite));
		struct stSprite *src, *dest;
		for (i = 0, src = buffer, dest = group->sprites; i < 15; i++, src++) {
			int spriteFound = 0;
			for (j = 0; (j < spriteSize) && (!spriteFound); j++) {
				spriteFound |= src->pattern[j];
			}
			if (!spriteFound) continue;
			// blit sprite
			dest->pattern = (byte*) calloc(spriteSize, sizeof(byte));
			for (j = 0; j < spriteSize; j++) {
				dest->pattern[j] = src->pattern[j];
			}
			dest++;
		}
	}
	
	// Destroys buffer
	for (i = 0; i < 15; i++) {
		free(buffer[i].pattern);
	}
	free(buffer);
}
