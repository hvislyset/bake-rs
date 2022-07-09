#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdlib.h>
#include <stdbool.h>

#define VARIABLE_TABLE_INITIAL_SIZE 100000

typedef struct bake_variable
{
    char *name;
    char *value;
    struct bake_variable *next;
} BakeVariable;

typedef struct
{
    size_t size;
    size_t count;
    BakeVariable **variables;
} BakeVariableTable;

BakeVariableTable *new_variable_table();

void free_variable_table(BakeVariableTable *variable_table);

BakeVariable *new_variable(const char *name, const char *value);

void free_variable(BakeVariable *variable);

size_t hash(const char *name, size_t table_size);

char *variable_get(BakeVariableTable *variable_table, const char *name);

void variable_set(BakeVariableTable *variable_table, const char *name, const char *value);

char *expand_variables(BakeVariableTable *variable_table, const char *string);

bool is_reserved_identifier(const char *name);

#endif // VARIABLE_H
