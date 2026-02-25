/*
 * Support routines for output in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "nametable.h"

/* Private function prototypes --------------------------------------------- */

// void optimizeLine(struct stChrClrWriter *instance, struct stLine *line, byte optColor);
void reallocateBlocks(struct stCharset *charset, int newBlockCount);

/* Function bodies --------------------------------------------------------- */

void nameTableProcessorOptions() {

	printf("\t-n[<00..ff>]\tgenerate NAMTBL [starting at value <n>]\n");
	printf("\t-bb<00..ff>\tblank block at position <nn>\n");
	printf("\t-rr\tremove repeated tiles\n");
	printf("\t-rm<0..f>\tremove solid tiles of <n> color\n");
}

void nameTableProcessorInit(struct stNameTableProcessor *this, int argc, char **argv) {

	// Init
	this->namtblOffset = this->blankAt = 0;
	this->emptyColorRemoved = 0;

	// Read arguments
	int i;
	if (((i = argStartsWith(argc, argv, "-n", 2)) != -1)) {
		this->namtblOffset = hexadecimalInt(&(argv[i][2]));
	}
	if ((this->generateBlank = ((i = argStartsWith(argc, argv, "-bb", 4)) != -1))) {
		this->blankAt = hexadecimalInt(&(argv[i][3]));
	}
	this->removeRepeated = (argEquals(argc, argv, "-rr") != -1);
	if ((this->removeEmpty = ((i = argStartsWith(argc, argv, "-rm", 4)) != -1))) {
		this->emptyColorRemoved = hexadecimalNibble(argv[i][3]);
	}
	this->traverseHorizontally =
		  (argEquals(argc, argv, "-th") != -1) ? 1
		: (argEquals(argc, argv, "-tv") != -1) ? 0
		: 1;
}

void nameTableProcessorGenerate(struct stNameTableProcessor *this,
		struct stNameTable *nametable, struct stCharset *charset) {

	// Initial namtbl
	nametable->namtblSize = charset->blockCount;
	nametable->namtbl = (int*) calloc(nametable->namtblSize, sizeof(int));

	// For each block...
	int i, b, c, blockCount, *it;
	struct stBlock *src, *dest;

	if (this->traverseHorizontally) {

		for (i = 0, b = 0, blockCount = 0, src = dest = charset->blocks, it = nametable->namtbl;
				i < charset->blockCount; i++, src++, it++) {
			// Remove if empty
			if (this->removeEmpty && isSolidBlock(src, this->emptyColorRemoved)) {
				*it = -1;
				continue;
			}
			// Remove if repeated
			if (this->removeRepeated && ((c = blockIndex(charset, src, b)) != -1)) {
				*it = c;
				continue;
			}
			// Not removed: move (compact blocks)
			if (src != dest) memcpy(dest, src, sizeof(struct stBlock));
			*it = b++;
			blockCount++;
			dest++;
		}

	} else {

		unsigned int x, y;

		for (x = 0, b = 0, blockCount = 0, src = dest = charset->blocks, it = nametable->namtbl; x < charset->width; x++) {
			for (y = 0; y < charset->height; y++, src++, it++) {

				// Remove if empty
				if (this->removeEmpty && isSolidBlock(src, this->emptyColorRemoved)) {
					*it = -1;
					continue;
				}
				// Remove if repeated
				if (this->removeRepeated && ((c = blockIndex(charset, src, b)) != -1)) {
					*it = c;
					continue;
				}
				// Not removed: move (compact blocks)
				if (src != dest) memcpy(dest, src, sizeof(struct stBlock));
				*it = b++;
				blockCount++;
				dest++;
			}
		}

		// Transposes the namtbl
		int *transposed = (int*) calloc(nametable->namtblSize, sizeof(int));

		for (i = 0; i < nametable->namtblSize; i++) {
			int toX = i / charset->height,
				toY = i % charset->height,
				to = toY * charset->width + toX;
			transposed[to] = nametable->namtbl[i];
		}

		free(nametable->namtbl);
		nametable->namtbl = transposed;
	}
	reallocateBlocks(charset, blockCount);

	if (this->generateBlank && (this->blankAt <= charset->blockCount)) {
		// Allocate
		reallocateBlocks(charset, charset->blockCount + 1);
		// Move
		int j;
		for (i = charset->blockCount - 1, j = i - 1; j >= this->blankAt; i--, j--) {
			memcpy(&charset->blocks[i], &charset->blocks[j], sizeof(struct stBlock));
		}
		// Blank
		struct stLine *itLine;
		for (i = 0, itLine = charset->blocks[this->blankAt].line; i < TILE_HEIGHT; i++, itLine++) {
			itLine->pattern = 0x00;
			itLine->color = 0x00;
		}
	}
}

