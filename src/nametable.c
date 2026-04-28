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

void reallocateBlocks(Charset *charset, int newBlockCount);

/* Function bodies --------------------------------------------------------- */

void nameTableProcessorOptions() {

    printf("\t-n[<00..ff>]\tgenerate NAMTBL [starting at value <n>]\n");
    printf("\t-bb<00..ff>\tblank block at position <nn>\n");
    printf("\t-rr\tremove repeated tiles\n");
    printf("\t-rm<0..f>\tremove solid tiles of <n> color\n");
}

void nameTableProcessorInit(NameTableProcessor *this, int argc, char **argv) {

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
    this->traverseHorizontally = (argEquals(argc, argv, "-th") != -1)   ? 1
                                 : (argEquals(argc, argv, "-tv") != -1) ? 0
                                                                        : 1;
}

void nameTableProcessorGenerate(NameTableProcessor *this, NameTable *nametable, Charset *charset) {

    // Initial namtbl
    nametable->namtblSize = charset->blockCount;
    nametable->namtbl = (int *)calloc(nametable->namtblSize, sizeof(int));

    // For each block...
    int i, b, c, blockCount, *it;
    Block *src, *dest;

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
            if (src != dest) {
                memcpy(dest, src, sizeof(Block));
            }
            *it = b++;
            blockCount++;
            dest++;
        }

    } else {

        unsigned int x, y;

        for (x = 0, b = 0, blockCount = 0, src = dest = charset->blocks, it = nametable->namtbl; x < charset->width;
             x++) {
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
                if (src != dest) {
                    memcpy(dest, src, sizeof(Block));
                }
                *it = b++;
                blockCount++;
                dest++;
            }
        }

        // Transposes the namtbl
        int *transposed = (int *)calloc(nametable->namtblSize, sizeof(int));

        for (i = 0; i < nametable->namtblSize; i++) {
            int toX = i / charset->height, toY = i % charset->height, to = toY * charset->width + toX;
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
            memcpy(&charset->blocks[i], &charset->blocks[j], sizeof(Block));
        }
        // Blank
        Line *itLine;
        for (i = 0, itLine = charset->blocks[this->blankAt].line; i < TILE_HEIGHT; i++, itLine++) {
            itLine->pattern = 0x00;
            itLine->color = 0x00;
        }
    }
}

void nameTableProcessorGenerateUsing(__attribute__((unused)) NameTableProcessor *this, NameTable *nametable,
                                     Charset *charset, Charset *screen) {

    // Initial namtbl
    nametable->namtblSize = screen->blockCount;
    nametable->namtbl = (int *)calloc(nametable->namtblSize, sizeof(int));

    // For each block...
    int i, j, *it;
    Block *block;
    for (i = 0, it = nametable->namtbl, block = screen->blocks; i < nametable->namtblSize; i++, it++, block++) {
        j = blockIndex(charset, block, charset->blockCount);
        *it = j;
    }
}

void nameTableProcessorApplyTo(NameTable *nametable, Charset *charset) {

    int i, expected, blockCount, *it;
    Block *src, *dest;

    for (i = expected = 0, blockCount = 0, src = dest = charset->blocks, it = nametable->namtbl;
         (i < charset->blockCount) && (i < nametable->namtblSize); i++, src++, it++) {
        // Empty or repeated in master pcx
        if ((*it == -1) || (*it < expected)) {
            continue;
        }

        // Not empty nor repeated in master pcx: move (compact blocks)
        if (src != dest) {
            memcpy(dest, src, sizeof(Block));
        }
        blockCount++;
        dest++;
    }
    reallocateBlocks(charset, blockCount);
}

void nameTableProcessorPostProcess(NameTableProcessor *this, NameTable *nametable) {

    int i, *it;
    for (i = 0, it = nametable->namtbl; i < nametable->namtblSize; i++, it++) {
        *it = (*it == -1) ? ((this->generateBlank) ? this->blankAt : -1)
                          : *it + this->namtblOffset + ((this->generateBlank) && (*it >= this->blankAt) ? 1 : 0);
    }
}

int nameTableProcessorWrite(NameTableProcessor *this, NameTable *nametable, FILE *namFile) {

    int i, *it;
    uint8_t b;
    for (i = 0, it = nametable->namtbl; i < nametable->namtblSize; i++, it++) {
        b = (uint8_t)(((*it == -1) ? (this->namtblOffset + 0xff) : (*it)) & 0xff);
        if (fwrite(&b, sizeof(uint8_t), 1, namFile) != 1) {
            return 1;
        }
    }
    return 0;
}

void nameTableProcessorDone(__attribute__((unused)) NameTableProcessor *this) {

    // nothing to do here
}

void nameTableDone(NameTable *this) {

    if (this->namtbl) {
        free(this->namtbl);
    }
    this->namtbl = NULL;
}

/* Private function bodies ------------------------------------------------- */

void reallocateBlocks(Charset *charset, int newBlockCount) {

    if (newBlockCount == charset->blockCount) {
        return;
    }

    charset->blockCount = newBlockCount;
    if (newBlockCount) {
        // Reallocate space for the blocks
        charset->blocks = (Block *)realloc(charset->blocks, newBlockCount * sizeof(Block));
    } else {
        // Remove blocks
        free(charset->blocks);
        charset->blocks = NULL;
    }
}
