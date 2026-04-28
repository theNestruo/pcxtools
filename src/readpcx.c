/*
 * Support routines reading PCX files
 * Coded by theNestruo
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "readpcx.h"

/* Data structures --------------------------------------------------------- */

// PCX header container
typedef struct {
  uint8_t id;      // Must be 0x0a
  uint8_t version; // 0=2.5, 2=2.8+pal., 3=2.8+default pal. 4=Paintbrush, 5=3.0+
  uint8_t encoding; // Must be 1, 1=RLE
  uint8_t bpp;
  uint16_t xMin, yMin, xMax, yMax;
  uint16_t hPpp, vPpp;
  uint8_t palette[48]; // only if (bpp<=4), 16*3 groups
  uint8_t reserved;
  uint8_t planes; // Max. 4
  uint16_t bytesWidth;
  uint16_t paletteInformation;        // 1=color, 2=grayscale
  uint16_t deviceWidth, deviceHeight; // Paintbrush IV+
  uint8_t padding[54];                // Up to 128 bytes
} PcxHeader;

/* Private function prototypes --------------------------------------------- */

uint8_t autoPalette(uint8_t value);

/* Function bodies --------------------------------------------------------- */

int pcxReaderRead(FILE *file, Bitmap *bitmap) {

  if (!file)
    return 1;

  if (!bitmap)
    return 2;

  // Reads the header
  PcxHeader header;
  if (fread(&header, sizeof(PcxHeader), 1, file) != 1) {
    printf("ERROR: Could not read header.\n");
    return 3;
  }

  // Validate header
  if (header.id != 0x0a) {
    printf("ERROR: Wrong PCX signature. Found %d, expected %d.\n", header.id,
           0x0a);
    return 4;
  }
  if (header.bpp != 8) {
    printf("ERROR: Wrong PCX color depth. Found %dbpp, only %dbpp supported.\n",
           header.bpp, 8);
    return 5;
  }
  if (header.encoding != 1) {
    printf("ERROR: Wrong PCX econding. Found %d, expected %d.\n",
           header.encoding, 1);
    return 6;
  }

  // Allocate space for the bitmap
  bitmap->width = (int)header.xMax - (int)header.xMin + 1;
  bitmap->height = (int)header.yMax - (int)header.yMin + 1;
  bitmap->bitmap =
      (uint8_t *)calloc(bitmap->width * bitmap->height, sizeof(uint8_t));

  // Unpack the PCX
  unsigned int y, x;
  for (y = 0; y < bitmap->height; y++) {
    for (x = 0; x < header.bytesWidth;) {
      uint8_t data, runLength;

      // Read data uint8_t
      if (fread(&data, 1, 1, file) != 1) {
        printf("ERROR: Could not read uint8_t.\n");
        return 7;
      }
      if ((data & 0xc0) != 0xc0) {
        // Non packed
        runLength = 1;
      } else {
        // Packed: (data & 0x3f) times the next uint8_t
        runLength = data & 0x3f;
        if (fread(&data, 1, 1, file) != 1) {
          printf("ERROR: Could not read uint8_t.\n");
          return 8;
        }
      }

      // Unpack
      while (runLength--) {
        // (skip extra padding pixels)
        if (x < bitmap->width) {
          bitmap->bitmap[bitmap->width * y + x] = autoPalette(data);
        }
        x++;
      }
    }
  }

  return 0;
}

/* Private function bodies ------------------------------------------------- */

uint8_t autoPalette(uint8_t value) {

  return (value < 0x80)
             ? value
             : (0xff - value); // palette is backwards (PhotoShop quirk)
}
