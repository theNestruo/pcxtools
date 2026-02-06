/*
 * Support routines for output in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "charset.h"

/* Symbolic constants ------------------------------------------------------ */

#define PATTERN_MODE_UNSET (0x00)
#define PATTERN_MODE_FOREGROUND (0x10)
#define PATTERN_MODE_BACKGROUND (0x20)
#define PATTERN_MODE_HIGH_LOW (0x40)
#define PATTERN_MODE_LOW_HIGH (0x80)
#define PATTERN_MODE_LIGHT_DARK (0x100)
#define PATTERN_MODE_DARK_LIGHT (0x200)
#define PATTERN_MODE_MASK (PATTERN_MODE_UNSET \
	| PATTERN_MODE_FOREGROUND | PATTERN_MODE_BACKGROUND \
	| PATTERN_MODE_HIGH_LOW | PATTERN_MODE_LOW_HIGH \
	| PATTERN_MODE_LIGHT_DARK | PATTERN_MODE_DARK_LIGHT)

/* Global vars ------------------------------------------------------------- */

extern int verbose;
extern int veryVerbose;

/* Private function prototypes --------------------------------------------- */

// Brigthness by color, matching the color order: 0146C285D937ABEF
const unsigned int brightness[] = { 0, 1, 5, 10, 2, 7, 3, 11, 6, 9, 12, 13, 4, 8, 14, 15 };

int patternMode(int isForeground, char bit);
void postProcessRange(struct stCharsetProcessor *instance, char *argstring);

void charsetProcessorInitForBitmap(struct stCharsetProcessor *instance, struct stBitmap *bitmap);
int readLine(struct stCharsetProcessor *instance, struct stLine *line, struct stBitmap *bitmap, int x, int y, struct stLine *previousLine);
void postProcessLine(struct stCharsetProcessor *instance, int index, struct stLine *line, struct stLine *previousLine);
void debugPostProcessLine(struct stCharsetProcessor *instance, struct stLine *from, struct stLine *to, struct stLine *previousLine, char *message);
void negateAndSwap(struct stLine *line);
int isLineEquals(struct stLine *line, struct stLine *other);
int isLineSingleColor(struct stLine *line);
byte colorAtBit(struct stLine *line, char bit);

/* Function bodies --------------------------------------------------------- */

// Supported arguments:
// -il		 ignore line on color collision
// -sa		 auto-detect stripped images (default)
// -sy		 force image to be detected as stripped (simplified algorithm)
// -sn		 force image to be detected as non-stripped (default algorithm)
// -hl		 force higher color to be foreground
// -lh		 force lower color to be foreground
// -ld		 force lighter foreground, darker background
// -dl		 force darker foreground, lighter background
// -f<0..7>	 force bit <n> to be foreground (set) on patterns
// -b<0..7>	 force bit <n> to be background (reset) on patterns
// -pf<0000> post-process only the specified address range (from)
// -pt<ffff> post-process only the specified address range (to)
void charsetProcessorOptions() {

	printf("\t-il\tignore line on color collision\n");
	printf("\t-sa\tauto-detect stripped images (default)\n");
	printf("\t-sy\tforce stripped image (forces using simplified algorithm)\n");
	printf("\t-sn\tforce non-stripped image\n");
	printf("\t-hl\tforce higher color to be foreground\n");
	printf("\t-lh\tforce lower color to be foreground\n");
	printf("\t-ld\tforce lighter foreground, darker background\n");
	printf("\t-dl\tforce darker foreground, lighter background\n");
	printf("\t-f<0..7>\tforce bit <n> to be foreground (set) on patterns\n");
	printf("\t-b<0..7>\tforce bit <n> to be background (reset) on patterns\n");
	printf("\t-pf<0000>\tpost-process only the specified address range (from)\n");
	printf("\t-pt<ffff>\tpost-process only the specified address range (to)\n");
}

