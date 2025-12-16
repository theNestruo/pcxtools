/*
 * Additional support routines for managing sprites
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "sprite+.h"
#include "output.h"

/* Symbolic constants ------------------------------------------------------ */

#define SPAT_END (0xd0)
#define SPAT_OB (0xd1)

#define OVER_TOP (0x01) // pixels over the layer
#define UNDER_BOTTOM (0x02) // pixels under the layer
#define TO_THE_LEFT (0x04) // pixels to the left of the layer in the same band
#define TO_THE_RIGHT (0x08) // pixels to the right of the layer in the same band
#define PIXEL_COVERED (0x10) // pixels covered by the layer

/* Global vars ------------------------------------------------------------- */

extern int verbose;
extern int veryVerbose;

/* Private data structures ------------------------------------------------- */

struct stSprite {
	byte pattern[32];
	byte attributes[4];
};

/* Private function prototypes --------------------------------------------- */

/* Private function prototypes ----- struct stSpriteSolver ----------------- */

int readSprite(struct stSpriteSolver *instance);
int initColorSolver(struct stSpriteSolver *instance, byte color);
void solve(struct stSpriteSolver *instance);
int checkBetterSolution(struct stSpriteSolver *instance, int indexes[16]);
int nextSolution(struct stSpriteSolver *instance, int indexes[16]);

byte getPixelAt(struct stSpriteSolver *instance, int x, int y);
struct stColorSolver *getColorSolver(struct stSpriteSolver *instance, byte color);
int getSpriteCount(struct stSpriteSolver *instance);
struct stScanlineCount getScanlineCount(struct stSpriteSolver *instance, int indexes[16]);
int stSpriteSolverWrite(struct stSpriteSolver *instance, int *spriteIndex, FILE *sprFile, FILE *spatFile);
int writeHeader(struct stSpriteSolver *instance, FILE *sprFile, FILE *spatFile);
int writePadding(struct stSpriteSolver *instance, FILE *spatFile);
void stSpriteSolverDone(struct stSpriteSolver *instance);

/* Private function prototypes ----- struct stColorSolver ------------------ */

void findRects(struct stColorSolver *instance);
int initRect(struct stColorSolver *instance, struct stRect *rects, int depth);
int checkRect(struct stColorSolver *instance, struct stRect *rects, int depth);
int moveRect(struct stColorSolver *instance, struct stRect *rects, int depth, int flags);
int checkSolution(struct stColorSolver *instance, struct stRect *rects);
int moveSolution(struct stColorSolver *instance, struct stRect *rects);
void saveSolution(struct stColorSolver *instance, struct stRect *rects);
int getColorScanlineCount(struct stColorSolver *instance, int index, int y);
void stColorSolverDone(struct stColorSolver *instance);

/* Private function prototypes ----- struct stRect ------------------------- */

void addLayer(struct stRect **rects, int *count, struct stRect *src);

/* Private function prototypes ----- struct stSprite ----------------------- */

void readPattern(struct stSprite *instance, struct stColorSolver *colorSolver, struct stRect *rect);
void readAttributes(struct stSprite *instance, struct stColorSolver *colorSolver, int spriteIndex, struct stRect *rect);
int writePattern(struct stSprite *instance, struct stSprWriterPlus *cfg, FILE *sprFile);
int writeAttributes(struct stSprite *instance, struct stSprWriterPlus *cfg, FILE *spatFile);

/* Private function prototypes ----- DEBUG --------------------------------- */

void debugSolution(struct stColorSolver *instance, struct stRect *rects, int size);

/* Function bodies --------------------------------------------------------- */

void sprWriterPlusOptions() {

	printf("\t-w<16..255>\tsprite width (default: 16px)\n");
	printf("\t-h<16..255>\tsprite height (default: 16px)\n");
	printf("\t-x<0..255>\tX offset (default: center)\n");
	printf("\t-y<0..255>\tY offset (default: middle)\n");
	printf("\t-p<0..4>\tattribute padding size (default: 1b)\n");
	printf("\t-t<00..ff>\tterminator byte (default: 0xD0 (SPAT_END))\n");
	printf("\t-b\tbinary spr/spat output (default: asm)\n");
	// printf("\t-vv\tVERY verbose execution\n"); // secret option (!^_^)
}

