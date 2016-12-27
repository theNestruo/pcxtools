/*
 * Support routines for Tiled (TMX)
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "tiled.h"
#include "args.h"

/* Constant values --------------------------------------------------------- */

/* Private function prototypes --------------------------------------------- */

/* Function bodies --------------------------------------------------------- */

void tiledOptions() {

	// printf("\t-i\tinverted. Flips bitmap vertically\n");
	// printf("\t-m\tmirrored. Flips bitmap horizontally\n");
	// printf("\t-1..f\treorganize data as metatiles of <1..f>x<1..f> bytes (hexa)\n");
	// printf("\t\totherwise, generate straightforward (default)\n");
}

void tiledInit(struct stTiled *this, int argc, char **argv) {

	// Init
	this->data= NULL;
	this->width = this->height = 0;

	// Read arguments
	// this->isFlip = (argEquals(argc, argv, "-i") != -1);
	// this->isMirror = (argEquals(argc, argv, "-m") != -1);
}

int tiledWrite(struct stTiled *this, FILE *binFile) {
	
	return (fwrite(this->data, this->width * this->height, 1, binFile) != 1)
		? 1
		: 0;
}

void tiledDone(struct stTiled *this) {

	if (this->data) free(this->data);
	this->data = NULL;
}

/* Private function bodies ------------------------------------------------- */
