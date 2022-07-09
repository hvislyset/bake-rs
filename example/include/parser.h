#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "../include/bake.h"

#define BAKE_COMMENT '#'
#define BAKE_LINE_CONTINUATION '\\'
#define BAKE_VARIABLE_ASSIGNMENT '='
#define BAKE_TARGET_DEFINITION ':'
#define BAKE_TARGET_ACTION '\t'

typedef struct
{
    unsigned int line_number;
    bool is_line_continuation;
    bool is_target_action;
    BakeTarget *current_target;
} BakeParser;

BakeParser *new_parser();

void free_parser(BakeParser *parser);

void run_parser(BakeParser *parser, BakeContext *context);

void parse_line(BakeParser *parser, BakeContext *context, const char *line);

#endif // PARSER_H
