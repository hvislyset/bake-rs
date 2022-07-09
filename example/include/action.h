#ifndef ACTION_H
#define ACTION_H

#include <stdlib.h>
#include <stdbool.h>

#include "../include/bake.h"

#define BAKE_SUPPRESS_STDOUT_MODIFIER '@'
#define BAKE_IGNORE_FAILURES_MODIFIER '-'

void execute_target_actions(BakeContext *context, size_t actions_count, char **actions);

bool execute_target_action(BakeContext *context, char *action, char *shell);

#endif // ACTION_H