void charsetProcessorInit(struct stCharsetProcessor *this, int argc, char **argv) {

	// Init
	this->ignoreCollision = 0;
	this->patternMode = PATTERN_MODE_UNSET;
	this->preferredBackground = 0x00;

	// Read arguments
	int i;
	this->ignoreCollision = (argEquals(argc, argv, "-il") != -1);
	this->forceStrippedImage =
		  (argEquals(argc, argv, "-sy") != -1) ? 1
		: (argEquals(argc, argv, "-sn") != -1) ? 0
		: -1;
	this->patternMode =
		  ((i = argStartsWith(argc, argv, "-f", 3)) != -1) ? patternMode(1, argv[i][2])
		: ((i = argStartsWith(argc, argv, "-b", 3)) != -1) ? patternMode(0, argv[i][2])
		: (argEquals(argc, argv, "-hl") != -1) ? PATTERN_MODE_HIGH_LOW
		: (argEquals(argc, argv, "-lh") != -1) ? PATTERN_MODE_LOW_HIGH
		: (argEquals(argc, argv, "-ld") != -1) ? PATTERN_MODE_LIGHT_DARK
		: (argEquals(argc, argv, "-dl") != -1) ? PATTERN_MODE_DARK_LIGHT
		: PATTERN_MODE_UNSET;
	this->postProcessRangeFrom =
		  ((i = argStartsWith(argc, argv, "-pf", 4)) != -1) ? hexadecimalInt(&(argv[i][3]))
		: -1;
	this->postProcessRangeTo =
		  ((i = argStartsWith(argc, argv, "-pt", 4)) != -1) ? hexadecimalInt(&(argv[i][3]))
		: -1;
}

int charsetProcessorRead(struct stCharsetProcessor *this, struct stCharset *charset, struct stBitmap *bitmap) {

	charsetProcessorInitForBitmap(this, bitmap);

	if (verbose)
			printf("Detected preferred background color = %1x, image is %s\n",
					this->preferredBackground,
					this->isStrippedImage
						? (this->forceStrippedImage == 1 ? "stripped (forced)" : "stripped")
						: (this->forceStrippedImage == 0 ? "not stripped (forced)" : "not stripped"));

	// Allocate space for the blocks
	charset->blockCount = ((int) (bitmap->width / TILE_WIDTH)) * ((int) (bitmap->height / TILE_HEIGHT));
	charset->blocks = (struct stBlock*) calloc(charset->blockCount, sizeof(struct stBlock));

	// For each block...
	unsigned int x, y;
	struct stBlock *itBlock;
	struct stLine previousLine0 = { 0x00, this->preferredBackground << 4 | this->preferredBackground };
	struct stLine *previousLine;
	for (y = 0, itBlock = charset->blocks, previousLine = &previousLine0; (y + TILE_HEIGHT) <= bitmap->height; y += TILE_HEIGHT) {
		for (x = 0; (x + TILE_WIDTH) <= bitmap->width; x += TILE_WIDTH, itBlock++) {

			// For each line...
			int i;
			struct stLine *itLine;
			for (i = 0, itLine = itBlock->line; i < TILE_HEIGHT; i++, previousLine = itLine, itLine++) {
				if (!readLine(this, itLine, bitmap, x, y + i, previousLine) && !this->ignoreCollision)
					return 1;
			}
		}
	}

	return 0;
}

void charsetProcessorPostProcess(struct stCharsetProcessor *this, struct stCharset *charset) {

	// For each block...
	int i;
	struct stBlock *itBlock;
	struct stLine previousLine0 = { 0x00, this->preferredBackground << 4 | this->preferredBackground };
	struct stLine *previousLine;
	for (i = 0, itBlock = charset->blocks, previousLine = &previousLine0; i < charset->blockCount; i++, itBlock++) {

		// For each line...
		int j;
		struct stLine *itLine;
		for (j = 0, itLine = itBlock->line; j < TILE_HEIGHT; j++, previousLine = itLine, itLine++) {
			postProcessLine(this, i * TILE_HEIGHT + j, itLine, previousLine);
		}
	}
}

int charsetProcessorWrite(__attribute__((unused)) struct stCharsetProcessor *this, struct stCharset *charset, FILE *chrFile, FILE *clrFile) {

	int i, j;
	struct stBlock *it;
	struct stLine *line;
	for (i = 0, it = charset->blocks; i < charset->blockCount; i++, it++) {
		for (j = 0, line = it->line; j < TILE_HEIGHT; j++, line++) {
			// Write value in output files
			if (fwrite(&(line->pattern), sizeof(byte), 1, chrFile) != 1)
				return 2;
			if (fwrite(&(line->color), sizeof(byte), 1, clrFile) != 1)
				return 3;
		}
	}

	return 0;
}

