#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "../include/config.h"

BakeConfig *new_config()
{
    BakeConfig *config = calloc(6, sizeof(BakeConfig));

    if (!config)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "Couldn't get current working directory\n");

        exit(EXIT_FAILURE);
    }
    else
    {
        config->directory = strdup(cwd);
    }

    return config;
}

void free_config(BakeConfig *config)
{
    free(config->directory);
    free(config->filename);
    free(config);
}
