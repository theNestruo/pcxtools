/*
 * Support routines for output files
 * Coded by theNestruo
 */

#pragma once

#include <stdint.h>

/* Function prototypes ----------------------------------------------------- */

int asmNewLine(FILE *file);
int asmComment(FILE *file, char *string, int inner);
int asmLabel(FILE *file, char *string);
int asmBytes(FILE *file, uint8_t *bytes, int byteCount);