void charsetProcessorDone(__attribute__((unused)) struct stCharsetProcessor *this) {

	// nothing to do here
}

void charsetDone(struct stCharset *this) {

	if (this->blocks) free(this->blocks);
	this->blocks = NULL;
}

int isSolidBlock(struct stBlock *block, byte color) {

	int i;
	struct stLine *line;
	for (i = 0, line = block->line; i < TILE_HEIGHT; i++, line++) {
		if (((line->pattern) != 0xff) && ((line->color & 0x0f) != color))
			return 0;
		if (((line->pattern) != 0x00) && ((line->color & 0xf0) != (color << 4)))
			return 0;
	}
	return 1;
}

int isBlockEquals(struct stBlock *this, struct stBlock *that) {

	int i;
	struct stLine *thisLine, *thatLine;
	for (i = 0, thisLine = this->line, thatLine = that->line; i < TILE_HEIGHT; i++, thisLine++, thatLine++)
		if (!isLineEquals(thisLine, thatLine))
			return 0;

	return 1;
}

int blockIndex(struct stCharset *charset, struct stBlock *block, int stopBefore) {

	int i;
	struct stBlock *it;
	for (i = 0, it = charset->blocks; (i < charset->blockCount) && (i < stopBefore); i++, it++)
		if (isBlockEquals(block, it))
			return i;

	return -1;
}

/* Private function bodies ------------------------------------------------- */

int patternMode(int isForeground, char bit) {

	if ((bit < '0') || (bit > '7'))
		return PATTERN_MODE_UNSET;

	return (isForeground ? PATTERN_MODE_FOREGROUND : PATTERN_MODE_BACKGROUND)
		| (bit - '0');
}

void charsetProcessorInitForBitmap(struct stCharsetProcessor *this, struct stBitmap *bitmap) {

	unsigned int count[16] = {0};
	unsigned int evenCount[16] = {0};
	unsigned int oddCount[16] = {0};
	unsigned int x, y;
	for (x = 0; x < bitmap->width; x++) {
		for (y = 0; y < bitmap->height; y += 2) {
			byte evenColor = bitmapGet(bitmap, x, y);
			byte oddColor = bitmapGet(bitmap, x, y + 1);
			count[evenColor]++;
			count[oddColor]++;
			evenCount[evenColor]++;
			oddCount[oddColor]++;
		}
	}

	byte mostFrequentColor = 0x00, mostFrequentEvenColor = 0x00, mostFrequentOddColor = 0x00;
	unsigned int maxCount = 0, maxEvenCount = 0, maxOddCount = 0;
	for (byte color = 0; color < 16; color++) {
		if (veryVerbose) printf("Color %x frequency: %d\n", color, count[color]);
		if (count[color] > maxCount) {
			maxCount = count[color];
			mostFrequentColor = color;
		}
		if (evenCount[color] > maxEvenCount) {
			maxEvenCount = evenCount[color];
			mostFrequentEvenColor = color;
		}
		if (oddCount[color] > maxOddCount) {
			maxOddCount = oddCount[color];
			mostFrequentOddColor = color;
		}
	}

	this->preferredBackground = mostFrequentColor;

	// Stripped image detection -----------------------------------------------

	// Forced to yes/no?
	if (this->forceStrippedImage != -1) {
		this->isStrippedImage = this->forceStrippedImage;
		return;
	}

	// Even/odd background is reference background and the other one is not?
	if ((mostFrequentColor == mostFrequentEvenColor) == (mostFrequentColor == mostFrequentOddColor)) {
		this->isStrippedImage = 0;
		return;
	}

	// Enough excess of reference background representation in even/odd background (>= 5%)?
	unsigned int excess = (mostFrequentColor == mostFrequentEvenColor)
			? abs((int) maxEvenCount * 2 - (int) maxCount) / 2
			: abs((int) maxOddCount  * 2 - (int) maxCount) / 2;
	unsigned int threshold = 5 * bitmap->width * bitmap->height / 100;
	this->isStrippedImage = excess >= threshold;
	return;
}

