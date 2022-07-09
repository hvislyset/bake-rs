#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/util.h"

char *trim_left(char *string)
{
    while (isspace((unsigned char)*string))
    {
        string++;
    }

    if (*string == 0)
    {
        return string;
    }

    char *end = string + strlen(string) - 1;

    end[1] = '\0';

    return string;
}

char *trim_right(char *string)
{
    if (*string == 0)
    {
        return string;
    }

    char *end = string + strlen(string) - 1;

    while (end > string && isspace((unsigned char)*end))
    {
        end--;
    }

    end[1] = '\0';

    return string;
}

char *trim(char *string)
{
    return trim_left(trim_right(string));
}

char *character_replace(char *string, char *targets, char replace_with)
{
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (strchr(targets, string[i]))
        {
            string[i] = replace_with;
        }
    }

    return string;
}

char **string_split(char *string, char *delimiter, size_t *size)
{
    size_t initial_size = 100;
    char **result = calloc(initial_size, sizeof(char *));

    if (!result)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    size_t token_count = 0;

    char *token = strtok(string, delimiter);

    while (token)
    {
        token_count++;

        if (token_count > initial_size)
        {
            initial_size *= 2;
            result = realloc(result, token_count);
        }

        result[token_count - 1] = token;

        token = strtok(NULL, delimiter);
    }

    *size = token_count;

    return result;
}

void usage()
{
    fprintf(stdout, "Usage: bake [options...] <target>\n\n \
    \t-C directoryname,    read the specification file from the specified directory\n \
    \t-f filename,         instead of reading from Bakefile or bakefile, read from the indicated file\n \
    \t-i,                  ignore unsuccessful termination of actions\n \
    \t-n,                  print each action without executing it\n \
    \t-p,                  print the internal representation of the specification file\n \
    \t-s,                  execute silently\n");
}
