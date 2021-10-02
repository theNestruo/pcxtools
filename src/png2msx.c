/*
 * PNG2MSX is a free tool to convert PNG images to TMS9918 format (MSX-1 VDP)
 * with many options to manage interdependant charsets or NAMTBL generation.
 * Coded by theNestruo.
 * Original tool coded by Edward A. Robsy Petrus [25/12/2004]
 *
 * Version history:
 * 02/10/2021  v3.0         Fixed -e and -g options being ignored
 * 07/12/2020  v3.0-alpha   forked from PCX2MSX+
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "charset.h"
#include "nametable.h"
#include "readpng.h"

/* Symbolic constants ------------------------------------------------------ */

#define MODE_SINGLE_PNG (0)
#define MODE_MULTIPLE_PNG (1)
#define MODE_SCREEN_MAPPING (2)

/* Global vars ------------------------------------------------------------- */

int titleShown = 0;

/* Function prototypes ----------------------------------------------------- */

void showTitle();
void showUsage();
int readCharset(struct stCharsetProcessor *charsetProcessor,
	struct stCharset *charset, struct stBitmap *bitmap, char *pngFilename, int verbose);
int writeCharset(struct stCharsetProcessor *charsetProcessor,
	struct stCharset *charset, char *pngFilename, int verbose);
int writeNameTable(struct stNameTableProcessor *nameTableProcessor,
	struct stNameTable *nameTable, char *pngFilename, int verbose);

/* Entry point ------------------------------------------------------------- */

int main(int argc, char **argv) {

	// Show usage if there are no parameters
	if (argc == 1) {
		showUsage();
		return 11;
	}

	int i = 0, argi = 0;

	// Parse main arguments
	int verbose = 0, dryRun = 0, generateNameTable = 0, mode = 0;
	char *pngFilename = NULL;
	if ((verbose = (argEquals(argc, argv, "-v") != -1)))
		showTitle();
	dryRun = argEquals(argc, argv, "-d") != -1;
	generateNameTable = argStartsWith(argc, argv, "-n", 2) != -1;
	if ((argi = argFilename(argc, argv)) != -1)
		pngFilename = argv[argi];
	mode = (argNextFilename(argc, argv, argi) == -1) ? MODE_SINGLE_PNG
		: generateNameTable ? MODE_SCREEN_MAPPING
		: MODE_MULTIPLE_PNG;
	if (!pngFilename) {
		showUsage();
		return 12;
	}

	struct stCharsetProcessor charsetProcessor = {0};
	struct stNameTableProcessor nameTableProcessor = {0};
	struct stBitmap bitmap = {0};
	struct stCharset charset = {0};
	struct stCharset screenCharset = {0};
	struct stNameTable nameTable = {0};

	// Read main file
	pngReaderInit(argc, argv);
	bitmapInit(&bitmap, argc, argv);
	charsetProcessorInit(&charsetProcessor, argc, argv);
	if ((i = readCharset(&charsetProcessor, &charset, &bitmap, pngFilename, verbose)))
		goto out;

	// Do the work
	switch (mode) {
	case MODE_SINGLE_PNG:
		if (verbose) printf("Working mode is: single PNG file.\n");

		nameTableProcessorInit(&nameTableProcessor, argc, argv);
		nameTableProcessorGenerate(&nameTableProcessor, &nameTable, &charset);
		charsetProcessorPostProcess(&charsetProcessor, &charset);

		if (dryRun)
			break;

		if ((i = writeCharset(&charsetProcessor, &charset, pngFilename, verbose)))
			goto out;
		if (generateNameTable) {
			nameTableProcessorPostProcess(&nameTableProcessor, &nameTable);
			if ((i = writeNameTable(&nameTableProcessor, &nameTable, pngFilename, verbose)))
				goto out;
		}
		break;

	case MODE_SCREEN_MAPPING:
		if (verbose) printf("Working mode is: screen(s) mapping.\n");

		// (main nametable generated just to apply -rm, -rr)
		nameTableProcessorInit(&nameTableProcessor, argc, argv);
		nameTableProcessorGenerate(&nameTableProcessor, &nameTable, &charset);

		// Next files
		while ((argi = argNextFilename(argc, argv, argi)) != -1) {
			// Read
			pngFilename = argv[argi];
			bitmapDone(&bitmap); // (free previous file resources)
			charsetDone(&screenCharset); // (free previous file resources)
			if ((i = readCharset(&charsetProcessor, &screenCharset, &bitmap, pngFilename, verbose)))
				goto out;

			// Generate
			nameTableDone(&nameTable); // (free previous file resources)
			nameTableProcessorGenerateUsing(&nameTableProcessor, &nameTable, &charset, &screenCharset);

			// Write
			nameTableProcessorPostProcess(&nameTableProcessor, &nameTable);
			if (!dryRun)
				if ((i = writeNameTable(&nameTableProcessor, &nameTable, pngFilename, verbose)))
					goto out;
		}
		break;

	case MODE_MULTIPLE_PNG:
		if (verbose) printf("Working mode is: multiple PNG files.\n");

		nameTableProcessorInit(&nameTableProcessor, argc, argv);
		nameTableProcessorGenerate(&nameTableProcessor, &nameTable, &charset);

		// First file
		charsetProcessorPostProcess(&charsetProcessor, &charset);
		if (!dryRun)
			if ((i = writeCharset(&charsetProcessor, &charset, pngFilename, verbose)))
				goto out;

		// Next files
		while ((argi = argNextFilename(argc, argv, argi)) != -1) {
			// Read
			pngFilename = argv[argi];
			bitmapDone(&bitmap); // (free previous file resources)
			charsetDone(&charset); // (free previous file resources)
			if ((i = readCharset(&charsetProcessor, &charset, &bitmap, pngFilename, verbose)))
				goto out;

			// Apply first file nametable
			nameTableProcessorApplyTo(&nameTable, &charset);

			// Write
			charsetProcessorPostProcess(&charsetProcessor, &charset);
			if (!dryRun)
				if ((i = writeCharset(&charsetProcessor, &charset, pngFilename, verbose)))
					goto out;
		}

		break;

	default:
		printf("ERROR: Invalid working mode.\n");
		i = 14;
		goto out;
	}

	if (verbose)
		printf("Done!\n");

out:
	// Exit gracefully
	charsetProcessorDone(&charsetProcessor);
	nameTableProcessorDone(&nameTableProcessor);
	bitmapDone(&bitmap);
	charsetDone(&charset);
	nameTableDone(&nameTable);
	return i;
}

