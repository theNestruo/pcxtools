/*
 * PCX2SPR is a free tool to convert PCX images to MSX-1 VDP sprites
 * (i.e. SPRTBL-ready values).
 * Coded by theNestruo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "sprite.h"
#include "readpcx.h"

/* Global vars ------------------------------------------------------------- */

int titleShown = 0;

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
	char *sprFilename = NULL;
	FILE *pcxFile = NULL;
	FILE *sprFile = NULL;
	struct stBitmap bitmap = {0};
	struct stSprWriter writer = {0};

	int i = 0, argi = 0;
	int verbose = 0, dryRun = 0;

	// Parse main arguments
	if ((verbose = (argEquals(argc, argv, "-v") != -1)))
		showTitle();
	dryRun = argEquals(argc, argv, "-d") != -1;

	if ((argi = argFilename(argc, argv)) != -1) {
		pcxFilename = argv[argi];
		sprFilename = append(argv[argi], ".spr");
	}

	if (!pcxFilename) {
		showUsage();
		i = 12;
		goto out;
	}

	if (verbose) {
		printf("Input file: %s\nOutput file: %s\n",
			pcxFilename, sprFilename);
	}

	// Initializes bitmap container and chr/clr processor
	bitmapInit(&bitmap, argc, argv);
	sprWriterInit(&writer, argc, argv);

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
		printf("Processing sprites...\n");
	sprWriterReadSprites(&writer, &bitmap);

	if (!dryRun) {
		if (verbose)
			printf("Writing output file...\n");
		if (!(sprFile = fopen(sprFilename, "wb"))) {
			printf("ERROR: Could not create %s.\n", sprFilename);
			i = 14;
			goto out;
		}
		if ((i = sprWriterWrite(&writer, sprFile))) {
			printf("ERROR: Failed writing %s.\n", sprFilename);
			goto out;
		}
	}

	if (verbose)
		printf("Done!\n");

out:
	// Exit gracefully
	if (sprFilename) free(sprFilename);
	if (pcxFile) fclose(pcxFile);
	if (sprFile) fclose(sprFile);
	bitmapDone(&bitmap);
	sprWriterDone(&writer);
	return i;
}

/* Function bodies ------------------------------------- */

void showTitle() {

	if (titleShown)
		return;
	printf("PCX2SPR: A tool to convert PCX images to TMS9918 sprites\n");
	printf("Deprecation notice: please consider using PNG2SPR instead\n");
	titleShown = 1;
}

void showUsage() {

	showTitle();
	printf("Usage:\n");
	printf("\tPCX2SPR [options] pcxFilename\n");
	printf("where:\n");
	printf("\tpcxFilename input PCX file\n");
	printf("options are:\n");
	printf("\t-v\tverbose execution\n");
	printf("\t-d\tdry run. Doesn't write output files\n");
	bitmapOptions();
	sprWriterOptions();
}
