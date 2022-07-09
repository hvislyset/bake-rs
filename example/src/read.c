#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/read.h"

FILE *read_bakefile(const char *filepath)
{
    FILE *fp;

    if (!filepath)
    {
        if (access("Bakefile", F_OK) == 0)
        {
            filepath = "Bakefile";
        }
        else if (access("bakefile", F_OK) == 0)
        {
            filepath = "bakefile";
        }
        else
        {
            fprintf(stderr, "Couldn't find a valid specification file in the current directory\n");

            exit(EXIT_FAILURE);
        }
    }

    fp = fopen(filepath, "r");

    if (!fp)
    {
        fprintf(stderr, "Couldn't open %s\n", filepath);

        exit(EXIT_FAILURE);
    }

    return fp;
}
