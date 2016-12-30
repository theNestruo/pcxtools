/*
 * Support routines for arguments
 * Coded by theNestruo
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "args.h"

/* Function bodies --------------------------------------------------------- */

int argFilename(int argc, char **argv) {
	
	int i = argNextFilename(argc, argv, 0);
	return i;
}

int argNextFilename(int argc, char **argv, int last) {
	
	int i;
	if (argv) for (i = last +1; i < argc; i++) {
		char *arg = argv[i];
		
		if (arg && strlen(arg) && (arg[0] != '-')) {
			// Found: not null, not empty, not a modifier
			return i;
		}
	}
	
	// Not found
	return -1;
}

int argEquals(int argc, char **argv, char *argstring) {
	
	int i;
	if (argv && argstring) for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (arg && strlen(arg) && (!strcmp(arg, argstring))) {
			// Found: not null, not empty, equals
			return i;
		}
	}
	
	// Not found
	return -1;
}

int argStartsWith(int argc, char **argv, char *argstring, int minlen) {
	
	int i;
	if (argv && argstring) for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (arg && (strlen(arg) >= minlen) && startsWith(arg, argstring)) {
			// Found: not null, not empty, has minimum length, starts with
			return i;
		}
	}
	
	// Not found
	return -1;
}


int startsWith(char *string, char *prefix) {

	int stringLength = strlen(string);
	int prefixLength = strlen(prefix);
	return (prefixLength <= stringLength)
		? (strncmp(prefix, string, prefixLength) == 0)
		: 0;
}

int endsWith(char *string, char *suffix) {

	int stringLength = strlen(string);
	int suffixLength = strlen(suffix);
	return (suffixLength <= stringLength)
		? (strncmp(suffix, string + stringLength - suffixLength, suffixLength) == 0)
		: 0;
}

char *append(char *a, char *b) {
	
	char *result = (char*) calloc (strlen(a) + strlen(b) + 1, sizeof(char));
	strcpy(result, a);
	strcat(result, b);
	return result;
}

int decimalInt(char *value) {

	int ret;
	for (ret = 0; *value; value++) {
		ret *= 10;
		ret += decimalDigit(*value);
	}
	return ret;
}

int decimalDigit(char c) {

	return ((c >= '0') && (c <= '9')) ? (c - '0')
		: 0;
}

int hexadecimalInt(char *value) {

	int ret;
	for (ret = 0x00; *value; value++) {
		ret *= 0x10;
		ret += hexadecimalNibble(*value);
	}
	return ret;
}

int hexadecimalNibble(char c) {

	return ((c >= '0') && (c <= '9')) ? (c - '0')
		: ((c >= 'a') && (c <= 'f')) ? (c - 'a' + 10)
		: ((c >= 'A') && (c <= 'F')) ? (c - 'F' + 10)
		: 0;
}