void nameTableProcessorGenerateUsing(__attribute__((unused)) struct stNameTableProcessor *this,
		struct stNameTable *nametable, struct stCharset *charset, struct stCharset *screen) {

	// Initial namtbl
	nametable->namtblSize = screen->blockCount;
	nametable->namtbl = (int*) calloc(nametable->namtblSize, sizeof(int));

	// For each block...
	int i, j, *it;
	struct stBlock *block;
	for (i = 0, it = nametable->namtbl, block = screen->blocks; i < nametable->namtblSize; i++, it++, block++) {
		j = blockIndex(charset, block, charset->blockCount);
		*it = j;
	}
}

void nameTableProcessorApplyTo(struct stNameTable *nametable, struct stCharset *charset) {

	int i, expected, blockCount, *it;
	struct stBlock *src, *dest;

	for (i = expected = 0, blockCount = 0, src = dest = charset->blocks, it = nametable->namtbl;
			(i < charset->blockCount) && (i < nametable->namtblSize); i++, src++, it++) {
		// Empty or repeated in master pcx
		if ((*it == -1) || (*it < expected))
			continue;

		// Not empty nor repeated in master pcx: move (compact blocks)
		if (src != dest) memcpy(dest, src, sizeof(struct stBlock));
		blockCount++;
		dest++;
	}
	reallocateBlocks(charset, blockCount);
}

void nameTableProcessorPostProcess(struct stNameTableProcessor *this, struct stNameTable *nametable) {

	int i, *it;
	for (i = 0, it = nametable->namtbl; i < nametable->namtblSize; i++, it++)
		*it = (*it == -1)
			? ((this->generateBlank) ? this->blankAt : -1)
			: *it + this->namtblOffset + ((this->generateBlank) && (*it >= this->blankAt) ? 1 : 0);
}

int nameTableProcessorWrite(struct stNameTableProcessor *this, struct stNameTable *nametable, FILE *namFile) {

	int i, *it;
	byte b;
	for (i = 0, it = nametable->namtbl; i < nametable->namtblSize; i++, it++) {
		b = (byte) (((*it == -1) ? (this->namtblOffset + 0xff) : (*it)) & 0xff);
		if (fwrite(&b, sizeof(byte), 1, namFile) != 1)
			return 1;
	}
	return 0;
}

void nameTableProcessorDone(__attribute__((unused)) struct stNameTableProcessor *this) {

	// nothing to do here
}

void nameTableDone(struct stNameTable *this) {

	if (this->namtbl) free(this->namtbl);
	this->namtbl = NULL;
}

/* Private function bodies ------------------------------------------------- */

void reallocateBlocks(struct stCharset *charset, int newBlockCount) {

	if (newBlockCount == charset->blockCount)
		return;

	charset->blockCount = newBlockCount;
	if (newBlockCount) {
		// Reallocate space for the blocks
		charset->blocks = (struct stBlock*) realloc(charset->blocks, newBlockCount * sizeof(struct stBlock));
	} else {
		// Remove blocks
		free(charset->blocks);
		charset->blocks = NULL;
	}
}

