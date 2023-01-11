/*
 * Support routines reading Tiled (TMX) files
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbj�rn Lindeijer [http://www.mapeditor.org/]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "readtmx.h"

/* Constant values --------------------------------------------------------- */

/* Private function prototypes --------------------------------------------- */

char* readProperty(char *tag, char *propertyName);

/* Function bodies --------------------------------------------------------- */

void tmxReaderOptions() {

	printf("\t-b\tmultibank charset support\n");
}

void tmxReaderInit(struct stTmxReader *this, int argc, char **argv) {

	// Init
	this->isMultibankCharset = 0;

	// Read arguments
	this->isMultibankCharset = (argEquals(argc, argv, "-b") != -1);
}

int tmxReaderRead(struct stTmxReader *this, FILE *file, struct stTiled *tiled) {

	if (!file)
		return 1;

	if (!tiled)
		return 2;

	int i = 0;

	// Allocate buffer
	int bufferSize = 1024; // should be enough (for control lines)
	char *buffer = malloc(bufferSize), *line;

	// Reads the header: XML
	if (!(line = fgets(buffer, bufferSize, file))) {
		printf("ERROR: Could not read XML header.\n");
		i = 3;
		goto out;
	}
	if (line != strstr(line, "<?xml")) {
		printf("ERROR: TMX file is not XML.\n");
		i = 4;
		goto out;
	}

	// Reads the header: TMX
	if (!(line = fgets(buffer, bufferSize, file))) {
		printf("ERROR: Could not read TMX header.\n");
		i = 5;
		goto out;
	}
	if (line != strstr(line, "<map")) {
		printf("ERROR: Expected <map> tag not found.\n");
		i = 6;
		goto out;
	}

	// Searches for the tag "layer"
	for(;;) {
		if (!(line = fgets(buffer, bufferSize, file))) {
			printf("ERROR: Missing <layer> tag.\n");
			i = 7;
			goto out;
		}
		if ((line = strstr(line, "<layer"))) {
			break;
		}
	}

	// Read layer properties
	char *height, *width, *name, *encoding;
	if ((!(height = readProperty(line, "height")))
			|| (!(width = readProperty(line, "width")))
			|| (!(name = readProperty(line, "name")))) {
		printf("ERROR: Invalid <layer>: missing properties.\n");
		i = 8;
		goto out;
	}
	if ((!(tiled->width = atoi(width)))
			|| (!(tiled->height = atoi(height)))) {
		printf("ERROR: Invalid <layer>: invalid width and/or height properties.\n");
		i = 9;
		goto out;
	}

	// Searches for the tag "data"
	if (!(line = fgets(buffer, bufferSize, file))) {
		printf("ERROR: Unexpected EOF inside <layer>.\n");
		i = 10;
		goto out;
	}
	if (!(line = strstr(line, "<data"))) {
		printf("ERROR: Missing <data> tag.\n");
		i = 11;
		goto out;
	}
	if (!(encoding = readProperty(line, "encoding"))) {
		printf("ERROR: Invalid <data>: missing encoding property.\n");
		i = 12;
		goto out;
	}
	if (strcmp(encoding, "csv")) {
		printf("ERROR: Invalid <data>: unsupported encoding \"%s\".\n", encoding);
		i = 13;
		goto out;
	}

	// Allocate space for the data
	bufferSize = tiled->width * 4 + 16;
	buffer = realloc(buffer, bufferSize); // "nnn," per byte and some margin
	tiled->data = (byte*) malloc(tiled->width * tiled->height);

	int y;
	byte *dest = tiled->data;
	for (y = 0; y < tiled->height; y++) {
		if (!(line = fgets(buffer, bufferSize, file))) {
			printf("ERROR: Unexpected EOF inside <data>.\n");
			i = 14;
			goto out;
		}

		int x, val;
		char *token;
		for (x = 0, token = strtok(line, ",");
				x < tiled->width;
				x++, token = strtok(NULL, ","), dest++) {
			if (!token) {
				printf("ERROR: Missing/invalid value at %d,%d.\n", x, y);
				i = 15;
				goto out;
			}
			val = atoi(token);
			if (val < 256) {
				*dest = (byte) (val - 1);

			} else if (this->isMultibankCharset) {
				*dest = (byte) ((val % 256) - 1);

			} else {
				printf("WARNING: Byte overflow at %d,%d: %d...\n", x, y, val);
			}
		}
	}

out:
	free(buffer);
	return i;
}

/* Private function bodies ------------------------------------------------- */

/**
 * Extracts the value of a property of a given tag.
 * @param tag where the property will be searched
 * Warning! The value of the tag will be modified
 * @param propertyName the name of the property
 * @ret pointer to the value of the tag
 */
char* readProperty(char *tag, char *propertyName) {

	char *property, *from, *to;
	if (!(property = strstr(tag, propertyName))) {
		return NULL;
	}
	if (!(from = strstr(property, "\""))) {
		return NULL;
	}
	from++;
	if (!(to = strstr(from, "\""))) {
		return NULL;
	}
	to[0] = '\0';

	return from;
}
