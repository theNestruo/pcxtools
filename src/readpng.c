/*
 * Support routines reading PNG files
 * Coded by theNestruo
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "readpng.h"

#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_CPP
#define LODEPNG_COMPILE_DISK
#include "lodepng/lodepng.h"

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short int word;
#endif

/* Data structures --------------------------------------------------------- */

struct stColor {

	// The RGBA values
	byte r;
	byte g;
	byte b;
	byte a;

	// The assigned index
	byte index;
};

/* Constant values --------------------------------------------------------- */

const int paletteLength = 15 + 3;

struct stColor meiseiTms9218Palette[] = {
			// transparent values
			{ 0x00, 0x00, 0x00, 0x00,  0 }, // actually transparent
			{ 0xFF, 0x00, 0xFF, 0xFF,  0 }, // fuchsia
			{ 0x40, 0x40, 0x40, 0xFF,  0 }, // dark grey (PCX2MSX reference files)
			// TI TMS9219 palette, from hap's meisei emulator
			{ 0x00, 0x00, 0x00, 0xFF,  1 }, // black
			{ 0x23, 0xCB, 0x32, 0xFF,  2 }, // medium green
			{ 0x60, 0xDD, 0x6C, 0xFF,  3 }, // light green
			{ 0x54, 0x4E, 0xFF, 0xFF,  4 }, // dark blue
			{ 0x7D, 0x70, 0xFF, 0xFF,  5 }, // light blue
			{ 0xD2, 0x54, 0x42, 0xFF,  6 }, // dark red
			{ 0x45, 0xE8, 0xFF, 0xFF,  7 }, // cyan
			{ 0xFA, 0x59, 0x48, 0xFF,  8 }, // medium red
			{ 0xFF, 0x7C, 0x6C, 0xFF,  9 }, // light red
			{ 0xD3, 0xC6, 0x3C, 0xFF, 10 }, // dark yellow
			{ 0xE5, 0xD2, 0x6D, 0xFF, 11 }, // light yellow
			{ 0x23, 0xB2, 0x2C, 0xFF, 12 }, // dark green
			{ 0xC8, 0x5A, 0xC6, 0xFF, 13 }, // magenta
			{ 0xCC, 0xCC, 0xCC, 0xFF, 14 }, // gray
			{ 0xFF, 0xFF, 0xFF, 0xFF, 15 }  // white
		};

struct stColor meiseiV9938Palette[] = {
			// transparent values
			{ 0x00, 0x00, 0x00, 0x00,  0 }, // actually transparent
			{ 0xFF, 0x00, 0xFF, 0xFF,  0 }, // fuchsia
			{ 0x40, 0x40, 0x40, 0xFF,  0 }, // dark grey (PCX2MSX reference files)
			// V9938 palette, from hap's meisei emulator
			{ 0x00, 0x00, 0x00, 0xFF,  1 }, // black
			{ 0x24, 0xDA, 0x24, 0xFF,  2 }, // medium green
			{ 0x6D, 0xFF, 0x6D, 0xFF,  3 }, // light green
			{ 0x24, 0x24, 0xFF, 0xFF,  4 }, // dark blue
			{ 0x48, 0x6D, 0xFF, 0xFF,  5 }, // light blue
			{ 0xB6, 0x24, 0x24, 0xFF,  6 }, // dark red
			{ 0x48, 0xDA, 0xFF, 0xFF,  7 }, // cyan
			{ 0xFF, 0x24, 0x24, 0xFF,  8 }, // medium red
			{ 0xFF, 0x6D, 0x6D, 0xFF,  9 }, // light red
			{ 0xDA, 0xDA, 0x24, 0xFF, 10 }, // dark yellow
			{ 0xDA, 0xDA, 0x91, 0xFF, 11 }, // light yellow
			{ 0x24, 0x91, 0x24, 0xFF, 12 }, // dark green
			{ 0xDA, 0x48, 0xB6, 0xFF, 13 }, // magenta
			{ 0xB6, 0xB6, 0xB6, 0xFF, 14 }, // gray
			{ 0xFF, 0xFF, 0xFF, 0xFF, 15 }  // white
		};

