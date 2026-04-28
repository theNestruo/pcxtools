/*
 * Support routines for bitmap
 * Coded by theNestruo
 */

#pragma once

#include <stdint.h>

/* Data structures --------------------------------------------------------- */

typedef struct {
	// Bitmap container
	uint8_t *bitmap;
	unsigned int width, height;

	// Arguments and options
	int isFlip;
	int isMirror;
} Bitmap;

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -i	inverted. Flips bitmap vertically
// -m	mirrored. Flips bitmap horizontally
void bitmapOptions();

void bitmapInit(Bitmap *instance, int argc, char **argv);

uint8_t bitmapGet(Bitmap *instance, int x, int y);

void bitmapDone(Bitmap *instance);

