#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct
{
    char *directory;
    char *filename;
    bool ignore_target_errors;
    bool print_shell_commands;
    bool print_bakefile;
    bool execute_silently;
} BakeConfig;

BakeConfig *new_config();

void free_config(BakeConfig *config);

#endif // CONFIG_H