void sprWriterPlusInit(struct stSprWriterPlus *this, int argc, char **argv) {

	// Init
	this->sprites = NULL;
	this->spriteCount = 0;
	this->spriteWidth = 16;
	this->spriteHeight = 16;
	this->offsetX = 8;
	this->offsetY = 8;
	this->attributePadding = 1;
	this->terminator = SPAT_END;
	this->binaryOutput = 0;

	// Read arguments
	int i;
	if ((i = argStartsWith(argc, argv, "-w", 2)) != -1) {
		this->spriteWidth = decimalInt(&(argv[i][2]));
	}
	if ((i = argStartsWith(argc, argv, "-h", 2)) != -1) {
		this->spriteHeight = decimalInt(&(argv[i][2]));
	}
	this->offsetX = ((i = argStartsWith(argc, argv, "-x", 2)) != -1)
		? decimalInt(&(argv[i][2]))
		: this->spriteWidth / 2;
	this->offsetY = ((i = argStartsWith(argc, argv, "-y", 2)) != -1)
		? decimalInt(&(argv[i][2]))
		: this->spriteHeight / 2;
	if ((i = argStartsWith(argc, argv, "-p", 2)) != -1) {
		this->attributePadding = decimalDigit(argv[i][2]);
	}
	if ((i = argStartsWith(argc, argv, "-t", 2)) != -1) {
		this->terminator = hexadecimalInt(&(argv[i][2]));
	}
	this->binaryOutput = (argEquals(argc, argv, "-b") != -1);
}

void sprWriterPlusReadSprites(struct stSprWriterPlus *this, struct stBitmap *bitmap) {

	this->spriteCount = ((int) (bitmap->width / this->spriteWidth)) * ((int) (bitmap->height / this->spriteHeight));
	this->sprites = (struct stSpriteSolver*) calloc(this->spriteCount, sizeof(struct stSpriteSolver));

	unsigned int x, y;
	struct stSpriteSolver *solver;
	for (y = 0, solver = this->sprites; (y + this->spriteHeight) <= bitmap->height; y += this->spriteHeight) {
		for (x = 0; (x + this->spriteWidth) <= bitmap->width; x += this->spriteWidth, solver++) {
			solver->cfg = this; // reference
			solver->bitmap = bitmap; // reference
			solver->x0 = x;
			solver->y0 = y;
			solver->width = this->spriteWidth;
			solver->height = this->spriteHeight;

			if (!readSprite(solver)) continue;

			solve(solver);
		}
	}
}

int sprWriterPlusWrite(struct stSprWriterPlus *this, FILE *sprFile, FILE *spatFile) {

	int i, spriteIndex;
	struct stSpriteSolver *it;
	for (i = 0, spriteIndex = 0, it = this->sprites; i < this->spriteCount; i++, it++) {
		if (!stSpriteSolverWrite(it, &spriteIndex, sprFile, spatFile)) return 1;
		if (this->terminator == SPAT_END) spriteIndex = 0;
	}

	if ((!this->binaryOutput) && (!asmComment(spatFile, "EOF", 0))) return 2;

	return 0;
}

void sprWriterPlusDone(struct stSprWriterPlus *this) {

	if (this->sprites) {
		int i;
		struct stSpriteSolver *it;
		for (i = 0, it = this->sprites; i < this->spriteCount; i++, it++) {
			stSpriteSolverDone(it);
		}
		free(this->sprites);
	}
}

/* Private function bodies ----- struct stSpriteSolver --------------------- */

int readSprite(struct stSpriteSolver *this) {

	// Check if sprite present
	int x, y, colorCount;
	byte color;
	for (y = 0, colorCount = 0; y < this->height; y++) for (x = 0; x < this->width; x++) {
		if (!(color = getPixelAt(this, x, y))) continue;
		if (initColorSolver(this, color)) colorCount++;
	}

	if (veryVerbose && colorCount)
		printf("(%d,%d): %d color(s) found\n", this->x0, this->y0, colorCount);

	return colorCount;
}

int initColorSolver(struct stSpriteSolver *this, byte color) {

	struct stColorSolver *colorSolver = &(this->colorSolver[color]);
	if (colorSolver->color) return 0; // already init

	this->colorSolver[color].spriteSolver = this; // reference
	this->colorSolver[color].color = color; // reference
	return 1;
}

