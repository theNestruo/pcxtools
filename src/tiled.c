/*
 * Support routines for Tiled (TMX)
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "tiled.h"

/* Constant values --------------------------------------------------------- */

/* Private function prototypes --------------------------------------------- */

/* Function bodies --------------------------------------------------------- */

void tiledOptions() {

	// printf("\t-i\tinverted. Flips bitmap vertically\n"); // (not yet implemented)
	// printf("\t-m\tmirrored. Flips bitmap horizontally\n"); // (not yet implemented)
	printf("\t-t<0..255>\treorganize data as metatiles of <0..255>x<0..255> bytes\n");
}

void tiledInit(struct stTiled *this, int argc, char **argv) {

	// Init
	this->data= NULL;
	this->width = this->height = 0;
	this->metatileSize = 1;

	// Read arguments
	// this->isFlip = (argEquals(argc, argv, "-i") != -1); // (not yet implemented)
	// this->isMirror = (argEquals(argc, argv, "-m") != -1); // (not yet implemented)

	int i;
	if ((i = argStartsWith(argc, argv, "-t", 2)) != -1) {
		this->metatileSize = decimalInt(&(argv[i][2]));
	}
}

int tiledWrite(struct stTiled *this, FILE *binFile) {

	// No rearrangment
	if (this->metatileSize == 1) {
		return (fwrite(this->data, this->width * this->height, 1, binFile) != 1)
			? 1
			: 0;
	}

	// Metatile rearrangment or mirrored
	int x, xMax, y, yMax, v;
	byte *groupSrc, *tileSrc, *src;
	for (y = 0, yMax = (int) (this->height / this->metatileSize), groupSrc = this->data;
			y < yMax;
			y++, groupSrc += this->width * this->metatileSize) {
		for (x = 0, xMax = (int) (this->width / this->metatileSize), tileSrc = groupSrc;
				x < xMax;
				x++, tileSrc += this->metatileSize) {
			for (v = 0, src = tileSrc;
					v < this->metatileSize;
					v++, src += this->width) {
				if (fwrite(src, this->metatileSize, 1, binFile) != 1) {
					return 2;
				}
			}
		}
	}
	return 0;
}

void tiledDone(struct stTiled *this) {

	if (this->data) free(this->data);
	this->data = NULL;
}

/* Private function bodies ------------------------------------------------- */
