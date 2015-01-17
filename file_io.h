#ifndef FILE_IO_H
#define FILE_IO_H

/* needed for getline() and getdelim() */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Reads a whole line from stream, with trailing newline if it's present.
 * Returns NULL on {error or EOF} */
char *read_line(FILE *stream);
/* Reads string with delimiter, removing delimiter from the end.
 * Returns NULL on {error or EOF} */
char *read_delimited(FILE *stream, int delim);

/* runs read_delimited and converts the result to int32_t */
int32_t read_int_delimited(FILE *stream, int delim);

#endif /* FILE_IO_H */