void solve(struct stSpriteSolver *this) {

	// Find solutions per color
	byte color;
	struct stColorSolver *colorSolver;
	for (color = 1; color < 16; color++) {
		if (!(colorSolver = getColorSolver(this, color))) continue;

		findRects(colorSolver);
	}
	if (veryVerbose) printf("%d sprites ", getSpriteCount(this));

	// Find best combined solution
	this->bestScanlineCount.value = getSpriteCount(this) + 1; // worst case
	this->bestScanlineCount.scanlineCount = this->height; // worst case
	int tmp[16] = {0};
	do {
		checkBetterSolution(this, tmp);
	} while (nextSolution(this, tmp));

	if (!veryVerbose) return;

	// Show best solution
	printf("(max %d at %d scanline(s)):\n",
		this->bestScanlineCount.value, this->bestScanlineCount.scanlineCount);
	printf("\t");
	for (color = 1; color < 16; color++) {
		if (!(colorSolver = getColorSolver(this, color))) continue;

		int solutionIndex, i;
		struct stRect *rect;
		for (solutionIndex = this->solutionIndexes[color], i = 0;
				i < colorSolver->rectsPerSolution;
				i++, solutionIndex++) {
			rect = &colorSolver->rects[solutionIndex];
			printf("(%d,%d)#%d ", rect->x, rect->y, colorSolver->color);
		}
	}
	printf("SPAT_END\n\n");
}

int checkBetterSolution(struct stSpriteSolver *this, int indexes[16]) {

	struct stScanlineCount scanlineCount = {0};

	// For each scanline
	int y;
	for (y = 0; y < this->height; y++) {
		// For each color
		byte color;
		int count;
		struct stColorSolver *colorSolver;
		for (color = 1, count = 0; color < 16; color++) {
			if (!(colorSolver = getColorSolver(this, color))) continue;
			count += getColorScanlineCount(colorSolver, indexes[color], y);
		}

		// worse? then quit
		if (count > this->bestScanlineCount.value) {
			return 0;
		}

		// update
		if (count > scanlineCount.value) {
			scanlineCount.value = count;
			scanlineCount.scanlineCount = 0;
		}
		if (count == scanlineCount.value)
			scanlineCount.scanlineCount++;
	}

	if ((scanlineCount.value > this->bestScanlineCount.value) // worse spritesPerScanline
		|| ((scanlineCount.value == this->bestScanlineCount.value) // same spritesPerScanline...
			&& (scanlineCount.scanlineCount >= this->bestScanlineCount.scanlineCount))) // ...but worse scanlineCount
				return 0; // not better

	// Update best solution
	byte color;
	for (color = 0; color < 16; color++) {
		this->solutionIndexes[color] = indexes[color];
	}
	this->bestScanlineCount.value = scanlineCount.value;
	this->bestScanlineCount.scanlineCount = scanlineCount.scanlineCount;
	return 1;
}

int nextSolution(struct stSpriteSolver *this, int indexes[16]) {

	byte color;
	struct stColorSolver *colorSolver;
	for (color = 1; color < 16; color++) {
		if (!(colorSolver = getColorSolver(this, color))) continue;

		indexes[color] += colorSolver->rectsPerSolution; // inc
		if (indexes[color] < colorSolver->rectCount) return 1; // no carry
		indexes[color] = 0; // carry
	}
	return 0; // overflow
}

byte getPixelAt(struct stSpriteSolver *this, int x, int y) {

	return ((x < 0) || (y < 0) || (x >= this->width) || (y >= this->height))
		? 0 // O.B.
		: bitmapGet(this->bitmap, x + this->x0, y + this->y0);
}

struct stColorSolver *getColorSolver(struct stSpriteSolver *this, byte color) {

	struct stColorSolver *colorSolver = &(this->colorSolver[color]);
	if (!colorSolver->color) return NULL;
	return colorSolver;
}

int getSpriteCount(struct stSpriteSolver *this) {

	int spriteCount = 0;

	byte color;
	struct stColorSolver *colorSolver;
	for (color = 1; color < 16; color++) {
		if (!(colorSolver = getColorSolver(this, color))) continue;
		spriteCount += colorSolver->rectsPerSolution;
	}
	return spriteCount;
}

