#ifndef UTIL_H
#define UTIL_H

char *trim_left(char *string);

char *trim_right(char *string);

char *trim(char *string);

char *character_replace(char *string, char *targets, char replace_with);

char **string_split(char *string, char *delimiter, size_t *size);

void usage();

#endif // UTIL_H