struct stColor reidracToshibaPalette[] = {
			// transparent values
			{ 0x00, 0x00, 0x00, 0x00,  0 }, // actually transparent
			{ 0xFF, 0x00, 0xFF, 0xFF,  0 }, // fuchsia
			{ 0x40, 0x40, 0x40, 0xFF,  0 }, // dark grey (PCX2MSX reference files)
			// TOSHIBA palette, from reidrac's MSX Pixel Tools
			// (https://github.com/reidrac/msx-pixel-tools)
			{    0,    0,    0, 0xFF,  1 }, // black
			{  102,  204,  102, 0xFF,  2 }, // medium green
			{  136,  238,  136, 0xFF,  3 }, // light green
			{   68,   68,  221, 0xFF,  4 }, // dark blue
			{  119,  119,  255, 0xFF,  5 }, // light blue
			{  187,   85,   85, 0xFF,  6 }, // dark red
			{  119,  221,  221, 0xFF,  7 }, // cyan
			{  221,  102,  102, 0xFF,  8 }, // medium red
			{  255,  119,  119, 0xFF,  9 }, // light red
			{  204,  204,   85, 0xFF, 10 }, // dark yellow
			{  238,  238,  136, 0xFF, 11 }, // light yellow
			{   85,  170,   85, 0xFF, 12 }, // dark green
			{  187,   85,  187, 0xFF, 13 }, // magenta
			{  204,  204,  204, 0xFF, 14 }, // gray
			{  238,  238,  238, 0xFF, 15 }  // white
		};

struct stColor wikipediaTms9918Palette[] = {
			// transparent values
			{ 0x00, 0x00, 0x00, 0x00,  0 }, // actually transparent
			{ 0xFF, 0x00, 0xFF, 0xFF,  0 }, // fuchsia
			{ 0x40, 0x40, 0x40, 0xFF,  0 }, // dark grey (PCX2MSX reference files)
			// TI TMS9918 palette, according Wikipedia
			// (https://en.wikipedia.org/wiki/Texas_Instruments_TMS9918#Colors)
			{ 0x00, 0x00, 0x00, 0xFF,  1 }, // black
			{ 0x0A, 0xAD, 0x1E, 0xFF,  2 }, // medium green
			{ 0x34, 0xC8, 0x4C, 0xFF,  3 }, // light green
			{ 0x2B, 0x2D, 0xE3, 0xFF,  4 }, // dark blue
			{ 0x51, 0x4B, 0xFB, 0xFF,  5 }, // light blue
			{ 0xBD, 0x29, 0x25, 0xFF,  6 }, // dark red
			{ 0x1E, 0xE2, 0xEF, 0xFF,  7 }, // cyan
			{ 0xFB, 0x2C, 0x2B, 0xFF,  8 }, // medium red
			{ 0xFF, 0x5F, 0x4C, 0xFF,  9 }, // light red
			{ 0xBD, 0xA2, 0x2B, 0xFF, 10 }, // dark yellow
			{ 0xD7, 0xB4, 0x54, 0xFF, 11 }, // light yellow
			{ 0x0A, 0x8C, 0x18, 0xFF, 12 }, // dark green
			{ 0xAF, 0x32, 0x9A, 0xFF, 13 }, // magenta
			{ 0xB2, 0xB2, 0xB2, 0xFF, 14 }, // gray
			{ 0xFF, 0xFF, 0xFF, 0xFF, 15 }  // white
		};

struct stColor yaziohPalette[] = {
			// transparent values
			{ 0x00, 0x00, 0x00, 0x00,  0 }, // actually transparent
			{ 0xFF, 0x00, 0xFF, 0xFF,  0 }, // fuchsia
			{ 0x40, 0x40, 0x40, 0xFF,  0 }, // dark grey (PCX2MSX reference files)
			// Yazioh palette
			{ 0x00, 0x00, 0x00, 0xFF,  1 }, // black
			{ 0x3E, 0xB8, 0x49, 0xFF,  2 }, // medium green
			{ 0x74, 0xD0, 0x7D, 0xFF,  3 }, // light green
			{ 0x59, 0x55, 0xE0, 0xFF,  4 }, // dark blue
			{ 0x80, 0x76, 0xF1, 0xFF,  5 }, // light blue
			{ 0xB9, 0x5E, 0x51, 0xFF,  6 }, // dark red
			{ 0x65, 0xDB, 0xEF, 0xFF,  7 }, // cyan
			{ 0xDB, 0x65, 0x59, 0xFF,  8 }, // medium red
			{ 0xFF, 0x89, 0x7D, 0xFF,  9 }, // light red
			{ 0xCC, 0xC3, 0x5E, 0xFF, 10 }, // dark yellow
			{ 0xDE, 0xD0, 0x87, 0xFF, 11 }, // light yellow
			{ 0x3A, 0xA2, 0x41, 0xFF, 12 }, // dark green
			{ 0xB7, 0x66, 0xB5, 0xFF, 13 }, // magenta
			{ 0xCC, 0xCC, 0xCC, 0xFF, 14 }, // gray
			{ 0xFF, 0xFF, 0xFF, 0xFF, 15 }  // white
		};

