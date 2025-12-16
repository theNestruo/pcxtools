/*
 * Support routines for arguments
 * Coded by theNestruo
 */

#ifndef ARGS_H_INCLUDED
#define ARGS_H_INCLUDED

/* Function prototypes ----------------------------------------------------- */

int argFilename(int argc, char **argv);

int argNextFilename(int argc, char **argv, int last);

int argEquals(int argc, char **argv, char *argstring);

int argStartsWith(int argc, char **argv, char *argstring, size_t minlen);

//

int startsWith(char *string, char *prefix);

int endsWith(char *string, char *suffix);

char *append(char *a, char *b);

//

int decimalInt(char *value);

int decimalDigit(char c);

int hexadecimalInt(char *value);

int hexadecimalNibble(char c);

#endif // ARGS_H_INCLUDED
