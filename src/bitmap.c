/*
 * Support routines for bitmap
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "bitmap.h"
#include "args.h"

/* Constant values --------------------------------------------------------- */

// struct stcolor paletteTMS9918[] = {
		// {   0,   0,   0 }, // black
		// {  35, 203,  50 }, // medium green
		// {  96, 221, 108 }, // light green
		// {  84,  78, 255 }, // dark blue
		// { 125, 112, 255 }, // light blue
		// { 210,  84,  66 }, // dark red
		// {  69, 232, 255 }, // cyan
		// { 250,  89,  72 }, // medium red
		// { 255, 124, 108 }, // light red
		// { 211, 198,  60 }, // dark yellow
		// { 229, 210, 109 }, // light yellow
		// {  35, 178,  44 }, // dark green
		// { 200,  90, 198 }, // magenta
		// { 204, 204, 204 }, // gray
		// { 255, 255, 255 }  // white
	// };
	
// struct stcolor paletteV9938[] = {
		// {  0,   0,   0 }, // black
		// { 36, 218,  36 }, // medium green
		// {109, 255, 109 }, // light green
		// { 36,  36, 255 }, // dark blue
		// { 72, 109, 255 }, // light blue
		// {182,  36,  36 }, // dark red
		// { 72, 218, 255 }, // cyan
		// {255,  36,  36 }, // medium red
		// {255, 109, 109 }, // light red
		// {218, 218,  36 }, // dark yellow
		// {218, 218, 145 }, // light yellow
		// { 36, 145,  36 }, // dark green
		// {218,  72, 182 }, // magenta
		// {182, 182, 182 }, // gray
		// {255, 255, 255 }  // white
	// };

/* Private function prototypes --------------------------------------------- */

byte autoPalette(byte value);

/* Function bodies --------------------------------------------------------- */

void bitmapOptions() {

	printf("\t-i\tinverted. Flips bitmap vertically\n");
	printf("\t-m\tmirrored. Flips bitmap horizontally\n");
	// printf("\t-p1\treindex palette by RGB values, matching MSX1 (TMS9918)\n");
	// printf("\t-p2\treindex palette by RGB values, matching MSX2 (V9938)\n");
}

void bitmapInit(struct stBitmap *this, int argc, char **argv) {

	// Init
	this->bitmap = NULL;
	this->width = this->height = 0;
	this->isFlip = this->isMirror = 0;
	this->reindexer = autoPalette;

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
	index = this->reindexer(index);
	if (index > 0x0f) {
		printf("WARN: Palette index greater than %d found at %d,%d: %x\n",
			0x0f, x0, y0, index);
		return 0;
	}
	return index;
}

void bitmapDone(struct stBitmap *this) {

	if (this->bitmap) free(this->bitmap);
	// if (this->palette) free(this->palette);
	this->bitmap = NULL;
}

/* Private function bodies ------------------------------------------------- */

byte autoPalette(byte value) {

	return (value < 0x80)
		? value
		: (0xff - value); // palette is backwards (PhotoShop quirk)
}
