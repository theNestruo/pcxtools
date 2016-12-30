/*
 * Support routines for Tiled (TMX)
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#ifndef TILED_H_INCLUDED
#define TILED_H_INCLUDED

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Data structures --------------------------------------------------------- */

// Tiled (TMX) container
struct stTiled {
	byte *data;
	int width;
	int height;
	
	// Arguments and options
	// int isFlip; // (not yet implemented)
	// int isMirror; // (not yet implemented)
	int metatileSize;
};

/* Function prototypes ----------------------------------------------------- */

// Supported arguments:
// -i		inverted. Flips bitmap vertically // (not yet implemented)
// -m		mirrored. Flips bitmap horizontally // (not yet implemented)
// -t<0..255>	reorganize data as metatiles of <0..255>x<0..255> bytes
void tiledOptions();

void tiledInit(struct stTiled *instance, int argc, char **argv);

int tiledWrite(struct stTiled *instance, FILE *binFile);

void tiledDone(struct stTiled *instance);

#endif // BITMAP_H_INCLUDED
