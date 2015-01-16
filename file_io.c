#include "file_io.h"

/* Reads a whole line from stream, with trailing newline if it's present.
 * Returns NULL on {error or EOF} */
char *read_line(FILE *stream)
{
    char *line = NULL;
    size_t buffer_len = 0;

    ssize_t len = getline(&line, &buffer_len, stream);

    if (len == -1) {
        free(line);
        return NULL;
    } else {
        return line;
    }
}

/* Reads string with delimiter, removing delimiter from the end.
 * Returns NULL on {error or EOF} */
char *read_delimited(FILE *stream, int delim)
{
    char *str = NULL;
    size_t buffer_len = 0;

    ssize_t len = getdelim(&str, &buffer_len, delim, stream);

    if (len == -1) {
        free(str);
        return NULL;
    }

    /* remove delimiter at the end */
    if (str[len - 1] == delim)
        str[len - 1] = '\0';
    /* TODO shrink allocated size after this? */

    return str;
}

int32_t read_int_delimited(FILE *stream, int delim)
{
    /* No error checking for now */
    char *str;
    long result;

    str = read_delimited(stream, delim);
    /* atol (long), because on some systems int may be lower than int32_t ? */
    result = atol(str);
    free(str);

    return result;
}
