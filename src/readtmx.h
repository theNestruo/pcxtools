/*
 * Support routines reading Tiled (TMX) files
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#pragma once

#include "tiled.h"

/* Data structures --------------------------------------------------------- */

// Tiled (TMX) reader
typedef struct {
	// Arguments and options
	int isMultibankCharset;
} TmxReader;

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -b		multibank charset support
void tmxReaderOptions();

void tmxReaderInit(TmxReader *instance, int argc, char **argv);

int tmxReaderRead(TmxReader *instance, FILE *file, Tiled *tiled);

