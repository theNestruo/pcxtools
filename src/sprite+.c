/*
 * Additional support routines for managing sprites
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "output.h"
#include "sprite+.h"

/* Symbolic constants ------------------------------------------------------ */

#define SPAT_END (0xd0)
#define SPAT_OB (0xd1)

#define OVER_TOP (0x01)     // pixels over the layer
#define UNDER_BOTTOM (0x02) // pixels under the layer
#define TO_THE_LEFT (0x04)  // pixels to the left of the layer in the same band
#define TO_THE_RIGHT (0x08) // pixels to the right of the layer in the same band
#define PIXEL_COVERED (0x10) // pixels covered by the layer

/* Global vars ------------------------------------------------------------- */

extern int verbose;
extern int veryVerbose;

/* Private data structures ------------------------------------------------- */

typedef struct {
  uint8_t pattern[32];
  uint8_t attributes[4];
} Sprite;

/* Private function prototypes --------------------------------------------- */

/* Private function prototypes ----- SpriteSolver ----------------- */

int readSprite(SpriteSolver *instance);
int initColorSolver(SpriteSolver *instance, uint8_t color);
void solve(SpriteSolver *instance);
int checkBetterSolution(SpriteSolver *instance, int indexes[16]);
int nextSolution(SpriteSolver *instance, int indexes[16]);

uint8_t getPixelAt(SpriteSolver *instance, int x, int y);
ColorSolver *getColorSolver(SpriteSolver *instance, uint8_t color);
int getSpriteCount(SpriteSolver *instance);
ScanlineCount getScanlineCount(SpriteSolver *instance, int indexes[16]);
int stSpriteSolverWrite(SpriteSolver *instance, int *spriteIndex, FILE *sprFile,
                        FILE *spatFile);
int writeHeader(SpriteSolver *instance, FILE *sprFile, FILE *spatFile);
int writePadding(SpriteSolver *instance, FILE *spatFile);
void stSpriteSolverDone(SpriteSolver *instance);

/* Private function prototypes ----- ColorSolver ------------------ */

void findRects(ColorSolver *instance);
int initRect(ColorSolver *instance, Rect *rects, int depth);
int checkRect(ColorSolver *instance, Rect *rects, int depth);
int moveRect(ColorSolver *instance, Rect *rects, int depth, int flags);
int checkSolution(ColorSolver *instance, Rect *rects);
int moveSolution(ColorSolver *instance, Rect *rects);
void saveSolution(ColorSolver *instance, Rect *rects);
int getColorScanlineCount(ColorSolver *instance, int index, int y);
void stColorSolverDone(ColorSolver *instance);

/* Private function prototypes ----- Rect ------------------------- */

void addLayer(Rect **rects, int *count, Rect *src);

/* Private function prototypes ----- Sprite ----------------------- */

void readPattern(Sprite *instance, ColorSolver *colorSolver, Rect *rect);
void readAttributes(Sprite *instance, ColorSolver *colorSolver, int spriteIndex,
                    Rect *rect);
int writePattern(Sprite *instance, SprWriterPlus *cfg, FILE *sprFile);
int writeAttributes(Sprite *instance, SprWriterPlus *cfg, FILE *spatFile);

/* Private function prototypes ----- DEBUG --------------------------------- */

void debugSolution(ColorSolver *instance, Rect *rects, int size);

/* Function bodies --------------------------------------------------------- */

void sprWriterPlusOptions() {

  printf("\t-w<16..255>\tsprite width (default: 16px)\n");
  printf("\t-h<16..255>\tsprite height (default: 16px)\n");
  printf("\t-x<0..255>\tX offset (default: center)\n");
  printf("\t-y<0..255>\tY offset (default: middle)\n");
  printf("\t-p<0..4>\tattribute padding size (default: 1b)\n");
  printf("\t-t<00..ff>\tterminator uint8_t (default: 0xD0 (SPAT_END))\n");
  printf("\t-b\tbinary spr/spat output (default: asm)\n");
  // printf("\t-vv\tVERY verbose execution\n"); // secret option (!^_^)
}