// void optimizeLine(struct stChrClrWriter *this, struct stLine *line, byte optColor) {

	// // Nothing to optimize
	// if (line->color == optColor) return;

	// byte swappedColor = ((optColor & 0x0f) << 4) | (optColor >> 4);

	// switch (this->patternMode & PATTERN_MODE_MASK) {
	// default:
		// if (line->color == swappedColor) {
			// negateAndSwap(line);
			// line->color = optColor;
			// return;
		// }
		// // (falls through)

	// case PATTERN_MODE_HIGH_LOW:
	// case PATTERN_MODE_LOW_HIGH:
		// // Cannot swap if two colors
		// if ((line->pattern == 0x00) && ((line->color & 0x0f) == (swappedColor & 0x0f))) {
			// negateAndSwap(line);
			// line->color = optColor;
			// return;
		// }
		// if ((line->pattern == 0xff) && ((line->color & 0xf0) == (swappedColor & 0xf0))) {
			// negateAndSwap(line);
			// line->color = optColor;
			// return;
		// }
		// // (falls through)

	// case PATTERN_MODE_FOREGROUND:
	// case PATTERN_MODE_BACKGROUND:
		// // Cannot swap, cannot set/reset bits
		// if ((line->pattern == 0x00) && ((line->color & 0x0f) == (optColor & 0x0f))) {
			// line->color = optColor;
			// return;
		// }
		// if ((line->pattern == 0xff) && ((line->color & 0xf0) == (optColor & 0xf0))) {
			// line->color = optColor;
			// return;
		// }
		// return;
	// }






	// int allowSetReset = ((this->patternMode & PATTERN_MODE_MASK) != PATTERN_MODE_FOREGROUND)
			// && ((this->patternMode & PATTERN_MODE_MASK) != PATTERN_MODE_BACKGROUND);

	// // Optimize 0x00 pattern
	// if (line->pattern == 0x00) {
		// if ((line->color & 0x0f) == (optColor & 0x0f)) {
			// line->color = optColor;
			// return;
		// }
		// if (allowSetReset && ((line->color & 0x0f) == (optColor >> 4))) {
			// negateAndSwap(line);
			// line->color = optColor;
			// return;
		// }
		// return;
	// }

	// // Optimize 0xff pattern
	// if (line->pattern == 0xff) {
		// if ((line->color >> 4) == (optColor >> 4)) {
			// line->color = optColor;
			// return;
		// }
		// if (allowSetReset && ((line->color >> 4) == (optColor & 0x0f))) {
			// negateAndSwap(line);
			// line->color = optColor;
			// return;
		// }
		// return;
	// }

	// int allowSwap = (this->patternMode & PATTERN_MODE_MASK) == PATTERN_MODE_UNSET;

	// // Optimize other pattern
	// if (allowSwap
			// && ((line->color & 0x0f) == (optColor >> 4))
			// && ((line->color >> 4) != (optColor && 0x0f))) {
		// negateAndSwap(line);
		// line->color = optColor;
		// return;
	// }

	// return;
// }

// // int isOptimizableLine(struct stChrClrWriter *this, struct stLine *line, byte optColor) {

	// // switch (this->patternMode & PATTERN_MODE_MASK) {
	// // case PATTERN_MODE_FOREGROUND:
	// // case PATTERN_MODE_BACKGROUND:
		// // // Doesn't allow swap
		// // if (line->pattern == 0x00)
			// // return (line->color & 0x0f) == (optColor & 0x0f);
		// // if (line->pattern == 0xff)
			// // return (line->color >> 4) == (optColor >> 4);
		// // return 0;

	// // default:
		// // // Allow swap
		// // if (line->pattern == 0x00)
			// // return ((line->color & 0x0f) == (optColor & 0x0f))
				// // || ((line->color & 0x0f) == (optColor >> 4));
		// // if (line->pattern == 0xff)
			// // return ((line->color >> 4) == (optColor & 0x0f))
				// // || ((line->color >> 4) == (optColor >> 4));
		// // return ((line->color & 0x0f) == (optColor >> 4))
			// // && ((line->color >> 4) == (optColor && 0x0f));
	// // }
// // }

// // void optimizeLine(struct stChrClrWriter *this, struct stLine *line, byte optColor) {

	// // if (line->pattern == 0x00) {
		// // if ((line->color & 0x0f) != (optColor & 0x0f))
			// // negateAndSwap(line);
		// // line->color = optColor;
		// // return;
	// // }

	// // if (line->pattern == 0xff) {
		// // if ((line->color >> 4) != (optColor >> 4))
			// // negateAndSwap(line);
		// // line->color = optColor;
		// // return;
	// // }

	// // negateAndSwap(line);
	// // line->color = optColor; // should be unnecesary
// // }
