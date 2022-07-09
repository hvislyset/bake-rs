#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "../include/parser.h"
#include "../include/read.h"
#include "../include/util.h"
#include "../include/variable.h"

BakeParser *new_parser()
{
    BakeParser *parser = calloc(4, sizeof(BakeParser));

    if (!parser)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    return parser;
}

void free_parser(BakeParser *parser)
{
    free(parser);
}

void run_parser(BakeParser *parser, BakeContext *context)
{
    FILE *fp = read_bakefile(context->config->filename);
    char line[BUFSIZ];
    char *line_buffer = calloc(1, sizeof(char));

    if (!line_buffer)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    while (fgets(line, BUFSIZ, fp))
    {
        size_t line_size = strlen(line);

        if (line[0] == BAKE_COMMENT)
        {
            continue;
        }

        if (!parser->is_line_continuation)
        {
            line_buffer = realloc(line_buffer, line_size + 1);

            if (!line_buffer)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            memset(line_buffer, 0, line_size + 1);
            strncpy(line_buffer, line, line_size);
        }
        else
        {
            line_buffer = trim(character_replace(line_buffer, "\\\n", ' '));

            character_replace(line, "\t", ' ');

            size_t new_size = line_size + strlen(line_buffer) + 1;

            line_buffer = realloc(line_buffer, new_size);

            if (!line_buffer)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strncat(line_buffer, line, new_size);

            parser->is_line_continuation = false;
        }

        if (line_size > 2 && line[line_size - 2] == BAKE_LINE_CONTINUATION)
        {
            parser->is_line_continuation = true;

            continue;
        }

        parse_line(parser, context, line_buffer);
        parser->line_number++;
    }

    free(line_buffer);

    fclose(fp);
}

void parse_line(BakeParser *parser, BakeContext *context, const char *line)
{
    size_t offset = 0;

    char *line_d = expand_variables(context->variables, line);

    while (line_d[offset] != '\0')
    {
        if (line_d[offset] == BAKE_VARIABLE_ASSIGNMENT)
        {
            size_t identifier = offset - 1;

            char *variable_name = calloc(offset + 1, sizeof(char));

            if (!variable_name)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            char *variable_value = calloc(strlen(line_d) - identifier, sizeof(char));

            if (!variable_value)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strncpy(variable_name, line_d, identifier + 1);
            strncpy(variable_value, line_d + identifier + 2, strlen(line_d) - identifier);

            char *variable_name_d = strdup(variable_name);
            char *variable_value_d = strdup(variable_value);
            variable_set(context->variables, trim(variable_name_d), trim(variable_value_d));

            free(variable_name);
            free(variable_value);
            free(variable_name_d);
            free(variable_value_d);

            break;
        }

        if (line_d[offset] == BAKE_TARGET_DEFINITION)
        {
            size_t identifier = offset - 1;

            char *target_name = calloc(offset + 1, sizeof(char));

            if (!target_name)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            char *dependencies = calloc(strlen(line_d) - identifier, sizeof(char));

            if (!dependencies)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strncpy(target_name, line_d, identifier + 1);
            strncpy(dependencies, line_d + identifier + 2, strlen(line_d) - identifier);

            char *target_name_d = strdup(target_name);
            BakeTarget *new_target = target_list_add(context->targets, trim(target_name_d));

            if (!new_target)
            {
                new_target = find_target(context->targets, target_name_d);

                if (!new_target)
                {
                    fprintf(stderr, "Unable to add or find existing target\n");

                    exit(EXIT_FAILURE);
                }
            }

            size_t dependencies_count;
            char *dependencies_d = strdup(dependencies);
            char **dependencies_list = string_split(trim(dependencies_d), " ", &dependencies_count);

            add_target_dependencies(context->targets, new_target, dependencies_count, dependencies_list);

            parser->is_target_action = true;
            parser->current_target = new_target;

            free(target_name);
            free(target_name_d);
            free(dependencies);
            free(dependencies_d);
            free(dependencies_list);

            break;
        }

        if (line_d[offset] == BAKE_TARGET_ACTION)
        {
            size_t identifier = strlen(line_d);

            if (!parser->is_target_action)
            {
                fprintf(stderr, "Target action appeared before or after target definition\n");

                exit(EXIT_FAILURE);
            }

            char *action = calloc(identifier + 1, sizeof(char));

            if (!action)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strncpy(action, line_d, identifier + 1);

            char *action_result = trim(character_replace(action, "\\\n", ' '));

            add_target_action(parser->current_target, action_result);

            free(action);

            break;
        }

        parser->is_target_action = false;

        offset++;
    }

    free(line_d);
}
