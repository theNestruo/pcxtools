/*
 * PCX2MSX is a free tool to convert PCX images to TMS9918 format (MSX-1 VDP)
 * Coded by theNestruo.
 * Original tool coded by Edward A. Robsy Petrus [25/12/2004]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "charset.h"
#include "readpcx.h"

/* Global vars ------------------------------------------------------------- */

int titleShown = 0;
int verbose = 0;
int veryVerbose = 0;

/* Function prototypes ----------------------------------------------------- */

void showTitle();
void showUsage();

/* Entry point ------------------------------------------------------------- */

int main(int argc, char **argv) {

	// Shows usage if there are no parameters
	if (argc == 1) {
		showUsage();
		return 11;
	}

	char *pcxFilename = NULL;
	char *chrFilename = NULL;
	char *clrFilename = NULL;
	FILE *pcxFile = NULL;
	FILE *chrFile = NULL;
	FILE *clrFile = NULL;
	struct stBitmap bitmap = {0};
	struct stCharset charset = {0};
	struct stCharsetProcessor processor = {0};

	int i = 0, argi = 0;
	int dryRun = 0;

	// Parse main arguments
	veryVerbose = argEquals(argc, argv, "-vv") != -1;
	verbose = (argEquals(argc, argv, "-v") != -1) | veryVerbose;
	if (verbose)
		showTitle();
	dryRun = argEquals(argc, argv, "-d") != -1;

	if ((argi = argFilename(argc, argv)) != -1) {
		pcxFilename = argv[argi];
		chrFilename = append(argv[argi], ".chr");
		clrFilename = append(argv[argi], ".clr");
	}

	if (!pcxFilename) {
		showUsage();
		i = 12;
		goto out;
	}

	if (verbose) {
		printf("Input file: %s\nOutput files: %s, %s\n",
			pcxFilename, chrFilename, clrFilename);
	}

	// Initializes bitmap container and chr/clr processor
	bitmapInit(&bitmap, argc, argv);
	charsetProcessorInit(&processor, argc, argv);

	// Main code
	if (verbose)
		printf("Reading input file...\n");
	if (!(pcxFile = fopen(pcxFilename, "rb"))) {
		printf("ERROR: Could not open %s.\n", pcxFilename);
		i = 13;
		goto out;
	}
	if ((i = pcxReaderRead(pcxFile, &bitmap)))
		goto out;

	if (verbose)
		printf("Processing blocks...\n");
	if ((i = charsetProcessorRead(&processor, &charset, &bitmap)))
		goto out;
	charsetProcessorPostProcess(&processor, &charset);

	if (!dryRun) {
		if (verbose)
			printf("Writing output files...\n");
		if (!(chrFile = fopen(chrFilename, "wb"))) {
			printf("ERROR: Could not create %s.\n", chrFilename);
			i = 14;
			goto out;
		}
		if (!(clrFile = fopen(clrFilename, "wb"))) {
			printf("ERROR: Could not create %s.\n", clrFilename);
			i = 15;
			goto out;
		}
		if ((i = charsetProcessorWrite(&processor, &charset, chrFile, clrFile))) {
			printf("ERROR: Failed writing %s and/or %s.\n",
				chrFilename, clrFilename);
			i = 17;
			goto out;
		}
	}

	if (verbose)
		printf("Done!\n");

out:
	// Exit gracefully
	if (chrFilename) free(chrFilename);
	if (clrFilename) free(clrFilename);
	if (pcxFile) fclose(pcxFile);
	if (chrFile) fclose(chrFile);
	if (clrFile) fclose(clrFile);
	bitmapDone(&bitmap);
	charsetDone(&charset);
	charsetProcessorDone(&processor);
	return i;
}

/* Function bodies --------------------------------------------------------- */

void showTitle() {

	if (titleShown)
		return;
	printf("PCX2MSX: A tool to convert PCX images to TMS9918 format\n");
	printf("Deprecation notice: please consider using PNG2MSX instead\n");
	titleShown = 1;
}

void showUsage() {

	showTitle();
	printf("Usage:\n");
	printf("\tPCX2MSX [options] pcxFilename\n");
	printf("where:\n");
	printf("\tpcxFilename\tinput PCX file\n");
	printf("options are:\n");
	printf("\t-v\tverbose execution\n");
	printf("\t-vv\tvery verbose execution\n");
	printf("\t-d\tdry run. Doesn't write output files\n");
	bitmapOptions();
	charsetProcessorOptions();
}

