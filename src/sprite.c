/*
 * Support routines for managing sprites in TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "sprite.h"

/* Symbolic constants ------------------------------------------------------ */

#define COLOR_MODE_HIGH_LOW (0x01)
#define COLOR_MODE_LOW_HIGH (0x02)
#define COLOR_MODE_LIGHT_DARK (0x03)
#define COLOR_MODE_DARK_LIGHT (0x04)

/* Private function prototypes --------------------------------------------- */

void processSpriteGroup(SprWriter *instance, SpriteGroup *group, Bitmap *bitmap, int x, int y);

/* Function bodies --------------------------------------------------------- */

// Supported arguments:
// -8	generate 8x8px sprites
// -h	generate half sprites (8x16px, 16b per sprite)
// -lh  higher colors will have higher priority planes (default)
// -hl  lower colors will have higher priority planes
// -ld	lighter colors will have higher priority planes
// -dl	darker colors will have higher priority planes
// -th  traverse spritesheet horizontally, then vertically (default)
// -tv  traverse spritesheet vertically, then horizontally
void sprWriterOptions() {

    printf("\t-8\tgenerate 8x8px sprites\n");
    printf("\t-h\tgenerate half sprites (8x16px, 16b per sprite)\n");
    printf("\t-hl\thigher colors will have higher priority planes (default)\n");
    printf("\t-lh\tlower colors will have higher priority planes\n");
    printf("\t-ld\tlighter colors will have higher priority planes\n");
    printf("\t-dl\tdarker colors will have higher priority planes\n");
    printf("\t-th\ttraverse spritesheet horizontally, then vertically (default)\n");
    printf("\t-tv\ttraverse spritesheet vertically, then horizontally\n");
}

void sprWriterInit(SprWriter *this, int argc, char **argv) {

    // Init
    this->groups = NULL;
    this->groupCount = 0;

    // Read arguments
    this->spriteWidth = (argEquals(argc, argv, "-8") != -1) ? 8 : (argEquals(argc, argv, "-h") != -1) ? 8 : 16;
    this->spriteHeight = (argEquals(argc, argv, "-8") != -1) ? 8 : 16;
    this->colorMode = (argEquals(argc, argv, "-hl") != -1)   ? COLOR_MODE_HIGH_LOW
                      : (argEquals(argc, argv, "-lh") != -1) ? COLOR_MODE_LOW_HIGH
                      : (argEquals(argc, argv, "-ld") != -1) ? COLOR_MODE_LIGHT_DARK
                      : (argEquals(argc, argv, "-dl") != -1) ? COLOR_MODE_DARK_LIGHT
                                                             : COLOR_MODE_HIGH_LOW;
    this->traverseHorizontally = (argEquals(argc, argv, "-th") != -1)   ? 1
                                 : (argEquals(argc, argv, "-tv") != -1) ? 0
                                                                        : 1;
}

void sprWriterReadSprites(SprWriter *this, Bitmap *bitmap) {

    this->groupCount = ((int)(bitmap->width / this->spriteWidth)) * ((int)(bitmap->height / this->spriteHeight));
    this->groups = (SpriteGroup *)calloc(this->groupCount, sizeof(SpriteGroup));

    unsigned int y, x;
    SpriteGroup *it;
    if (this->traverseHorizontally) {
        for (y = 0, it = this->groups; (y + this->spriteHeight) <= bitmap->height; y += this->spriteHeight) {
            for (x = 0; (x + this->spriteWidth) <= bitmap->width; x += this->spriteWidth, it++) {
                processSpriteGroup(this, it, bitmap, x, y);
            }
        }
    } else {
        for (x = 0, it = this->groups; (x + this->spriteWidth) <= bitmap->width; x += this->spriteWidth) {
            for (y = 0; (y + this->spriteHeight) <= bitmap->height; y += this->spriteHeight, it++) {
                processSpriteGroup(this, it, bitmap, x, y);
            }
        }
    }
}

int sprWriterWrite(SprWriter *this, FILE *sprFile) {

    size_t spriteSize = (this->spriteWidth / 8) * this->spriteHeight;

    // For each group...
    int i;
    SpriteGroup *it;
    for (i = 0, it = this->groups; i < this->groupCount; i++, it++) {

        // For each sprite...
        int j;
        Sprite *sprite;
        for (j = 0, sprite = it->sprites; j < it->spriteCount; j++, sprite++) {
            if (fwrite(sprite->pattern, sizeof(uint8_t), spriteSize, sprFile) != spriteSize) {
                return 2;
            }
        }
    }
    return 0;
}

