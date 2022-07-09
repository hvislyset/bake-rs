#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "../include/bake.h"
#include "../include/parser.h"
#include "../include/util.h"

int main(int argc, char **argv)
{
    BakeContext *context = new_context();
    char *target = calloc(BUFSIZ, sizeof(char));

    if (!target)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    int option;

    while ((option = getopt(argc, argv, "C:f:inpsh")) != -1)
    {
        switch (option)
        {
        case 'C':
            context->config->directory = strdup(optarg);

            if (chdir(context->config->directory) < 0)
            {
                fprintf(stderr, "Could not change in to directory: %s\n", context->config->directory);

                exit(EXIT_FAILURE);
            }

            break;

        case 'f':
            context->config->filename = strdup(optarg);

            break;

        case 'i':
            context->config->ignore_target_errors = true;

            break;

        case 'n':
            context->config->print_shell_commands = true;

            break;

        case 'p':
            context->config->print_bakefile = true;

            break;

        case 's':
            context->config->execute_silently = true;

            break;

        case 'h':
            usage();

            exit(EXIT_SUCCESS);

            break;

        case '?':
            break;
        }
    }

    if (optind < argc)
    {
        strncpy(target, argv[optind], strlen(argv[optind]));
    }

    srand(time(NULL));

    BakeParser *parser = new_parser();

    run_parser(parser, context);
    run_build(context, target);

    free_parser(parser);
    free_context(context);
    free(target);

    return 0;
}
