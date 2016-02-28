#ifndef GETLINE_C
#define GETLINE_C

int getstr (char ** lineptr, size_t *n, FILE * stream, char terminator, int offset);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

#endif
