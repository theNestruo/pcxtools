/*
 * Support routines reading PCX files
 * Coded by theNestruo
 */

#ifndef READPCX_H_INCLUDED
#define READPCX_H_INCLUDED

#include "bitmap.h"

/* Function prototypes ----------------------------------------------------- */

int pcxReaderRead(FILE *file, struct stBitmap *bitmap);

#endif // READPCX_H_INCLUDED