// struct stScanlineCount getScanlineCount(struct stSpriteSolver *this, int solutionIndexes[16]) {

	// struct stScanlineCount ret = {0};

	// // For each scanline
	// int y, count;
	// for (y = 0; y < this->height; y++) {
		// // For each color
		// byte color;
		// struct stColorSolver *colorSolver;
		// for (color = 1, count = 0; color < 16; color++) {
			// if (!(colorSolver = getColorSolver(this, color))) continue;
			// count += getColorScanlineCount(colorSolver, solutionIndexes[color], y);
		// }
		// if (count >= this->bestScanlineCount) {
			// // already worse; skip (fake worst value)
			// ret.value = count;
			// ret.scanlineCount = this->height;
			// break;
		// }
		// if (count > ret.value) {
			// ret.value = count;
			// ret.scanlineCount = 1;
		// } else if (count == ret.value)
			// ret.value++;
	// }

	// return ret;
// }

int stSpriteSolverWrite(struct stSpriteSolver *this, int *spriteIndex, FILE *sprFile, FILE *spatFile) {

	int n = getSpriteCount(this);
	if (!n) return 1;

	struct stSprite *buffer = (struct stSprite*) calloc(n, sizeof(struct stSprite));

	// Read pattern and attributes
	byte color;
	int solutionIndex, j;
	struct stSprite *sprite;
	struct stColorSolver *colorSolver;
	struct stRect *rect;
	for (color = 1, sprite = buffer; color < 16; color++) {
		if (!(colorSolver = getColorSolver(this, color))) continue;

		for (solutionIndex = this->solutionIndexes[color], j = 0;
				j < colorSolver->rectsPerSolution;
				j++, solutionIndex++, sprite++) {
			rect = &colorSolver->rects[solutionIndex];
			readPattern(sprite, colorSolver, rect);
			readAttributes(sprite, colorSolver, (*spriteIndex)++, rect);
		}
	}

	// Write pattern and attributes
	int i = 0;
	if (!writeHeader(this, sprFile, spatFile)) goto out;
	for (j = 0, sprite = buffer; j < n; j++, sprite++) {
		if (!writePattern(sprite, this->cfg, sprFile)) goto out;
		if (!writeAttributes(sprite, this->cfg, spatFile)) goto out;
	}
	if (!writePadding(this, spatFile)) goto out;

	i = 1;
out:
	// Exit gracefully
	free(buffer);
	return i;
}

int writeHeader(struct stSpriteSolver *this, FILE *sprFile, FILE *spatFile) {

	if (this->cfg->binaryOutput) return 1;

	int i = 0;
	char *buffer = (char*) calloc(32, sizeof(char));

	// comment
	sprintf(buffer, "%d sprite(s) at %d,%d", getSpriteCount(this), this->x0, this->y0);
	if (!(i = asmComment(sprFile, buffer, 0))) goto out;
	if (!(i = asmComment(spatFile, buffer, 0))) goto out;

	// label
	sprintf(buffer, ".SPRITE_%d_%d", this->x0, this->y0);
	if (!(i = asmLabel(sprFile, buffer))) goto out;
	if (!(i = asmLabel(spatFile, buffer))) goto out;
out:
	// Exit gracefully
	free(buffer);
	return i;
}

int writePadding(struct stSpriteSolver *this, FILE *spatFile) {

	if (!this->cfg->attributePadding) return 1;

	byte *paddingBuffer = (byte*) calloc(this->cfg->attributePadding, sizeof(byte));
	paddingBuffer[0] = this->cfg->terminator;

	return (this->cfg->binaryOutput)
		? (fwrite(paddingBuffer, sizeof(byte), this->cfg->attributePadding, spatFile) == (size_t) this->cfg->attributePadding)
		: (asmComment(spatFile, "padding", 1)
			&& asmBytes(spatFile, paddingBuffer, this->cfg->attributePadding)
			&& asmNewLine(spatFile));
}

void stSpriteSolverDone(struct stSpriteSolver *this) {

	int i;
	struct stColorSolver *it;
	for (i = 0, it = this->colorSolver; i < 15; i++, it++) {
		stColorSolverDone(it);
	}
}

/* Private function bodies ----- struct stColorSolver ---------------------- */