void sprWriterPlusInit(SprWriterPlus *this, int argc, char **argv) {

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

void sprWriterPlusReadSprites(SprWriterPlus *this, Bitmap *bitmap) {

  this->spriteCount = ((int)(bitmap->width / this->spriteWidth)) *
                      ((int)(bitmap->height / this->spriteHeight));
  this->sprites =
      (SpriteSolver *)calloc(this->spriteCount, sizeof(SpriteSolver));

  unsigned int x, y;
  SpriteSolver *solver;
  for (y = 0, solver = this->sprites;
       (y + this->spriteHeight) <= bitmap->height; y += this->spriteHeight) {
    for (x = 0; (x + this->spriteWidth) <= bitmap->width;
         x += this->spriteWidth, solver++) {
      solver->cfg = this;      // reference
      solver->bitmap = bitmap; // reference
      solver->x0 = x;
      solver->y0 = y;
      solver->width = this->spriteWidth;
      solver->height = this->spriteHeight;

      if (!readSprite(solver))
        continue;

      solve(solver);
    }
  }
}

int sprWriterPlusWrite(SprWriterPlus *this, FILE *sprFile, FILE *spatFile) {

  int i, spriteIndex;
  SpriteSolver *it;
  for (i = 0, spriteIndex = 0, it = this->sprites; i < this->spriteCount;
       i++, it++) {
    if (!stSpriteSolverWrite(it, &spriteIndex, sprFile, spatFile))
      return 1;
    if (this->terminator == SPAT_END)
      spriteIndex = 0;
  }

  if ((!this->binaryOutput) && (!asmComment(spatFile, "EOF", 0)))
    return 2;

  return 0;
}

void sprWriterPlusDone(SprWriterPlus *this) {

  if (this->sprites) {
    int i;
    SpriteSolver *it;
    for (i = 0, it = this->sprites; i < this->spriteCount; i++, it++) {
      stSpriteSolverDone(it);
    }
    free(this->sprites);
  }
}

/* Private function bodies ----- SpriteSolver --------------------- */

int readSprite(SpriteSolver *this) {

  // Check if sprite present
  int x, y, colorCount;
  uint8_t color;
  for (y = 0, colorCount = 0; y < this->height; y++)
    for (x = 0; x < this->width; x++) {
      if (!(color = getPixelAt(this, x, y)))
        continue;
      if (initColorSolver(this, color))
        colorCount++;
    }

  if (veryVerbose && colorCount)
    printf("(%d,%d): %d color(s) found\n", this->x0, this->y0, colorCount);

  return colorCount;
}

int initColorSolver(SpriteSolver *this, uint8_t color) {

  ColorSolver *colorSolver = &(this->colorSolver[color]);
  if (colorSolver->color)
    return 0; // already init

  this->colorSolver[color].spriteSolver = this; // reference
  this->colorSolver[color].color = color;       // reference
  return 1;
}

void solve(SpriteSolver *this) {

  // Find solutions per color
  uint8_t color;
  ColorSolver *colorSolver;
  for (color = 1; color < 16; color++) {
    if (!(colorSolver = getColorSolver(this, color)))
      continue;

    findRects(colorSolver);
  }
  if (veryVerbose)
    printf("%d sprites ", getSpriteCount(this));

  // Find best combined solution
  this->bestScanlineCount.value = getSpriteCount(this) + 1; // worst case
  this->bestScanlineCount.scanlineCount = this->height;     // worst case
  int tmp[16] = {0};
  do {
    checkBetterSolution(this, tmp);
  } while (nextSolution(this, tmp));

  if (!veryVerbose)
    return;

  // Show best solution
  printf("(max %d at %d scanline(s)):\n", this->bestScanlineCount.value,
         this->bestScanlineCount.scanlineCount);
  printf("\t");
  for (color = 1; color < 16; color++) {
    if (!(colorSolver = getColorSolver(this, color)))
      continue;

    int solutionIndex, i;
    Rect *rect;
    for (solutionIndex = this->solutionIndexes[color], i = 0;
         i < colorSolver->rectsPerSolution; i++, solutionIndex++) {
      rect = &colorSolver->rects[solutionIndex];
      printf("(%d,%d)#%d ", rect->x, rect->y, colorSolver->color);
    }
  }
  printf("SPAT_END\n\n");
}

int checkBetterSolution(SpriteSolver *this, int indexes[16]) {

  ScanlineCount scanlineCount = {0};

  // For each scanline
  int y;
  for (y = 0; y < this->height; y++) {
    // For each color
    uint8_t color;
    int count;
    ColorSolver *colorSolver;
    for (color = 1, count = 0; color < 16; color++) {
      if (!(colorSolver = getColorSolver(this, color)))
        continue;
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

  if ((scanlineCount.value >
       this->bestScanlineCount.value) // worse spritesPerScanline
      ||
      ((scanlineCount.value ==
        this->bestScanlineCount.value) // same spritesPerScanline...
       &&
       (scanlineCount.scanlineCount >=
        this->bestScanlineCount.scanlineCount))) // ...but worse scanlineCount
    return 0;                                    // not better

  // Update best solution
  uint8_t color;
  for (color = 0; color < 16; color++) {
    this->solutionIndexes[color] = indexes[color];
  }
  this->bestScanlineCount.value = scanlineCount.value;
  this->bestScanlineCount.scanlineCount = scanlineCount.scanlineCount;
  return 1;
}

int nextSolution(SpriteSolver *this, int indexes[16]) {

  uint8_t color;
  ColorSolver *colorSolver;
  for (color = 1; color < 16; color++) {
    if (!(colorSolver = getColorSolver(this, color)))
      continue;

    indexes[color] += colorSolver->rectsPerSolution; // inc
    if (indexes[color] < colorSolver->rectCount)
      return 1;         // no carry
    indexes[color] = 0; // carry
  }
  return 0; // overflow
}

uint8_t getPixelAt(SpriteSolver *this, int x, int y) {

  return ((x < 0) || (y < 0) || (x >= this->width) || (y >= this->height))
             ? 0 // O.B.
             : bitmapGet(this->bitmap, x + this->x0, y + this->y0);
}

ColorSolver *getColorSolver(SpriteSolver *this, uint8_t color) {

  ColorSolver *colorSolver = &(this->colorSolver[color]);
  if (!colorSolver->color)
    return NULL;
  return colorSolver;
}

int getSpriteCount(SpriteSolver *this) {

  int spriteCount = 0;

  uint8_t color;
  ColorSolver *colorSolver;
  for (color = 1; color < 16; color++) {
    if (!(colorSolver = getColorSolver(this, color)))
      continue;
    spriteCount += colorSolver->rectsPerSolution;
  }
  return spriteCount;
}

// ScanlineCount getScanlineCount(SpriteSolver *this, int solutionIndexes[16]) {

// ScanlineCount ret = {0};

// // For each scanline
// int y, count;
// for (y = 0; y < this->height; y++) {
// // For each color
// uint8_t color;
// ColorSolver *colorSolver;
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

int stSpriteSolverWrite(SpriteSolver *this, int *spriteIndex, FILE *sprFile,
                        FILE *spatFile) {

  int n = getSpriteCount(this);
  if (!n)
    return 1;

  Sprite *buffer = (Sprite *)calloc(n, sizeof(Sprite));

  // Read pattern and attributes
  uint8_t color;
  int solutionIndex, j;
  Sprite *sprite;
  ColorSolver *colorSolver;
  Rect *rect;
  for (color = 1, sprite = buffer; color < 16; color++) {
    if (!(colorSolver = getColorSolver(this, color)))
      continue;

    for (solutionIndex = this->solutionIndexes[color], j = 0;
         j < colorSolver->rectsPerSolution; j++, solutionIndex++, sprite++) {
      rect = &colorSolver->rects[solutionIndex];
      readPattern(sprite, colorSolver, rect);
      readAttributes(sprite, colorSolver, (*spriteIndex)++, rect);
    }
  }

  // Write pattern and attributes
  int i = 0;
  if (!writeHeader(this, sprFile, spatFile))
    goto out;
  for (j = 0, sprite = buffer; j < n; j++, sprite++) {
    if (!writePattern(sprite, this->cfg, sprFile))
      goto out;
    if (!writeAttributes(sprite, this->cfg, spatFile))
      goto out;
  }
  if (!writePadding(this, spatFile))
    goto out;

  i = 1;
out:
  // Exit gracefully
  free(buffer);
  return i;
}

int writeHeader(SpriteSolver *this, FILE *sprFile, FILE *spatFile) {

  if (this->cfg->binaryOutput)
    return 1;

  int i = 0;
  char *buffer = (char *)calloc(32, sizeof(char));

  // comment
  sprintf(buffer, "%d sprite(s) at %d,%d", getSpriteCount(this), this->x0,
          this->y0);
  if (!(i = asmComment(sprFile, buffer, 0)))
    goto out;
  if (!(i = asmComment(spatFile, buffer, 0)))
    goto out;

  // label
  sprintf(buffer, ".SPRITE_%d_%d", this->x0, this->y0);
  if (!(i = asmLabel(sprFile, buffer)))
    goto out;
  if (!(i = asmLabel(spatFile, buffer)))
    goto out;
out:
  // Exit gracefully
  free(buffer);
  return i;
}

int writePadding(SpriteSolver *this, FILE *spatFile) {

  if (!this->cfg->attributePadding)
    return 1;

  uint8_t *paddingBuffer =
      (uint8_t *)calloc(this->cfg->attributePadding, sizeof(uint8_t));
  paddingBuffer[0] = this->cfg->terminator;

  return (this->cfg->binaryOutput)
             ? (fwrite(paddingBuffer, sizeof(uint8_t),
                       this->cfg->attributePadding,
                       spatFile) == (size_t)this->cfg->attributePadding)
             : (asmComment(spatFile, "padding", 1) &&
                asmBytes(spatFile, paddingBuffer,
                         this->cfg->attributePadding) &&
                asmNewLine(spatFile));
}

void stSpriteSolverDone(SpriteSolver *this) {

  int i;
  ColorSolver *it;
  for (i = 0, it = this->colorSolver; i < 15; i++, it++) {
    stColorSolverDone(it);
  }
}

/* Private function bodies ----- ColorSolver ---------------------- */

void findRects(ColorSolver *this) {

  if (veryVerbose)
    printf("\t#%d: ", this->color);

  for (this->rectsPerSolution = 1;; this->rectsPerSolution++) {
    if (veryVerbose)
      printf("%dx ", this->rectsPerSolution);

    // alloc
    Rect *rects = (Rect *)calloc(this->rectsPerSolution, sizeof(Rect));

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
    if (this->rectCount)
      break; // yes
  }

  if (veryVerbose)
    printf("%d solution(s)\n", this->rectCount / this->rectsPerSolution);
}

int initRect(ColorSolver *this, Rect *rects, int depth) {

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

int checkRect(ColorSolver *this, Rect *rects, int depth) {

  int flags = 0;

  int x, y, height = this->spriteSolver->height,
            width = this->spriteSolver->width;
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      if (getPixelAt(this->spriteSolver, x, y) != this->color)
        continue;

      // Covered by previous rects?
      int i;
      Rect *rect;
      for (i = 0, rect = rects; i < depth; i++, rect++) {
        if ((x >= rect->x) && (x < rect->x + 16) && (y >= rect->y) &&
            (y < rect->y + 16))
          break;
      }
      if (i < depth)
        continue; // yes
      // no

      // note: rect == &(rects[depth]) now
      if (y < rect->y)
        return flags | OVER_TOP; // quickly stop
      if (x < rect->x)
        return flags | TO_THE_LEFT; // quickly skip scanline
      if (x >= rect->x + 16)
        return flags | TO_THE_RIGHT; // move right
      if (y >= rect->y + 16)
        return flags | UNDER_BOTTOM; // move bottom
      flags |= PIXEL_COVERED;        // mark as "valid" new position
    }

  return flags;
}

int moveRect(ColorSolver *this, Rect *rects, int depth, int flags) {

  Rect *rect = &(rects[depth]);

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
    if (++rect->x <= this->spriteSolver->width - 16)
      return 1; // right
                // down (falls through)
  }

skipScanline:
  rect->x = 0;
  return (++rect->y < this->spriteSolver->height);
}

int checkSolution(ColorSolver *this, Rect *rects) {

  int x, y, height = this->spriteSolver->height,
            width = this->spriteSolver->width;
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      if (getPixelAt(this->spriteSolver, x, y) != this->color)
        continue;

      // Is covered by rects?
      int i;
      Rect *rect;
      for (i = 0, rect = rects; i < this->rectsPerSolution; i++, rect++) {
        if ((x >= rect->x) && (x < rect->x + 16) && (y >= rect->y) &&
            (y < rect->y + 16))
          break;
      }
      if (i == this->rectsPerSolution)
        return 0; // no
    }

  return 1;
}

int moveSolution(ColorSolver *this, Rect *rects) {

  int depth;
  for (depth = this->rectsPerSolution - 1; depth >= 0; depth--) {

    // Move depth layer
    int moved;
    int flags = checkRect(this, rects, depth);
    do {
      moved = moveRect(this, rects, depth, flags);
    } while (moved &&
             (!((flags = checkRect(this, rects, depth)) & PIXEL_COVERED)));

    // Moved? then reset depth+1..n layers
    if (moved) {
      // printf("m%d ", depth);
      int i;
      for (i = depth + 1; i < this->rectsPerSolution; i++) {
        if (!initRect(this, rects, i))
          break; // couldn't reset layer
                 // printf("i%d ", i);
      }
      if (i == this->rectsPerSolution)
        return 1; // layers reset
    }

    // Not moved? then move depth-1 layer
  }
  return 0; // no more layers to move
}

void saveSolution(ColorSolver *this, Rect *rects) {

  int i;
  Rect *rect;
  for (i = 0, rect = rects; i < this->rectsPerSolution; i++, rect++) {
    addLayer(&this->rects, &this->rectCount, rect);
  }
}

int getColorScanlineCount(ColorSolver *this, int index, int y) {

  int count = 0;

  // For each rect
  int i;
  Rect *layer;
  for (i = 0; i < this->rectsPerSolution; i++) {
    layer = &(this->rects[index + i]);
    if ((y >= layer->y) && (y <= layer->y + 15))
      count++;
  }

  return count;
}

void stColorSolverDone(ColorSolver *this) {

  if (this->rects)
    free(this->rects);
}

/* Private function bodies ----- Rect ----------------------------- */

void addLayer(Rect **rects, int *count, Rect *src) {

  (*count)++;
  (*rects) = (*rects) ? (Rect *)realloc((*rects), (*count) * sizeof(Rect))
                      : (Rect *)calloc((*count), sizeof(Rect));

  Rect *rect = &(*rects)[(*count) - 1]; // last rect
  rect->x = src->x;
  rect->y = src->y;
}

void readPattern(Sprite *this, ColorSolver *colorSolver, Rect *rect) {

  int i0, i1, k, x0, x1, y;
  for (i0 = 0, i1 = 16, y = rect->y; i0 < 16; i0++, i1++, y++) {
    for (k = 7, x0 = rect->x, x1 = rect->x + 8; x0 < rect->x + 8;
         k--, x0++, x1++) {
      if (getPixelAt(colorSolver->spriteSolver, x0, y) == colorSolver->color)
        this->pattern[i0] |= (1 << k);
      if (getPixelAt(colorSolver->spriteSolver, x1, y) == colorSolver->color)
        this->pattern[i1] |= (1 << k);
    }
  }
}

void readAttributes(Sprite *this, ColorSolver *colorSolver, int spriteIndex,
                    Rect *rect) {

  SprWriterPlus *cfg = colorSolver->spriteSolver->cfg;

  this->attributes[0] = (uint8_t)(rect->y - cfg->offsetY);
  this->attributes[1] = (uint8_t)(rect->x - cfg->offsetX);
  this->attributes[2] = (uint8_t)(spriteIndex << 2);
  this->attributes[3] = (uint8_t)(colorSolver->color);
}

int writePattern(Sprite *this, SprWriterPlus *cfg, FILE *sprFile) {

  return (cfg->binaryOutput)
             ? fwrite(this->pattern, sizeof(uint8_t), 32, sprFile) == 32
             : asmBytes(sprFile, this->pattern, 16) &&
                   asmBytes(sprFile, &(this->pattern[16]), 16);
}

int writeAttributes(Sprite *this, SprWriterPlus *cfg, FILE *spatFile) {

  return (cfg->binaryOutput)
             ? (fwrite(this->attributes, sizeof(uint8_t), 4, spatFile) == 4)
             : asmBytes(spatFile, this->attributes, 4);
}

/* Private function bodies ----- DEBUG ------------------------------------- */

void debugSolution(__attribute__((unused)) ColorSolver *this, Rect *rects,
                   int size) {

  int i;
  Rect *rect;
  for (i = 0, rect = rects; i < size; i++, rect++) {
    printf("(%d,%d)-(%d,%d)\t", rect->x, rect->y, rect->x + 15, rect->y + 15);
  }
  // printf("\n");
}

// EOF
