/*
 * TMX2BIN is a free tool to convert Tiled maps to binary
 * Coded by theNestruo
 * Tiled (c) 2008-2013 Thorbjørn Lindeijer [http://www.mapeditor.org/]
 *
 * Version history:
 * 26/12/2016  v0.2  Integrated into PCXTOOLS suite
 * 31/03/2013  v0.1  Initial version
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "readtmx.h"

/* Global vars ------------------------------------------------------------- */

int titleShown = 0;
int verbose = 0;

/* Function prototypes ----------------------------------------------------- */

void showTitle();
void showUsage();

/* Entry point ------------------------------------------------------------- */

int main(int argc, char **argv) {

	// Show usage if there are no parameters
	if (argc == 1) {
		showUsage();
		return 11;
	}

	char *tmxFilename = NULL;
	char *binFilename = NULL;
	FILE *tmxFile = NULL;
	FILE *binFile = NULL;
	struct stTiled tiled = {0};
	
	int i = 0, argi = 0;
	int dryRun = 0;
	
	// Parse main arguments
	if ((verbose = (argEquals(argc, argv, "-v") != -1)))
		showTitle();
	dryRun = argEquals(argc, argv, "-d") != -1;
	
	if ((argi = argFilename(argc, argv)) != -1)
		tmxFilename = argv[argi];
	
	if (!tmxFilename) {
		showUsage();
		i = 12;
		goto out;
	}
	
	if (verbose)
		printf("Input file: %s\n", tmxFilename);
	
	// Initializes container
	tiledInit(&tiled, argc, argv);
	binFilename = append(tmxFilename, ".bin");
	
	if (verbose)
		printf("Output file: %s\n", binFilename);

	// Main code
	if (verbose)
		printf("Reading input file...\n");
	if (!(tmxFile = fopen(tmxFilename, "rt"))) {
		printf("ERROR: Could not open %s.\n", tmxFilename);
		i = 13;
		goto out;
	}
	if ((i = tmxReaderRead(tmxFile, &tiled)))
		goto out;
	
	if (!dryRun) {
		if (verbose)
			printf("Writing output file...\n");
		if (!(binFile = fopen(binFilename, "wb"))) {
			printf("ERROR: Could not create %s.\n", binFilename);
			i = 14;
			goto out;
		}
		if ((i = tiledWrite(&tiled, binFile))) {
			printf("ERROR: Failed writing %s.\n", binFilename);
			i = 15;
			goto out;
		}
	}
	
	if (verbose)
		printf("Done!\n");
	
out:
	// Exit gracefully
	if (binFilename) free(binFilename);
	if (tmxFile) fclose(tmxFile);
	if (binFile) fclose(binFile);
	tiledDone(&tiled);
	return i;
}

/* Function bodies ------------------------------------- */

/*
 * Show application title
 */
void showTitle() {

	if (titleShown)
		return;
	printf("TMX2BIN: A tool to convert simple Tiled maps to binary\n");
	titleShown = 1;
}

/*
 * Show application usage
 */
void showUsage() {

	showTitle();
	printf("Usage:\n");
	printf("\tTMX2BIN [options] tmxFilename\n"); // [outputFilename]\n");
	printf("where:\n");
	printf("\ttmxFilename\tinput TMX file\n");
	// printf("\toutputFilename output BIN file (default is output.BIN)\n");
	printf("options are:\n");
	printf("\t-v\tverbose execution\n");
	printf("\t-d\tdry run. Doesn't write output files\n");
	tiledOptions();
}
