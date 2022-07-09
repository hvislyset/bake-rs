#include <stdio.h>

#include "../include/bake.h"
#include "../include/action.h"
#include "../include/util.h"

BakeContext *new_context()
{
    BakeContext *context = calloc(3, sizeof(BakeContext));

    if (!context)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    context->config = new_config();
    context->variables = new_variable_table();
    context->targets = new_target_list();

    return context;
}

void free_context(BakeContext *context)
{
    free_config(context->config);
    free_variable_table(context->variables);
    free_target_list(context->targets);
    free(context);
}

void dfs(BakeContext *context, BakeTarget *target, int *visited)
{
    visited[target->key] = GRAY;

    for (size_t i = 0; i < target->dependencies_count; i++)
    {
        BakeTarget *d_target = target->dependencies[i];

        if (visited[d_target->key] == GRAY)
        {
            printf("Cycle exists\n");

            exit(EXIT_FAILURE);
        }

        if (!visited[d_target->key])
        {
            dfs(context, d_target, visited);
        }
    }

    visited[target->key] = BLACK;

    if (target_requires_rebuilding(target))
    {
        execute_target_actions(context, target->actions_count, target->actions);
    }
}

void recursive_build(BakeContext *context, BakeTarget *target, size_t total_targets)
{
    int *visited = calloc(total_targets, sizeof(int));

    if (!visited)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < target->dependencies_count; i++)
    {
        BakeTarget *d_target = target->dependencies[i];

        if (!visited[d_target->key])
        {
            dfs(context, d_target, visited);
        }
    }

    if (target_requires_rebuilding(target))
    {
        execute_target_actions(context, target->actions_count, target->actions);
    }

    free(visited);
}

void run_build(BakeContext *context, const char *target)
{
    if (target[0] != '\0')
    {
        BakeTarget *btarget = find_target(context->targets, target);

        if (!btarget)
        {
            fprintf(stdout, "Nothing to do for target: %s\n", target);

            exit(EXIT_SUCCESS);
        }

        if (!context->config->print_bakefile)
        {
            recursive_build(context, btarget, context->targets->count);
        }
        else
        {
            print_internal_representation(context);

            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        BakeTarget *btarget = find_default_target(context->targets);

        if (!btarget)
        {
            fprintf(stdout, "No targets specified\n");

            exit(EXIT_SUCCESS);
        }
        else
        {
            if (!context->config->print_bakefile)
            {
                recursive_build(context, btarget, context->targets->count);
            }
            else
            {
                print_internal_representation(context);

                exit(EXIT_SUCCESS);
            }
        }
    }
}

void print_internal_representation(BakeContext *context)
{
    BakeVariableTable *variable_table = context->variables;
    BakeTargetList *target_list = context->targets;

    printf("=== VARIABLES ===\n\n");
    for (size_t i = 0; i < variable_table->size; i++)
    {
        if (variable_table->variables[i])
        {
            printf("%s = %s\n", variable_table->variables[i]->name, variable_table->variables[i]->value);
        }
    }

    printf("\n=== TARGETS ===\n");
    for (size_t i = 0; i < target_list->count; i++)
    {
        printf("\nTARGET:\t%s\n", target_list->targets[i]->name);
        printf("DEPENDENCIES:\n");

        for (size_t j = 0; j < target_list->targets[i]->dependencies_count; j++)
        {
            printf("\t- %s\n", target_list->targets[i]->dependencies[j]->name);
        }

        printf("ACTIONS:\n");

        for (size_t k = 0; k < target_list->targets[i]->actions_count; k++)
        {
            printf("\t- %s\n", target_list->targets[i]->actions[k]);
        }
    }
}