int readLine(struct stCharsetProcessor *this, struct stLine *line, struct stBitmap *bitmap, int x, int y, __attribute__((unused)) struct stLine *previousLine) {

	int i, j;
	byte colors[TILE_WIDTH];
	byte pattern = 0x00, bgcandidate = 0xff, fgcandidate = 0xff;

	// Read the colors
	for (i = 0; i < TILE_WIDTH; i++) {
		colors[i] = bitmapGet(bitmap, x + i, y);
	}

	// Create the pattern
	for (i = 0, j = TILE_WIDTH - 1; i < TILE_WIDTH; i++, j--) {
		byte color = colors[j];
		if (bgcandidate == 0xff) {
			bgcandidate = color;
			// pattern |= 0<<i; // (unnecesary)
		} else if (bgcandidate == color) {
			// pattern |= 0<<i; // (unnecesary)
		} else if (fgcandidate == 0xff) {
			fgcandidate = color;
			pattern |= 1<<i;
		} else if (fgcandidate == color) {
			pattern |= 1<<i;
		} else {
			// More than two colors
			printf("%s: Collision at %d,%d: %x%x%x%x%x%x%x%x\n",
				this->ignoreCollision ? "WARN" : "ERROR",
				x, y,
				colors[0], colors[1], colors[2], colors[3],
				colors[4], colors[5], colors[6], colors[7]);
			line->pattern = 0x00;
			line->color = 0x00;
			return 0;
		}
	}

	if (fgcandidate == 0xff) {
		line->pattern = 0xff;
		line->color = (bgcandidate & 0x0f) << 4 | this->preferredBackground;
	} else {
		line->pattern = pattern;
		line->color = (fgcandidate & 0x0f) << 4 | (bgcandidate & 0x0f);
	}

	/*
	 * (postProcess delayed after nametable processing)
	 *
	postProcessLine(this, line, previousLine);
	 */

	return 1;
}

