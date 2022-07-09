#ifndef TARGET_H
#define TARGET_H

#include <stdlib.h>
#include <stdbool.h>

#define TARGET_LIST_INITIAL_SIZE 1000
#define ACTIONS_INITIAL_SIZE 100
#define DEPENDENCIES_INITIAL_SIZE 100

enum VisitedStates
{
    WHITE,
    GRAY,
    BLACK,
};

typedef struct bake_target
{
    char *name;
    size_t actions_size;
    size_t actions_count;
    char **actions;
    bool is_default_target;
    size_t key;
    size_t dependencies_size;
    size_t dependencies_count;
    struct bake_target **dependencies;
} BakeTarget;

typedef struct
{
    size_t size;
    size_t count;
    BakeTarget **targets;
} BakeTargetList;

BakeTargetList *new_target_list();

void free_target_list(BakeTargetList *target_list);

BakeTarget *target_list_add(BakeTargetList *target_list, const char *name);

void resize_target_list(BakeTargetList *target_list);

BakeTarget *new_target(const char *name);

BakeTarget *find_target(BakeTargetList *target_list, const char *name);

BakeTarget *find_default_target(BakeTargetList *target_list);

void add_target_dependencies(BakeTargetList *target_list, BakeTarget *target, size_t dependencies_count, char **dependencies);

void resize_target_dependencies(BakeTarget *target);

void free_target(BakeTarget *target);

void add_target_action(BakeTarget *target, const char *action);

void resize_target_actions(BakeTarget *target);

bool target_requires_rebuilding(BakeTarget *target);

#endif // TARGET_H
