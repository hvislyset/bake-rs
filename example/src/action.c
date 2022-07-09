#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "../include/action.h"

void execute_target_actions(BakeContext *context, size_t actions_count, char **actions)
{
    char *default_shell = getenv("SHELL");

    if (!default_shell)
    {
        default_shell = strdup("/bin/bash");
    }

    for (size_t i = 0; i < actions_count; i++)
    {
        if (!execute_target_action(context, actions[i], default_shell))
        {
            fprintf(stdout, "Error executing one or more actions\n");
        }
    }
}

bool execute_target_action(BakeContext *context, char *action, char *shell)
{
    bool suppress_stdout = false;
    bool ignore_failures = false;

    if (action[0] == BAKE_SUPPRESS_STDOUT_MODIFIER)
    {
        suppress_stdout = true;

        action++;
    }

    if (action[0] == BAKE_IGNORE_FAILURES_MODIFIER)
    {
        ignore_failures = true;

        action++;
    }

    if (context->config->ignore_target_errors)
    {
        ignore_failures = true;
    }

    if (context->config->print_shell_commands)
    {
        suppress_stdout = true;
    }

    if (context->config->execute_silently)
    {
        suppress_stdout = true;
    }

    if (!context->config->print_shell_commands)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            fprintf(stderr, "Failed to execute target action\n");

            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            if (execl(shell, shell, "-c", action, NULL) < 0 && !ignore_failures)
            {
                return false;
            }
        }
        else
        {
            int status;

            wait(&status);
        }
    }

    if (!suppress_stdout || context->config->print_shell_commands)
    {
        fprintf(stdout, "%s\n", action);
    }

    return true;
}
