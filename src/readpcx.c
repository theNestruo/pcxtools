/*
 * Support routines reading PCX files
 * Coded by theNestruo
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "args.h"
#include "readpcx.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short int word;
#endif

/* Data structures --------------------------------------------------------- */

// PCX header container
struct stPcxHeader {
	byte id; // Must be 0x0a
	byte version; // 0=2.5, 2=2.8+pal., 3=2.8+default pal. 4=Paintbrush, 5=3.0+
	byte encoding; // Must be 1, 1=RLE
	byte bpp;
	word xMin, yMin, xMax, yMax;
	word hPpp, vPpp;
	byte palette[48]; // only if (bpp<=4), 16*3 groups
	byte reserved;
	byte planes; // Max. 4
	word bytesWidth;
	word paletteInformation; // 1=color, 2=grayscale
	word deviceWidth, deviceHeight; // Paintbrush IV+
	byte padding[54]; // Up to 128 bytes
};

/* Function bodies --------------------------------------------------------- */

int pcxReaderRead(FILE *file, struct stBitmap *bitmap) {

	if (!file)
		return 1;
		
	if (!bitmap)
		return 2;
	
	// Reads the header
	struct stPcxHeader header;
	if (fread(&header, sizeof(struct stPcxHeader), 1, file) != 1) {
		printf("ERROR: Could not read header.\n");
		return 3;
	}
	
	// Validate header
	if (header.id != 0x0a) {
		printf("ERROR: Wrong PCX signature. Found %d, expected %d.\n", header.id, 0x0a);
		return 4;
	}
	if (header.bpp != 8) {
		printf("ERROR: Wrong PCX color depth. Found %dbpp, only %dbpp supported.\n", header.bpp, 8);
		return 5;
	}
	if (header.encoding != 1) {
		printf("ERROR: Wrong PCX econding. Found %d, expected %d.\n", header.encoding, 1);
		return 6;
	}
	
	// Allocate space for the bitmap
	bitmap->width = (int) header.xMax - (int) header.xMin + 1;
	bitmap->height = (int) header.yMax - (int) header.yMin + 1;
	bitmap->bitmap = (byte*) calloc(bitmap->width * bitmap->height, sizeof(byte));
	
	// Unpack the PCX
	int y, x;
	for (y = 0; y < bitmap->height; y++) {
		for (x = 0; x < header.bytesWidth; ) {
			byte data, runLength;
			
			// Read data byte
			if (fread(&data, 1, 1, file) != 1) {
				printf("ERROR: Could not read byte.\n");
				return 7;
			}
			if ((data & 0xc0) != 0xc0) {
				// Non packed
				runLength = 1;
			} else {
				// Packed: (data & 0x3f) times the next byte
				runLength = data & 0x3f;
				if (fread(&data, 1, 1, file) != 1) {
					printf("ERROR: Could not read byte.\n");
					return 8;
				}
			}
			
			// Unpack
			while (runLength--) {
				// (skip extra padding pixels)
				if (x < bitmap->width) {
					bitmap->bitmap[bitmap->width * y + x] = data;
				}
				x++;
			}
		}
	}
	
	// // VGA palette ID
	// byte paletteId;
	// if (fread(&paletteId, 1, 1, file) != 1) {
		// printf("ERROR: Could not read VGA palette ID.\n");
		// return 9;
	// }
	// if (paletteId != 0x0c) {
		// printf("ERROR: Wrong VGA palette ID. Found %d, expected %d.\n", paletteId, 0x0c);
		// return 10;
	// }
	
	// // VGA palette
	// bitmap->palette = (struct stcolor*) malloc(16 * sizeof(struct stcolor));
	// if (fread(bitmap->palette, sizeof(struct stcolor), 16, file) != 16) {
		// printf("ERROR: Could not read palette.\n");
		// return 11;
	// }
	
	return 0;
}
