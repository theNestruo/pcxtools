/*
 * PNG2SPR+ is a free tool to extract MSX-1 VDP sprites from PNG images
 * (i.e. SPRTBL-ready values) with extra options
 * Coded by theNestruo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bitmap.h"
#include "sprite+.h"
#include "readpng.h"

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

	char *pngFilename = NULL;
	char *sprFilename = NULL;
	char *spatFilename = NULL;
	FILE *sprFile = NULL;
	FILE *spatFile = NULL;
	struct stBitmap bitmap = {0};
	struct stSprWriterPlus writer = {0};

	int i = 0, argi = 0;
	int dryRun = 0;

	// Parse main arguments
	veryVerbose = argEquals(argc, argv, "-vv") != -1;
	verbose  = (argEquals(argc, argv, "-v") != -1) | veryVerbose;
	if (verbose)
		showTitle();
	dryRun = argEquals(argc, argv, "-d") != -1;

	if ((argi = argFilename(argc, argv)) != -1)
		pngFilename = argv[argi];

	if (!pngFilename) {
		showUsage();
		i = 12;
		goto out;
	}

	if (verbose)
		printf("Input file: %s\n", pngFilename);

	// Initializes bitmap container and chr/clr processor
	pngReaderInit(argc, argv);
	bitmapInit(&bitmap, argc, argv);
	sprWriterPlusInit(&writer, argc, argv);

	sprFilename = append(pngFilename, writer.binaryOutput ? ".spr" : ".spr.asm");
	spatFilename = append(pngFilename, writer.binaryOutput ? ".spat" : ".spat.asm");

	if (verbose)
		printf("Output files: %s, %s\n", sprFilename, spatFilename);

	// Main code
	if (verbose)
		printf("Reading input file...\n");
	if ((i = pngReaderRead(pngFilename, &bitmap)))
		goto out;

	if (verbose)
		printf("Processing sprites...\n");
	sprWriterPlusReadSprites(&writer, &bitmap);

	if (!dryRun) {
		if (verbose)
			printf("Writing output files...\n");
		if (!(sprFile = fopen(sprFilename, writer.binaryOutput ? "wb" : "wt"))) {
			printf("ERROR: Could not create %s.\n", sprFilename);
			i = 14;
			goto out;
		}
		if (!(spatFile = fopen(spatFilename, writer.binaryOutput ? "wb" : "wt"))) {
			printf("ERROR: Could not create %s.\n", spatFilename);
			i = 15;
			goto out;
		}
		if ((i = sprWriterPlusWrite(&writer, sprFile, spatFile))) {
			printf("ERROR: Failed writing %s/%s (%d).\n", sprFilename, spatFilename, i);
			goto out;
		}
	}

	if (verbose)
		printf("Done!\n");

out:
	// Exit gracefully
	if (sprFilename) free(sprFilename);
	if (sprFile) fclose(sprFile);
	if (spatFile) fclose(spatFile);
	bitmapDone(&bitmap);
	sprWriterPlusDone(&writer);
	return i;
}

/* Function bodies ------------------------------------- */

void showTitle() {

	if (titleShown)
		return;
	printf("PNG2SPR+: A tool to extract MSX-1 VDP sprites from PNG images\n");
	titleShown = 1;
}

void showUsage() {

	showTitle();
	printf("Usage:\n");
	printf("\tPNG2SPR+ [options] pngFilename\n");
	printf("where:\n");
	printf("\tpngFilename input PNG file\n");
	printf("options are:\n");
	printf("\t-v\tverbose execution\n");
	printf("\t-d\tdry run. Doesn't write output files\n");
	pngReaderOptions();
	bitmapOptions();
	sprWriterPlusOptions();
}
