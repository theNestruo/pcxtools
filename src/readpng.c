/*
 * Support routines reading PNG files
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "readpng.h"

#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_CPP
#define LODEPNG_COMPILE_DISK
#include "lodepng\lodepng.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short int word;
#endif

/* Data structures --------------------------------------------------------- */

struct stColor {

	// The RGBA values
	byte r;
	byte g;
	byte b;
	byte a;

	// The assigned index
	byte index;
};

/* Constant values --------------------------------------------------------- */

struct stColor palette[] = {
		{ 0x00, 0x00, 0x00, 0xff,  1 }, // black		TMS9918	V9938
		{ 0x23, 0xcb, 0x32, 0xff,  2 }, // medium green	TMS9918
		{ 0x24, 0xda, 0x24, 0xff,  2 }, // medium green			V9938
		{ 0x60, 0xdd, 0x6c, 0xff,  3 }, // light green	TMS9918
		{ 0x6d, 0xff, 0x6d, 0xff,  3 }, // light green			V9938
		{ 0x54, 0x4e, 0xff, 0xff,  4 }, // dark blue	TMS9918
		{ 0x24, 0x24, 0xff, 0xff,  4 }, // dark blue			V9938
		{ 0x7d, 0x70, 0xff, 0xff,  5 }, // light blue	TMS9918
		{ 0x48, 0x6d, 0xff, 0xff,  5 }, // light blue			V9938
		{ 0xd2, 0x54, 0x42, 0xff,  6 }, // dark red		TMS9918
		{ 0xb6, 0x24, 0x24, 0xff,  6 }, // dark red				V9938
		{ 0x45, 0xe8, 0xff, 0xff,  7 }, // cyan			TMS9918
		{ 0x48, 0xda, 0xff, 0xff,  7 }, // cyan					V9938
		{ 0xfa, 0x59, 0x48, 0xff,  8 }, // medium red	TMS9918
		{ 0xff, 0x24, 0x24, 0xff,  8 }, // medium red			V9938
		{ 0xff, 0x7c, 0x6c, 0xff,  9 }, // light red	TMS9918
		{ 0xff, 0x6d, 0x6d, 0xff,  9 }, // light red			V9938
		{ 0xd3, 0xc6, 0x3c, 0xff, 10 }, // dark yellow	TMS9918
		{ 0xda, 0xda, 0x24, 0xff, 10 }, // dark yellow			V9938
		{ 0xe5, 0xd2, 0x6d, 0xff, 11 }, // light yellow	TMS9918
		{ 0xda, 0xda, 0x91, 0xff, 11 }, // light yellow			V9938
		{ 0x23, 0xb2, 0x2c, 0xff, 12 }, // dark green	TMS9918
		{ 0x24, 0x91, 0x24, 0xff, 12 }, // dark green			V9938
		{ 0xc8, 0x5a, 0xc6, 0xff, 13 }, // magenta		TMS9918
		{ 0xda, 0x48, 0xb6, 0xff, 13 }, // magenta				V9938
		{ 0xcc, 0xcc, 0xcc, 0xff, 14 }, // gray			TMS9918
		{ 0xb6, 0xb6, 0xb6, 0xff, 14 }, // gray					V9938
		{ 0xff, 0xff, 0xff, 0xff, 15 }, // white		TMS9918	V9938

		{ 0x40, 0x40, 0x40, 0xff,  0 }, // dark grey as transparent
		{ 0x00, 0x00, 0x00, 0x00,  0 }  // actually transparent
	};

/* Private function prototypes --------------------------------------------- */

byte paletteIndex(byte r, byte g, byte b, byte a);
int distance(struct stColor *color, byte r, byte g, byte b, byte a);

/* Function bodies --------------------------------------------------------- */

int pngReaderRead(char *pngFilename, struct stBitmap *bitmap) {

	if (!pngFilename)
		return 1;

	if (!bitmap)
		return 2;

	// Reads the PNG file (as 32-bit RGBA raw)
	byte *pngImage = 0;
	unsigned int pngWidth, pngHeight;
	unsigned int pngError = lodepng_decode32_file(&pngImage, &pngWidth, &pngHeight, pngFilename);
	if (pngError) {
		printf("ERROR: Could not read PNG: %u: %s\n", pngError, lodepng_error_text(pngError));
		return 2 + pngError;
	}

	// Allocate space for the bitmap
	bitmap->width = pngWidth;
	bitmap->height = pngHeight;
	bitmap->bitmap = (byte*) calloc(bitmap->width * bitmap->height, sizeof(byte));

	// Populates the bitmap
	int y, x;
	byte *source, *target;
	for (source = pngImage, target = bitmap->bitmap, y = 0; y < bitmap->height; y++) {
		for (x = 0; x < bitmap->width; x++) {
			byte r = *(source++);
			byte g = *(source++);
			byte b = *(source++);
			byte a = *(source++);
			*(target++) = paletteIndex(r, g, b, a);
		}
	}

// out:
	// Exit gracefully
	if (pngImage) free(pngImage);
	return 0;
}

/* Private function bodies ------------------------------------------------- */

byte paletteIndex(byte r, byte g, byte b, byte a) {

	int i, n;
	struct stColor *it;
	struct stColor *closestColor = 0;
	int minDistance = 255*4;
	for (i = 0, n = 30, it = palette; i < n; i++, it++) {
		int dist = distance(it, r, g, b, a);
		if (dist == 0) {
			return it->index;
		}
		if (dist < minDistance) {
			closestColor = it;
			minDistance = dist;
		}
	}
	return closestColor->index;
}

int distance(struct stColor *color, byte r, byte g, byte b, byte a) {

	if ((color->a == 0x00) && (a == 0x00)) {
		return 0;
	}

	return    abs((int) color->r - (int) r)
			+ abs((int) color->g - (int) g)
			+ abs((int) color->b - (int) b)
			+ abs((int) color->a - (int) a);
}

