#ifndef BAKE_H
#define BAKE_H

#include "../include/config.h"
#include "../include/variable.h"
#include "../include/target.h"

typedef struct
{
    BakeConfig *config;
    BakeVariableTable *variables;
    BakeTargetList *targets;
} BakeContext;

BakeContext *new_context();

void free_context(BakeContext *context);

void run_build(BakeContext *context, const char *target);

void dfs(BakeContext *context, BakeTarget *target, int *visited);

void recursive_build(BakeContext *context, BakeTarget *target, size_t total_targets);

void print_internal_representation(BakeContext *context);

#endif // BAKE_H