/* Function bodies --------------------------------------------------------- */

void showTitle() {

	if (titleShown)
		return;
	printf("PNG2MSX: A tool to convert PNG images to TMS9918 format\n");
	titleShown = 1;
}

void showUsage() {

	showTitle();
	printf("Usage:\n");
	printf("\tPNG2MSX [options] charset.png\n");
	printf("\tPNG2MSX [options] charset.png [extra.png ...]\n");
	printf("\tPNG2MSX [options] charset.png -n [screen.png ...]\n");
	printf("where:\n");
	printf("\tcharset.png\tinput PNG file\n");
	printf("\textra.png\tadditional input PNG files: extra charsets\n");
	printf("\tscreen.png\tadditional input PNG files: screens to map\n");
	printf("options are:\n");
	printf("\t-v\tverbose execution\n");
	printf("\t-d\tdry run. Doesn't write output files\n");
	pngReaderOptions();
	bitmapOptions();
	charsetProcessorOptions();
	nameTableProcessorOptions();
}

int readCharset(struct stCharsetProcessor *charsetProcessor, struct stCharset *charset,
	struct stBitmap *bitmap, char *pngFilename, int verbose) {

	int i = 0;

	if (verbose) printf("Reading input file %s...\n", pngFilename);
	if ((i = pngReaderRead(pngFilename, bitmap)))
		goto out;

	if (verbose) printf("Processing blocks...\n");
	if ((i = charsetProcessorRead(charsetProcessor, charset, bitmap)))
		goto out;

out:
	// Exit gracefully
	return i;
}

int writeCharset(struct stCharsetProcessor *charsetProcessor, struct stCharset *charset, char *pngFilename, int verbose) {

	int i = 0;

	char *chrFilename = NULL;
	char *clrFilename = NULL;
	FILE *chrFile = NULL;
	FILE *clrFile = NULL;

	chrFilename = append(pngFilename, ".chr");
	clrFilename = append(pngFilename, ".clr");
	if (verbose)
		printf("Writing output files %s, %s...\n", chrFilename, clrFilename);

	if (!(chrFile = fopen(chrFilename, "wb"))) {
		printf("ERROR: Could not create %s.\n", chrFilename);
		i = 31;
		goto out;
	}

	if (!(clrFile = fopen(clrFilename, "wb"))) {
		printf("ERROR: Could not create %s.\n", clrFilename);
		i = 32;
		goto out;
	}

	if ((i = charsetProcessorWrite(charsetProcessor, charset, chrFile, clrFile))) {
		printf("ERROR: Failed writing %s and/or %s.\n", chrFilename, clrFilename);
		goto out;
	}

out:
	// Exit gracefully
	if (chrFilename) free(chrFilename);
	if (clrFilename) free(clrFilename);
	if (chrFile) fclose(chrFile);
	if (clrFile) fclose(clrFile);
	return i;
}

int writeNameTable(struct stNameTableProcessor *nameTableProcessor, struct stNameTable *nameTable, char *pngFilename, int verbose) {

	int i = 0;

	char *namFilename = NULL;
	FILE *namFile = NULL;

	namFilename = append(pngFilename, ".nam");
	if (verbose)
		printf("Writing output file %s...\n", namFilename);
	if (!(namFile = fopen(namFilename, "wb"))) {
		printf("ERROR: Could not create %s.\n", namFilename);
		i = 41;
		goto out;
	}
	if ((i = nameTableProcessorWrite(nameTableProcessor, nameTable, namFile))) {
		printf("ERROR: Failed writing %s.\n", namFilename);
		goto out;
	}

out:
	// Exit gracefully
	if (namFilename) free(namFilename);
	if (namFile) fclose(namFile);
	return i;
}