void findRects(struct stColorSolver *this) {

	if (veryVerbose) printf("\t#%d: ", this->color);

	for (this->rectsPerSolution = 1; ; this->rectsPerSolution++) {
		if (veryVerbose) printf("%dx ", this->rectsPerSolution);

		// alloc
		struct stRect *rects = (struct stRect*) calloc(this->rectsPerSolution, sizeof(struct stRect));

		// init for this size
		int depth;
		for (depth = 0; depth < this->rectsPerSolution; depth++)
			if (!initRect(this, rects, depth))
				break; // couldn't init for this size
		if (depth < this->rectsPerSolution) {
			free(rects);
			continue; // couldn't init for this size
		}

		do {
			if (checkSolution(this, rects))
				saveSolution(this, rects);
		} while (moveSolution(this, rects));

		// dealloc
		free(rects);

		// solutions found?
		if (this->rectCount) break; // yes
	}

	if (veryVerbose) printf("%d solution(s)\n", this->rectCount / this->rectsPerSolution);
}

int initRect(struct stColorSolver *this, struct stRect *rects, int depth) {

	rects[depth].x = depth ? rects[depth - 1].x : 0;
	rects[depth].y = depth ? rects[depth - 1].y : -15;

	int flags;
	while (!((flags = checkRect(this, rects, depth)) & PIXEL_COVERED)) {
		if (!moveRect(this, rects, depth, flags)) {
			return 0;
		}
	}

	return 1;
}

int checkRect(struct stColorSolver *this, struct stRect *rects, int depth) {

	int flags = 0;

	int x, y, height = this->spriteSolver->height, width = this->spriteSolver->width;
	for (y = 0; y < height; y++) for (x = 0; x < width; x++) {
		if (getPixelAt(this->spriteSolver, x, y) != this->color)
			continue;

		// Covered by previous rects?
		int i;
		struct stRect *rect;
		for (i = 0, rect = rects; i < depth; i++, rect++) {
			if ((x >= rect->x) && (x < rect->x + 16) && (y >= rect->y) && (y < rect->y + 16))
				break;
		}
		if (i < depth) continue; // yes
		// no

		// note: rect == &(rects[depth]) now
		if (y < rect->y)	return flags | OVER_TOP; // quickly stop
		if (x < rect->x)	return flags | TO_THE_LEFT; // quickly skip scanline
		if (x >= rect->x + 16)	return flags | TO_THE_RIGHT; // move right
		if (y >= rect->y + 16)	return flags | UNDER_BOTTOM; // move bottom
		flags |= PIXEL_COVERED; // mark as "valid" new position
	}

	return flags;
}

int moveRect(struct stColorSolver *this, struct stRect *rects, int depth, int flags) {

	struct stRect *rect = &(rects[depth]);

	// over top: invalid
	if (flags & OVER_TOP)
		return 0;

	// to the left and is the last layer: it is safe to skip to the next scanline
	if ((flags & TO_THE_LEFT) && (depth == this->rectsPerSolution - 1)) {
		goto skipScanline;
	}

	// to the left or right: move to the right then to the next scanline
	if (flags & (TO_THE_LEFT | TO_THE_RIGHT)) {
		// (no skip to next scanline if TO_THE_LEFT
		// because next layer can deal with those pixels)
		if (++rect->x <= this->spriteSolver->width -16)
			return 1; // right
		// down (falls through)
	}

skipScanline:
	rect->x = 0;
	return (++rect->y < this->spriteSolver->height);
}

int checkSolution(struct stColorSolver *this, struct stRect *rects) {

	int x, y, height = this->spriteSolver->height, width = this->spriteSolver->width;
	for (y = 0; y < height; y++) for (x = 0; x < width; x++) {
		if (getPixelAt(this->spriteSolver, x, y) != this->color)
			continue;

		// Is covered by rects?
		int i;
		struct stRect *rect;
		for (i = 0, rect = rects; i < this->rectsPerSolution; i++, rect++) {
			if ((x >= rect->x) && (x < rect->x + 16) && (y >= rect->y) && (y < rect->y + 16))
				break;
		}
		if (i == this->rectsPerSolution) return 0; // no
	}

	return 1;
}

