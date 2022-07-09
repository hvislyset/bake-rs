#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "../include/variable.h"

BakeVariableTable *new_variable_table()
{
    BakeVariableTable *table = calloc(3, sizeof(BakeVariableTable));

    if (!table)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    table->size = VARIABLE_TABLE_INITIAL_SIZE;
    table->variables = calloc(table->size, sizeof(BakeVariable));

    if (!table->variables)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    return table;
}

void free_variable_table(BakeVariableTable *variable_table)
{
    for (size_t i = 0; i < variable_table->size; i++)
    {
        BakeVariable *variable = variable_table->variables[i];

        if (variable)
        {
            free_variable(variable);
        }
    }

    free(variable_table->variables);
    free(variable_table);
}

BakeVariable *new_variable(const char *name, const char *value)
{
    BakeVariable *variable = calloc(3, sizeof(BakeVariable));

    if (!variable)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    variable->name = strdup(name);
    variable->value = strdup(value);
    variable->next = NULL;

    return variable;
}

void free_variable(BakeVariable *variable)
{
    free(variable->name);
    free(variable->value);

    BakeVariable *current = variable;

    while (current)
    {
        BakeVariable *temp = current;
        current = current->next;
        free(temp);
    }
}

size_t hash(const char *name, size_t table_size)
{
    size_t hash = 5381;
    int c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % table_size;
}

char *variable_get(BakeVariableTable *variable_table, const char *name)
{
    size_t bucket = hash(name, variable_table->size);

    BakeVariable *variable = variable_table->variables[bucket];

    while (variable)
    {
        if (strcmp(variable->name, name) == 0)
        {
            return variable->value;
        }

        variable = variable->next;
    }

    return "";
}

void variable_set(BakeVariableTable *variable_table, const char *name, const char *value)
{
    size_t bucket = hash(name, variable_table->size);

    BakeVariable *current_variable = variable_table->variables[bucket];

    if (!current_variable)
    {
        variable_table->variables[bucket] = new_variable(name, value);
        variable_table->count++;

        return;
    }

    BakeVariable *previous_variable;

    while (current_variable)
    {
        if (strcmp(current_variable->name, name) == 0)
        {
            free(current_variable->value);

            current_variable->value = strdup(value);

            return;
        }

        previous_variable = current_variable;
        current_variable = previous_variable->next;
    }

    previous_variable->next = new_variable(name, value);
    variable_table->count++;
}

char *expand_variables(BakeVariableTable *variable_table, const char *string)
{
    char *result = calloc(1, sizeof(char));

    if (!result)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    size_t left_offset = 0;

    while (string[offset] != '\0')
    {
        if (string[offset] == '$' && string[offset + 1] == '(')
        {
            size_t left_value_size = strlen(result) + offset + 1;
            result = realloc(result, left_value_size);

            if (!result)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            char *left_value = calloc(left_value_size, sizeof(char));

            if (!left_value)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strncpy(left_value, string + (left_offset), offset - left_offset);
            strncat(result, left_value, offset - left_offset);

            size_t variable_offset = offset + 2;
            size_t variable_length = 0;

            while (string[variable_offset] != ')')
            {
                if (string[variable_offset] == '\0')
                {
                    fprintf(stderr, "Incomplete variable reference\n");

                    exit(EXIT_FAILURE);
                }

                variable_offset++;
                variable_length++;
            }

            char *variable_name = calloc(variable_length + 1, sizeof(char));

            if (!variable_name)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strncpy(variable_name, string + (variable_offset - variable_length), variable_length);

            char *variable_value = variable_get(variable_table, variable_name);

            if (strcmp(variable_value, "") == 0)
            {
                variable_value = getenv(variable_name);

                if (!variable_value && is_reserved_identifier(variable_name))
                {
                    if (strcmp(variable_name, "PID") == 0)
                    {
                        pid_t pid = getpid();
                        char buf[11];

                        sprintf(buf, "%d", pid);

                        variable_value = buf;
                    }

                    if (strcmp(variable_name, "PPID") == 0)
                    {
                        pid_t ppid = getppid();
                        char buf[11];

                        sprintf(buf, "%d", ppid);

                        variable_value = buf;
                    }

                    if (strcmp(variable_name, "PWD") == 0)
                    {
                        char cwd[PATH_MAX];

                        if (getcwd(cwd, sizeof(cwd)))
                        {
                            sprintf(cwd, "%s", cwd);

                            variable_value = cwd;
                        }
                    }

                    if (strcmp(variable_name, "RAND") == 0)
                    {
                        char buf[11];

                        sprintf(buf, "%d", rand());

                        variable_value = buf;
                    }
                }
                else
                {
                    variable_value = "";
                }
            }

            size_t new_size = strlen(result) + strlen(variable_value) + 1;
            result = realloc(result, new_size);

            if (!result)
            {
                fprintf(stderr, "No memory\n");

                exit(EXIT_FAILURE);
            }

            strcat(result, variable_value);

            left_offset = variable_offset + 1;

            free(left_value);
            free(variable_name);
        }

        offset++;
    }

    if (left_offset != offset)
    {
        char *right_value = calloc(strlen(string) - left_offset + 1, sizeof(char));

        if (!right_value)
        {
            fprintf(stderr, "No memory\n");

            exit(EXIT_FAILURE);
        }

        strncpy(right_value, string + (left_offset), strlen(string) - left_offset);

        result = realloc(result, strlen(result) + strlen(right_value) + 1);

        if (!result)
        {
            fprintf(stderr, "No memory\n");

            exit(EXIT_FAILURE);
        }

        strcat(result, right_value);

        free(right_value);
    }

    return result;
}

bool is_reserved_identifier(const char *name)
{
    return strcmp(name, "PID") == 0 ||
           strcmp(name, "PPID") == 0 ||
           strcmp(name, "PWD") == 0 ||
           strcmp(name, "RAND") == 0;
}