/* Global vars ------------------------------------------------------------- */

extern int verbose;
extern int veryVerbose;

/* Private function prototypes --------------------------------------------- */

struct stColor* guessPalette(unsigned int pngWidth, unsigned int pngHeight, byte *pngImage);
int paletteDistance(struct stColor palette[], unsigned int pngWidth, unsigned int pngHeight, byte *pngImage);
int closestColorDistance(struct stColor palette[], byte r, byte g, byte b, byte a);
byte paletteIndex(struct stColor palette[], byte r, byte g, byte b, byte a);
int euclideanDistance(struct stColor *color, byte r, byte g, byte b, byte a);
int weightedDistance(struct stColor *color, byte r, byte g, byte b, byte a);

int (*distance) (struct stColor *color, byte r, byte g, byte b, byte a) = euclideanDistance;

/* Function bodies --------------------------------------------------------- */

void pngReaderOptions() {

	printf("\t-e\tindex color by euclidean distance (default)\n");
	printf("\t-g\tindex color by weighted distance\n");
}

void pngReaderInit(int argc, char **argv) {

	// Read arguments
	if (argEquals(argc, argv, "-e") != -1)
		distance = euclideanDistance;
	if (argEquals(argc, argv, "-g") != -1)
		distance = weightedDistance;
}

int pngReaderRead(char *pngFilename, struct stBitmap *bitmap) {

	if (!pngFilename)
		return 1;

	if (!bitmap)
		return 2;

	int i = 0;

	// Reads the PNG file (as 32-bit RGBA raw)
	byte *pngImage = 0;
	unsigned int pngWidth, pngHeight;
	unsigned int pngError = lodepng_decode32_file(&pngImage, &pngWidth, &pngHeight, pngFilename);
	if (pngError) {
		printf("ERROR: Could not read PNG: %u: %s\n", pngError, lodepng_error_text(pngError));
		i = 2 + pngError;
		goto out;
	}

	// Guesses the palette
	struct stColor *palette = guessPalette(pngWidth, pngHeight, pngImage);
	if (verbose) printf("Using %s...\n",
			palette == meiseiTms9218Palette ? "TI TMS9219 palette, from hap's meisei emulator"
			: palette == meiseiV9938Palette ? "V9938 palette, from hap's meisei emulator"
			: palette == reidracToshibaPalette ? "TOSHIBA palette, from reidrac's MSX Pixel Tools"
			: palette == wikipediaTms9918Palette ? "TI TMS9918 palette, according Wikipedia"
			: palette == yaziohPalette ? "Yazioh palette"
			: "unknown palette");

	// Allocate space for the bitmap
	bitmap->width = pngWidth;
	bitmap->height = pngHeight;
	bitmap->bitmap = (byte*) calloc(bitmap->width * bitmap->height, sizeof(byte));

	// Populates the bitmap
	unsigned int y;
	byte *source, *target;
	for (source = pngImage, target = bitmap->bitmap, y = 0; y < bitmap->height; y++) {
		for (unsigned int x = 0; x < bitmap->width; x++) {
			byte r = *(source++);
			byte g = *(source++);
			byte b = *(source++);
			byte a = *(source++);
			*(target++) = paletteIndex(palette, r, g, b, a);
		}
	}

out:
	// Exit gracefully
	if (pngImage) free(pngImage);
	return i;
}

/* Private function bodies ------------------------------------------------- */

struct stColor* guessPalette(unsigned int pngWidth, unsigned int pngHeight, byte *pngImage) {

	struct stColor* closestPalette = meiseiTms9218Palette;
	int minDistance = paletteDistance(meiseiTms9218Palette, pngWidth, pngHeight, pngImage);
	if (veryVerbose) printf("Distance %u: TI TMS9219 palette, from hap's meisei emulator\n", minDistance);
	if (minDistance == 0) return meiseiTms9218Palette;

