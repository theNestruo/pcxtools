/*
 * Support routines reading PNG files
 * Coded by theNestruo
 */

#ifndef READPNG_H_INCLUDED
#define READPNG_H_INCLUDED

#include "bitmap.h"

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -e	index color by euclidean distance (default)
// -g	index color by weighted distance
void pngReaderOptions();

void pngReaderInit(int argc, char **argv);

int pngReaderRead(char *pngFilename, struct stBitmap *bitmap);

#endif // READPNG_H_INCLUDED
