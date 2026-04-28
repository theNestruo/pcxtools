/*
 * Support routines for Tiled (TMX)
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#pragma once

#include <stdint.h>

/* Data structures --------------------------------------------------------- */

// Tiled (TMX) container
typedef struct {
	uint8_t *data;
	int width;
	int height;

	// Arguments and options
	// int isFlip; // (not yet implemented)
	// int isMirror; // (not yet implemented)
	int metatileSize;
} Tiled;

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -i		inverted. Flips bitmap vertically // (not yet implemented)
// -m		mirrored. Flips bitmap horizontally // (not yet implemented)
// -t<0..255>	reorganize data as metatiles of <0..255>x<0..255> bytes
void tiledOptions();

void tiledInit(Tiled *instance, int argc, char **argv);

int tiledWrite(Tiled *instance, FILE *binFile);

void tiledDone(Tiled *instance);
