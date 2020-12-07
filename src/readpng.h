/*
 * Support routines reading PNG files
 * Coded by theNestruo
 */

#ifndef READPNG_H_INCLUDED
#define READPNG_H_INCLUDED

#include "bitmap.h"

/* Function prototypes ----------------------------------------------------- */

int pngReaderRead(char *pngFilename, struct stBitmap *bitmap);

#endif // READPNG_H_INCLUDED
