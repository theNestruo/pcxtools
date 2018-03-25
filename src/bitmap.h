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

// struct stcolor {

	// byte r;
	// byte g;
	// byte b;
// };

struct stBitmap {
	// Bitmap container
	byte *bitmap;
	unsigned int width, height;
	
	// // Palette container
	// struct stcolor *palette;
	
	// Arguments and options
	int isFlip;
	int isMirror;
	byte (*reindexer) (byte);
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -i	inverted. Flips bitmap vertically
// -m	mirrored. Flips bitmap horizontally
// -ps	palette is backwards (PhotoShop quirk)
void bitmapOptions();

void bitmapInit(struct stBitmap *instance, int argc, char **argv);

byte bitmapGet(struct stBitmap *instance, int x, int y);

void bitmapDone(struct stBitmap *instance);

#endif // BITMAP_H_INCLUDED
