/*
 * Support routines for output files
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "args.h"
#include "output.h"

/* Function bodies --------------------------------------------------------- */

int asmNewLine(FILE *file) {

	return fwrite("\n", sizeof(char), 1, file) == 1;
}

int asmComment(FILE *file, char *string, int inner) {

	int i = 0;
	
	char *buffer = append(inner ? "\t; " : "; ", string);
	if (fwrite(buffer, sizeof(char), strlen(buffer), file) != strlen(buffer)) goto out;
	i = asmNewLine(file);
out:
	free(buffer);
	return i;
}

int asmBytes(FILE *file, byte *bytes, int byteCount) {

	int i = 0;
	
	char *init = "\t.db\t0x%02x", *cont = ", 0x%02x", *buffer = (char*) calloc(16, sizeof(char));
	int j;
	byte *b;
	for (j = 0, b = bytes; j < byteCount; j++, b++) {
		sprintf(buffer, j ? cont : init, *b);
		if (fwrite(buffer, sizeof(char), strlen(buffer), file) != strlen(buffer)) goto out;
	}
	i = asmNewLine(file);
out:
	free(buffer);
	return i;
}