	int dist = paletteDistance(meiseiV9938Palette, pngWidth, pngHeight, pngImage);
	if (veryVerbose) printf("Distance %u: V9938 palette, from hap's meisei emulator\n", dist);
	if (dist == 0) return meiseiV9938Palette;
	if (dist < minDistance) closestPalette = meiseiV9938Palette;

	dist = paletteDistance(reidracToshibaPalette, pngWidth, pngHeight, pngImage);
	if (veryVerbose) printf("Distance %u: TOSHIBA palette, from reidrac's MSX Pixel Tools\n", dist);
	if (dist == 0) return reidracToshibaPalette;
	if (dist < minDistance) closestPalette = reidracToshibaPalette;

	dist = paletteDistance(wikipediaTms9918Palette, pngWidth, pngHeight, pngImage);
	if (veryVerbose) printf("Distance %u: TI TMS9918 palette, according Wikipedia\n", dist);
	if (dist == 0) return wikipediaTms9918Palette;
	if (dist < minDistance) closestPalette = wikipediaTms9918Palette;

	dist = paletteDistance(yaziohPalette, pngWidth, pngHeight, pngImage);
	if (veryVerbose) printf("Distance %u: Yazioh palette\n", dist);
	if (dist == 0) return yaziohPalette;
	if (dist < minDistance) closestPalette = yaziohPalette;

	return closestPalette;
}

int paletteDistance(struct stColor palette[], unsigned int pngWidth, unsigned int pngHeight, byte *pngImage) {

	int totalDistance = 0;
	unsigned int y, x;
	byte *source;
	for (source = pngImage, y = 0; y < pngHeight; y++) {
		for (x = 0; x < pngWidth; x++) {
			byte r = *(source++);
			byte g = *(source++);
			byte b = *(source++);
			byte a = *(source++);
			int dist = closestColorDistance(palette, r, g, b, a);
			if (dist > 0) totalDistance += dist;
		}
	}
	return totalDistance;
}

int closestColorDistance(struct stColor palette[], byte r, byte g, byte b, byte a) {

	int i;
	struct stColor *it;
	int minDistance = -1;
	for (i = 0, it = palette; i < paletteLength; i++, it++) {
		int dist = distance(it, r, g, b, a);
		if (dist < 0) continue;
		if (dist == 0) return 0;
		if ((minDistance == -1) || (dist < minDistance)) minDistance = dist;
	}
	return minDistance;
}

byte paletteIndex(struct stColor palette[], byte r, byte g, byte b, byte a) {

	int i;
	struct stColor *it;
	struct stColor *closestColor = 0;
	int minDistance = -1;
	for (i = 0, it = palette; i < paletteLength; i++, it++) {
		int dist = distance(it, r, g, b, a);
		if (dist < 0) continue;
		if (dist == 0) {
			return it->index;
		}
		if ((minDistance == -1) || (dist < minDistance)) {
			closestColor = it;
			minDistance = dist;
		}
	}
	return closestColor->index;
}

int euclideanDistance(struct stColor *color, byte r, byte g, byte b, byte a) {

	// (transparent hack)
	if (color->a == 0x00)
		return (a == 0x00) ? 0 : -1;

	int deltaR = (int) color->r - (int) r;
	int deltaG = (int) color->g - (int) g;
	int deltaB = (int) color->b - (int) b;

	return sqrt(
			  (deltaR * deltaR)
			+ (deltaG * deltaG)
			+ (deltaB * deltaB)
		);
}

int weightedDistance(struct stColor *color, byte r, byte g, byte b, byte a) {

	// (transparent hack)
	if (color->a == 0x00)
		return (a == 0x00) ? 0 : -1;

	int deltaR = (int) color->r - (int) r;
	int deltaG = (int) color->g - (int) g;
	int deltaB = (int) color->b - (int) b;

	float averageR = (color->r + r) / 2.0f;

	float weightR = 2.0f + (averageR / 255.0f);
	float weightG = 4.0f;
	float weightB = 3.0f - (averageR / 255.0f);

	return sqrt(
			  (int) (weightR * deltaR * deltaR)
			+ (int) (weightG * deltaG * deltaG)
			+ (int) (weightB * deltaB * deltaB)
		);
}
