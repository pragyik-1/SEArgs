#ifndef SEARGS_H
#define SEARGS_H

#include <stdbool.h>

#define ARG_DEF(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, REQUIRED, DEFAULT)   \
  (arg_def_t) {                                                                \
    .name = LONG_NAME, .short_name = SHORT_NAME, .desc = DESCRIPTION,          \
    .required = REQUIRED, .type = TYPE, .default_val = DEFAULT,                \
  }

#define REQUIRED_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION)                 \
  ARG_DEF(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, true, (arg_val_t){0})

#define OPTIONAL_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, DEFAULT)        \
  ARG_DEF(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, false, DEFAULT)

// NOTE: Only use this macro if defs is a static array in scope, doesnt work
// with pointers if so use parse_args()
#define PARSE_ARGS(argc, argv, defs)                                           \
  ((defs) ? parse_args(argc, argv, defs, sizeof(defs) / sizeof(defs[0])) : NULL)

// This expects you to pass a static string, dynamic strings may cause dangling
// pointers.
#define STRING_VAL(value) ((arg_val_t){.string_val = (value)})
#define INT_VAL(value) ((arg_val_t){.int_val = (value)})
// Usually presence of the flag determines its effects so it will always be
// defaulted as false
#define FLAG_VAL ((arg_val_t){.flag_val = false})
#define DOUBLE_VAL(value) ((arg_val_t){.double_val = (value)})

// The type of the arg (int, float, string or flag)
typedef enum { ARG_FLAG, ARG_INT, ARG_STRING, ARG_DOUBLE } arg_type_t;

typedef union {
  bool flag_val;
  int int_val;
  char *string_val;
  double double_val;
} arg_val_t;

// Representation of an arg, provides necessary metadata
typedef struct {
  const char *name;
  const char short_name;
  const char *desc;
  const bool required;
  const arg_type_t type;
  const arg_val_t default_val;
} arg_def_t;

// State of the argument, contains its value and whether it is found or not.
typedef struct {
  arg_val_t value;
  bool found;
  bool string_allocated;
} arg_state_t;

// list of args with their definitions and states.
typedef struct {
  int num_args;
  const arg_def_t *defs;
  arg_state_t *states;
} args_t;

args_t *parse_args(int argc, const char *argv[], const arg_def_t *args_defs,
                   int num_args);
const arg_def_t *get_arg_def(const arg_def_t valid_args[], const char *name,
                             int num_defs);
void *get_arg_val(args_t *args, const char *name);
void free_args(args_t **p_args);
#endif