void sprWriterDone(SprWriter *this) {

    if (!this->groups) {
        return;
    }

    // For each group...
    int i;
    SpriteGroup *it;
    for (i = 0, it = this->groups; i < this->groupCount; i++, it++) {
        if (!it->sprites) {
            continue;
        }

        // For each sprite...
        int j;
        Sprite *sprite;
        for (j = 0, sprite = it->sprites; j < it->spriteCount; j++, sprite++) {
            if (sprite->pattern) {
                free(sprite->pattern);
            }
        }
        free(it->sprites);
    }
    free(this->groups);
    this->groups = NULL;
}

/* Private function bodies ------------------------------------------------- */

void processSpriteGroup(SprWriter *this, SpriteGroup *group, Bitmap *bitmap, int x0, int y0) {

    int spriteSize = (this->spriteWidth / 8) * this->spriteHeight;

    // Create buffer and reset values
    int i, j;
    Sprite *buffer = (Sprite *)calloc(15, sizeof(Sprite));
    for (i = 0; i < 15; i++) {
        buffer[i].pattern = (uint8_t *)calloc(spriteSize, sizeof(uint8_t));
    }

    // Create the pattern
    int col, y, x;
    for (col = 0, j = 0; col < this->spriteWidth; col += 8) { // for each 8px width column
        for (y = 0; y < this->spriteHeight; y++, j++) {       // for each line
            for (x = 0; x < 8; x++) {                         // for each pixel
                uint8_t color = bitmapGet(bitmap, x0 + col + x, y0 + y);
                if (color) {
                    color = color - 1;
                    buffer[color].pattern[j] |= (0x80 >> x);
                }
            }
        }
    }

    // Count patterns
    group->spriteCount = 0;
    for (i = 0; i < 15; i++) {
        for (j = 0; j < spriteSize; j++) {
            if (buffer[i].pattern[j]) {
                group->spriteCount++;
                break;
            }
        }
    }

    // Blit patterns
    if (!group->spriteCount) {
        group->sprites = NULL;

    } else {

        // Brigthness by color, matching the color order: 0146C285D937ABEF
        const unsigned int colorOrderHighLow[] = {0xF - 1, 0xE - 1, 0xD - 1, 0xC - 1, 0xB - 1,
                                                  0xA - 1, 0x9 - 1, 0x8 - 1, 0x7 - 1, 0x6 - 1,
                                                  0x5 - 1, 0x4 - 1, 0x3 - 1, 0x2 - 1, 0x1 - 1};
        const unsigned int colorOrderLowHigh[] = {0x1 - 1, 0x2 - 1, 0x3 - 1, 0x4 - 1, 0x5 - 1,
                                                  0x6 - 1, 0x7 - 1, 0x8 - 1, 0x9 - 1, 0xA - 1,
                                                  0xB - 1, 0xC - 1, 0xD - 1, 0xE - 1, 0xF - 1};
        const unsigned int colorOrderLightDark[] = {0xF - 1, 0xE - 1, 0xB - 1, 0xA - 1, 0x7 - 1,
                                                    0x3 - 1, 0x9 - 1, 0xD - 1, 0x5 - 1, 0x8 - 1,
                                                    0x2 - 1, 0xC - 1, 0x6 - 1, 0x4 - 1, 0x1 - 1};
        const unsigned int colorOrderDarkLight[] = {0x1 - 1, 0x4 - 1, 0x6 - 1, 0xC - 1, 0x2 - 1,
                                                    0x8 - 1, 0x5 - 1, 0xD - 1, 0x9 - 1, 0x3 - 1,
                                                    0x7 - 1, 0xA - 1, 0xB - 1, 0xE - 1, 0xF - 1};
        const unsigned int *colorOrder = this->colorMode == COLOR_MODE_LOW_HIGH     ? colorOrderLowHigh
                                         : this->colorMode == COLOR_MODE_LIGHT_DARK ? colorOrderLightDark
                                         : this->colorMode == COLOR_MODE_DARK_LIGHT ? colorOrderDarkLight
                                                                                    : colorOrderHighLow;

        group->sprites = (Sprite *)calloc(group->spriteCount, sizeof(Sprite));
        Sprite *dest;
        for (i = 0, dest = group->sprites; i < 15; i++) {
            Sprite *src = &buffer[colorOrder[i]];

            int spriteFound = 0;
            for (j = 0; (j < spriteSize) && (!spriteFound); j++) {
                spriteFound |= src->pattern[j];
            }
            if (!spriteFound) {
                continue;
            }
            // blit sprite
            dest->pattern = (uint8_t *)calloc(spriteSize, sizeof(uint8_t));
            for (j = 0; j < spriteSize; j++) {
                dest->pattern[j] = src->pattern[j];
            }
            dest++;
        }
    }

    // Destroys buffer
    for (i = 0; i < 15; i++) {
        free(buffer[i].pattern);
    }
    free(buffer);
}
