/*
 * PCX2MSX+ is a free tool to convert PCX images to TMS9918 format (MSX-1 VDP)
 * with many options to manage interdependant charsets or NAMTBL generation.
 * Coded by theNestruo.
 * Original tool coded by Edward A. Robsy Petrus [25/12/2004]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "charset.h"
#include "nametable.h"
#include "readpcx.h"

/* Symbolic constants ------------------------------------------------------ */

#define MODE_SINGLE_PCX (0)
#define MODE_MULTIPLE_PCX (1)
#define MODE_SCREEN_MAPPING (2)

/* Global vars ------------------------------------------------------------- */

int titleShown = 0;

/* Function prototypes ----------------------------------------------------- */

void showTitle();
void showUsage();
int readCharset(struct stCharsetProcessor *charsetProcessor,
	struct stCharset *charset, struct stBitmap *bitmap, char *pcxFilename, int verbose);
int writeCharset(struct stCharsetProcessor *charsetProcessor,
	struct stCharset *charset, char *pcxFilename, int verbose);
int writeNameTable(struct stNameTableProcessor *nameTableProcessor,
	struct stNameTable *nameTable, char *pcxFilename, int verbose);

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
	char *pcxFilename = NULL;
	if ((verbose = (argEquals(argc, argv, "-v") != -1)))
		showTitle();
	dryRun = argEquals(argc, argv, "-d") != -1;
	generateNameTable = argStartsWith(argc, argv, "-n", 2) != -1;
	if ((argi = argFilename(argc, argv)) != -1)
		pcxFilename = argv[argi];
	mode = (argNextFilename(argc, argv, argi) == -1) ? MODE_SINGLE_PCX
		: generateNameTable ? MODE_SCREEN_MAPPING
		: MODE_MULTIPLE_PCX;
	if (!pcxFilename) {
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
	bitmapInit(&bitmap, argc, argv);
	charsetProcessorInit(&charsetProcessor, argc, argv);
	if ((i = readCharset(&charsetProcessor, &charset, &bitmap, pcxFilename, verbose)))
		goto out;

	// Do the work
	switch (mode) {
	case MODE_SINGLE_PCX:
		if (verbose) printf("Working mode is: single PCX file.\n");

		nameTableProcessorInit(&nameTableProcessor, argc, argv);
		nameTableProcessorGenerate(&nameTableProcessor, &nameTable, &charset);
		charsetProcessorPostProcess(&charsetProcessor, &charset);

		if (dryRun)
			break;

		if ((i = writeCharset(&charsetProcessor, &charset, pcxFilename, verbose)))
			goto out;
		if (generateNameTable) {
			nameTableProcessorPostProcess(&nameTableProcessor, &nameTable);
			if ((i = writeNameTable(&nameTableProcessor, &nameTable, pcxFilename, verbose)))
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
			pcxFilename = argv[argi];
			bitmapDone(&bitmap); // (free previous file resources)
			charsetDone(&screenCharset); // (free previous file resources)
			if ((i = readCharset(&charsetProcessor, &screenCharset, &bitmap, pcxFilename, verbose)))
				goto out;

			// Generate
			nameTableDone(&nameTable); // (free previous file resources)
			nameTableProcessorGenerateUsing(&nameTableProcessor, &nameTable, &charset, &screenCharset);

			// Write
			nameTableProcessorPostProcess(&nameTableProcessor, &nameTable);
			if (!dryRun)
				if ((i = writeNameTable(&nameTableProcessor, &nameTable, pcxFilename, verbose)))
					goto out;
		}
		break;

	case MODE_MULTIPLE_PCX:
		if (verbose) printf("Working mode is: multiple PCX files.\n");

		nameTableProcessorInit(&nameTableProcessor, argc, argv);
		nameTableProcessorGenerate(&nameTableProcessor, &nameTable, &charset);

		// First file
		charsetProcessorPostProcess(&charsetProcessor, &charset);
		if (!dryRun)
			if ((i = writeCharset(&charsetProcessor, &charset, pcxFilename, verbose)))
				goto out;

		// Next files
		while ((argi = argNextFilename(argc, argv, argi)) != -1) {
			// Read
			pcxFilename = argv[argi];
			bitmapDone(&bitmap); // (free previous file resources)
			charsetDone(&charset); // (free previous file resources)
			if ((i = readCharset(&charsetProcessor, &charset, &bitmap, pcxFilename, verbose)))
				goto out;

			// Apply first file nametable
			nameTableProcessorApplyTo(&nameTable, &charset);

			// Write
			charsetProcessorPostProcess(&charsetProcessor, &charset);
			if (!dryRun)
				if ((i = writeCharset(&charsetProcessor, &charset, pcxFilename, verbose)))
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
	printf("PCX2MSX+: A tool to convert PCX images to TMS9918 format\n");
	printf("Deprecation notice: please consider using PNG2MSX instead\n");
	titleShown = 1;
}

void showUsage() {

	showTitle();
	printf("Usage:\n");
	printf("\tPCX2MSX [options] mainPcx\n");
	printf("\tPCX2MSX [options] mainPcx [extraPcx...]\n");
	printf("\tPCX2MSX [options] mainPcx -n [screensPcx...]\n");
	printf("where:\n");
	printf("\tmainPcx\tinput PCX file\n");
	printf("\textraPcx\textra input PCX files: secondary charsets\n");
	printf("\tscreensPcx\textra input PCX files: screens to map\n");
	printf("options are:\n");
	printf("\t-v\tverbose execution\n");
	printf("\t-d\tdry run. Doesn't write output files\n");
	bitmapOptions();
	charsetProcessorOptions();
	nameTableProcessorOptions();
}

int readCharset(struct stCharsetProcessor *charsetProcessor, struct stCharset *charset,
	struct stBitmap *bitmap, char *pcxFilename, int verbose) {

	int i = 0;

	FILE *pcxFile = NULL;

	if (verbose) printf("Reading input file %s...\n", pcxFilename);
	if (!(pcxFile = fopen(pcxFilename, "rb"))) {
		printf("ERROR: Could not open %s.\n", pcxFilename);
		i = 21;
		goto out;
	}

	if ((i = pcxReaderRead(pcxFile, bitmap)))
		goto out;

	if (verbose) printf("Processing blocks...\n");
	if ((i = charsetProcessorRead(charsetProcessor, charset, bitmap)))
		goto out;

out:
	// Exit gracefully
	if (pcxFile) fclose(pcxFile);
	return i;
}

int writeCharset(struct stCharsetProcessor *charsetProcessor, struct stCharset *charset, char *pcxFilename, int verbose) {

	int i = 0;

	char *chrFilename = NULL;
	char *clrFilename = NULL;
	FILE *chrFile = NULL;
	FILE *clrFile = NULL;

	chrFilename = append(pcxFilename, ".chr");
	clrFilename = append(pcxFilename, ".clr");
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

int writeNameTable(struct stNameTableProcessor *nameTableProcessor, struct stNameTable *nameTable, char *pcxFilename, int verbose) {

	int i = 0;

	char *namFilename = NULL;
	FILE *namFile = NULL;

	namFilename = append(pcxFilename, ".nam");
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
