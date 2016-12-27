/*
 * Support routines reading Tiled (TMX) files
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#ifndef READTMX_H_INCLUDED
#define READTMX_H_INCLUDED

#include "tiled.h"

/* Function prototypes ----------------------------------------------------- */

int tmxReaderRead(FILE *file, struct stTiled *tiled);

#endif // READTMX_H_INCLUDED
