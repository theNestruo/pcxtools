/*
 * Support routines reading Tiled (TMX) files
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#ifndef READTMX_H_INCLUDED
#define READTMX_H_INCLUDED

#include "tiled.h"

/* Data structures --------------------------------------------------------- */

// Tiled (TMX) reader
struct stTmxReader {
	// Arguments and options
	int isMultibankCharset;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -b		multibank charset support
void tmxReaderOptions();

void tmxReaderInit(struct stTmxReader *instance, int argc, char **argv);

int tmxReaderRead(struct stTmxReader *instance, FILE *file, struct stTiled *tiled);

#endif // READTMX_H_INCLUDED
