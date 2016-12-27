/*
 * Support routines for output files
 * Coded by theNestruo
 */

#ifndef OUTPUT_H_INCLUDED
#define OUTPUT_H_INCLUDED

/* Types ------------------------------------------------------------------- */

#ifndef byte
typedef unsigned char byte;
#endif

/* Function prototypes ----------------------------------------------------- */

int asmNewLine(FILE *file);
int asmComment(FILE *file, char *string, int inner);
int asmLabel(FILE *file, char *string);
int asmBytes(FILE *file, byte *bytes, int byteCount);

#endif // OUTPUT_H_INCLUDED
