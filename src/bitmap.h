/*
 * Support routines for bitmap
 * Coded by theNestruo
 */

#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

struct stBitmap {
	// Bitmap container
	byte *bitmap;
	unsigned int width, height;

	// Arguments and options
	int isFlip;
	int isMirror;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -i	inverted. Flips bitmap vertically
// -m	mirrored. Flips bitmap horizontally
void bitmapOptions();

void bitmapInit(struct stBitmap *instance, int argc, char **argv);

byte bitmapGet(struct stBitmap *instance, int x, int y);

void bitmapDone(struct stBitmap *instance);

#endif // BITMAP_H_INCLUDED