void postProcessLine(struct stCharsetProcessor *this, int index, struct stLine *line, struct stLine *previousLine) {

	struct stLine copyOfLine = { line->pattern, line->color };

	int patternMode =
			   ((this->postProcessRangeFrom == -1) || (this->postProcessRangeFrom <= index))
			&& ((this->postProcessRangeTo   == -1) || (this->postProcessRangeTo   >= index))
			? this->patternMode
			: PATTERN_MODE_UNSET;

	// Apply current pattern mode
	switch (patternMode & PATTERN_MODE_MASK) {
	case PATTERN_MODE_FOREGROUND:
		// Force foreground bit
		if (!(line->pattern & (1 << (this->patternMode & 0x07)))) {
			negateAndSwap(line);
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force foreground bit (inverted line)");
			return;
		}
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force foreground bit");
		return;

	case PATTERN_MODE_BACKGROUND:
		// Force background bit
		if (line->pattern & (1 << (this->patternMode & 0x07))) {
			negateAndSwap(line);
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force background bit (inverted line)");
			return;
		}
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force background bit");
		return;

	case PATTERN_MODE_HIGH_LOW:
		// Force higher color foreground
		if ((line->color >> 4) < (line->color & 0x0f)) {
			negateAndSwap(line);
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force higher color foreground (inverted line)");
			return;
		}
		if ((line->pattern == 0xff) && ((line->color >> 4) < 2)) {
			negateAndSwap(line); // WORKAROUND: forces color 0 or 1 always background
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force higher color foreground (workaround for 0/1)");
			return;
		}
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force higher color foreground");
		return;

	case PATTERN_MODE_LOW_HIGH:
		// Force higher color background
		if ((line->color & 0x0f) < (line->color >> 4)) {
			negateAndSwap(line);
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force higher color background (inverted line)");
			return;
		}
		if ((line->pattern == 0x00) && ((line->color & 0x0f) == 15)) {
			negateAndSwap(line); // WORKAROUND: forces color 15 always foreground
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force higher color background (workaround for 15)");
			return;
		}
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force higher color background");
		return;

	case PATTERN_MODE_LIGHT_DARK:
		// Force lighter color foreground
		if (brightness[line->color >> 4] < brightness[line->color & 0x0f]) {
			negateAndSwap(line);
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force lighter color foreground (inverted line)");
		}
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force lighter color foreground");
		return;

	case PATTERN_MODE_DARK_LIGHT:
		// Force darker color foreground
		if (brightness[line->color & 0x0f] < brightness[line->color >> 4]) {
			negateAndSwap(line);
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force darker color foreground (inverted line)");
		}
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Force darker color foreground");
		return;
	}

	// PATTERN_MODE_UNSET

	// Choses the best pattern+color combination
	if (isLineEquals(line, previousLine)) {
		line->pattern = previousLine->pattern;
		line->color = previousLine->color;
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Same as previous line");
		return;
	}

	// Single color (no fg candidate)
	const int singleColor = isLineSingleColor(line);
	if (singleColor != -1) {

		if (this->isStrippedImage) {
			// This seems to yield better compression ratios than more complex algorithms
			// for stripped images that have rapidly changing either CHRLTBL or CLRTBL bytes
			// (this should be no-op)
			line->pattern = 0xff;
			line->color = singleColor << 4 | this->preferredBackground;
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Full foreground");
			return;
		}

		// Attempts to reuse the previous CLRTBL value
		if (singleColor == (previousLine->color & 0x0f)) {
			line->pattern = 0x00;
			line->color = previousLine->color;
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Full background (reuses previous line colors)");
			return;
		}
		if (singleColor == (previousLine->color >> 4)) {
			line->pattern = 0xff;
			line->color = previousLine->color;
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Full foreground (reuses previous line colors)");
			return;
		}

		int isBackground = (brightness[this->preferredBackground] < 8)
				== (brightness[singleColor] < 8);

		// Attempts to use preferred background
		if (isBackground) {
			line->pattern = 0x00;
			line->color = 0x00 << 4 | singleColor;
			debugPostProcessLine(this, &copyOfLine, line, previousLine, "Full background");
			return;
		}

		// Uses the single color as foreground over the preferred background
		// (this should be no-op)
		line->pattern = 0xff;
		line->color = singleColor << 4 | this->preferredBackground;
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Full foreground (over preferred background)");
		return;
	}

	// Two colors -------------------------------------------------------------

	const byte fg = (line->color >> 4);
	const byte bg = (line->color & 0x0f);

	// Attempts to reuse the previous CLRTBL value
	if (line->color == previousLine->color) {
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Reuses previous line colors");
		return;
	}
	if (((bg << 4) | fg) == previousLine->color) {
		negateAndSwap(line);
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Reuses previous line colors (inverted line)");
		return;
	}

	// Attempts to use the preferred background
	if (bg == this->preferredBackground) {
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Preferred background");
		return;
	}
	if (fg == this->preferredBackground) {
		negateAndSwap(line);
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Preferred background (inverted line)");
		return;
	}

	// Preferred background not present and cannot reuse previous CLRTBL
	const int isInverted = (brightness[this->preferredBackground] < 8)
				? (brightness[bg] > brightness[fg])  // light-over-dark if preferred background is dark
				: (brightness[bg] < brightness[fg]); // dark-over-light if preferred background is light

	if (isInverted) {
		negateAndSwap(line);
		debugPostProcessLine(this, &copyOfLine, line, previousLine, "Two colors (inverted line)");
		return;
	}

	debugPostProcessLine(this, &copyOfLine, line, previousLine, "Two colors");
	return;
}

void debugPostProcessLine(struct stCharsetProcessor *this, struct stLine *from, struct stLine *to, struct stLine *previousLine, char *message) {

	if (!veryVerbose) return;

	printf("[%02x %02x] %s [%02x %02x] (pref.bg=%x, previous=[%02x %02x]) %s\n",
			from->pattern, from->color,
			((from->pattern == to->pattern) && (from->color == to->color)) ? "==" : "XX",
			to->pattern, to->color,
			this->preferredBackground, previousLine->pattern, previousLine->color,
			message);
}

void negateAndSwap(struct stLine *line) {

	line->pattern ^= 0xff;
	line->color = ((line->color & 0x0f) << 4) | (line->color >> 4);
}

int isLineEquals(struct stLine *thisLine, struct stLine *thatLine) {

	int i;
	for (i = 0; i < TILE_WIDTH; i++)
		if (colorAtBit(thisLine, i) != colorAtBit(thatLine, i))
			return 0;

	return 1;
}

int isLineSingleColor(struct stLine *line) {

	return    (line->pattern == 0x00) ? (line->color & 0x0f)
			: (line->pattern == 0xff) ? (line->color >> 4)
			: ((line->color & 0x0f) == (line->color >> 4)) ? (line->color & 0x0f)
			: -1;
}

byte colorAtBit(struct stLine *line, char bit) {

	return (line->pattern & (0x01 << bit))
		? (line->color >> 4)
		: (line->color & 0x0f);
}
