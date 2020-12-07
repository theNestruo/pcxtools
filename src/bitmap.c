/*
 * Support routines for bitmap
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "args.h"

/* Function bodies --------------------------------------------------------- */

void bitmapOptions() {

	printf("\t-i\tinverted. Flips bitmap vertically\n");
	printf("\t-m\tmirrored. Flips bitmap horizontally\n");
}

void bitmapInit(struct stBitmap *this, int argc, char **argv) {

	// Init
	this->bitmap = NULL;
	this->width = this->height = 0;
	this->isFlip = this->isMirror = 0;

	// Read arguments
	this->isFlip = (argEquals(argc, argv, "-i") != -1);
	this->isMirror = (argEquals(argc, argv, "-m") != -1);
}

byte bitmapGet(struct stBitmap *this, int x0, int y0) {

	if ((x0 < 0) || (y0 < 0) || (x0 >= this->width) || (y0 >= this->height))
		return 0;

	int y = this->isFlip ? this->height - y0 - 1 : y0;
	int x = this->isMirror ? this->width - x0 - 1 : x0;

	byte index = this->bitmap[x + y * this->width];
	if (index > 0x0f) {
		printf("WARN: Palette index greater than %d found at %d,%d: %x\n",
			0x0f, x0, y0, index);
		return 0;
	}
	return index;
}

void bitmapDone(struct stBitmap *this) {

	if (this->bitmap) free(this->bitmap);
	this->bitmap = NULL;
}