int moveSolution(struct stColorSolver *this, struct stRect *rects) {

	int depth;
	for (depth = this->rectsPerSolution - 1; depth >= 0; depth--) {

		// Move depth layer
		int moved;
		int flags = checkRect(this, rects, depth);
		do {
			moved = moveRect(this, rects, depth, flags);
		} while (moved && (!((flags = checkRect(this, rects, depth)) & PIXEL_COVERED)));

		// Moved? then reset depth+1..n layers
		if (moved) {
			// printf("m%d ", depth);
			int i;
			for (i = depth + 1; i < this->rectsPerSolution; i++) {
				if (!initRect(this, rects, i))
					break; // couldn't reset layer
				// printf("i%d ", i);
			}
			if (i == this->rectsPerSolution) return 1; // layers reset
		}

		// Not moved? then move depth-1 layer
	}
	return 0; // no more layers to move
}

void saveSolution(struct stColorSolver *this, struct stRect *rects) {

	int i;
	struct stRect *rect;
	for (i = 0, rect = rects; i < this->rectsPerSolution; i++, rect++) {
		addLayer(&this->rects, &this->rectCount, rect);
	}
}

int getColorScanlineCount(struct stColorSolver *this, int index, int y) {

	int count = 0;

	// For each rect
	int i;
	struct stRect *layer;
	for (i = 0; i < this->rectsPerSolution; i++) {
		layer = &(this->rects[index + i]);
		if ((y >= layer->y) && (y <= layer->y + 15)) count++;
	}

	return count;
}

void stColorSolverDone(struct stColorSolver *this) {

	if (this->rects) free(this->rects);
}

/* Private function bodies ----- struct stRect ----------------------------- */

void addLayer(struct stRect **rects, int *count, struct stRect *src) {

	(*count)++;
	(*rects) = (*rects)
		? (struct stRect*) realloc((*rects), (*count) * sizeof(struct stRect))
		: (struct stRect*) calloc((*count), sizeof(struct stRect));

	struct stRect *rect = &(*rects)[(*count) - 1]; // last rect
	rect->x = src->x;
	rect->y = src->y;
}

void readPattern(struct stSprite *this, struct stColorSolver *colorSolver, struct stRect *rect) {

	int i0, i1, k, x0, x1, y;
	for (i0 = 0, i1 = 16, y = rect->y; i0 < 16; i0++, i1++, y++) {
		for (k = 7, x0 = rect->x, x1 = rect->x + 8; x0 < rect->x + 8; k--, x0++, x1++) {
			if (getPixelAt(colorSolver->spriteSolver, x0, y) == colorSolver->color)
				this->pattern[i0] |= (1 << k);
			if (getPixelAt(colorSolver->spriteSolver, x1, y) == colorSolver->color)
				this->pattern[i1] |= (1 << k);
		}
	}
}

void readAttributes(struct stSprite *this, struct stColorSolver *colorSolver, int spriteIndex, struct stRect *rect) {

	struct stSprWriterPlus *cfg = colorSolver->spriteSolver->cfg;

	this->attributes[0] = (byte) (rect->y - cfg->offsetY);
	this->attributes[1] = (byte) (rect->x - cfg->offsetX);
	this->attributes[2] = (byte) (spriteIndex << 2);
	this->attributes[3] = (byte) (colorSolver->color);
}

int writePattern(struct stSprite *this, struct stSprWriterPlus *cfg, FILE *sprFile) {

	return (cfg->binaryOutput)
		? fwrite(this->pattern, sizeof(byte), 32, sprFile) == 32
		: asmBytes(sprFile, this->pattern, 16)
			&& asmBytes(sprFile, &(this->pattern[16]), 16);
}

int writeAttributes(struct stSprite *this, struct stSprWriterPlus *cfg, FILE *spatFile) {

	return (cfg->binaryOutput)
		? (fwrite(this->attributes, sizeof(byte), 4, spatFile) == 4)
		: asmBytes(spatFile, this->attributes, 4);
}

/* Private function bodies ----- DEBUG ------------------------------------- */

void debugSolution(struct stColorSolver __UNUSED_PARAM(*this), struct stRect *rects, int size) {

	int i;
	struct stRect *rect;
	for (i = 0, rect = rects; i < size; i++, rect++) {
		printf("(%d,%d)-(%d,%d)\t", rect->x, rect->y, rect->x+15, rect->y+15);
	}
	// printf("\n");
}

// EOF
