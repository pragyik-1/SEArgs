#ifndef SEARGS_H
#define SEARGS_H

#include <errno.h>
#include <stdbool.h>
#include <string.h>

// -------------------- MACROS TO BE USED BY THE USER --------------------------
#define REQUIRED_INT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                   \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_INT, DESCRIPTION, true, (arg_val_t){0})
#define REQUIRED_DOUBLE_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_DOUBLE, DESCRIPTION, true, (arg_val_t){0})
// expects static strings
#define REQUIRED_STRING_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_STRING, DESCRIPTION, true, (arg_val_t){0})

// using explicit macros you dont need to wrap your value with macros like
// INT_VAL() on the user side as thats already handled
#define OPTIONAL_INT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)          \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_INT, DESCRIPTION, false, INT_VAL(DEFAULT))
#define OPTIONAL_DOUBLE_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)       \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_DOUBLE, DESCRIPTION, false,               \
          DOUBLE_VAL(DEFAULT))
// expects static strings
#define OPTIONAL_STRING_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)       \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_STRING, DESCRIPTION, false,               \
          STRING_VAL(DEFAULT))
#define FLAG_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                           \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_FLAG, DESCRIPTION, false, FLAG_VAL)

#define seargs_ok(p) ((p) && (p)->error.code == SEARGS_OK)
#define seargs_err(p) (!(p) || (p)->error.code != SEARGS_OK)

/* Clears error, evaluates expr, returns its value */
#define seargs_try(p, expr)                                                    \
  ((p)->error =                                                                \
       (seargs_error_t){.code = SEARGS_OK, .msg = NULL, .arg_name = NULL},     \
   (expr))

#define GET_INT_ARG(parser, name) seargs_try(parser, get_int_arg_(parser, name))
#define GET_DOUBLE_ARG(parser, name)                                           \
  seargs_try(parser, get_double_arg_(parser, name))
#define GET_FLOAT_ARG(parser, name)                                            \
  seargs_try(parser, get_float_arg_(parser, name))
#define GET_STRING_ARG(parser, name)                                           \
  seargs_try(parser, get_string_arg_(parser, name))
#define GET_FLAG_ARG(parser, name)                                             \
  seargs_try(parser, get_flag_arg_(parser, name))

// -----------------------------------------------------------------------------

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

typedef enum {
  SEARGS_OK = 0,
  MISSING_ARG_ERR,
  INVALID_VALUE_ERR,
  UNKNOWN_ARG_ERR,
  MISSING_VALUE_ERR,
  INVALID_ARG_ERR,
} seargs_err_codes;

typedef struct {
  seargs_err_codes code;
  const char *msg;
  const char *arg_name;
} seargs_error_t;

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
  seargs_error_t error;
  const char **pos_args;
  int num_pos_args;
} parser_t;

parser_t *parse_args(int argc, const char *argv[], const arg_def_t *args_defs,
                     size_t num_args);
const arg_def_t *get_arg_def(const arg_def_t valid_args[], const char *name,
                             int num_defs);
// Returns a void pointer with the value of the arg, strings are returned as is
// and not a pointer, Returns null if not found
void *get_arg_val(parser_t *parser, const char *name);
void free_parser(parser_t **p_parser);
bool validate_arg_defs(const arg_def_t *defs, int num_args);
void print_help(const arg_def_t *defs, int num_args);

// ##########
// <-------------- INTERNAL HELPER MACROS AND FUNCTIONS ------------------->
// ##########
static bool contains_format_specifier(const char *str) {
  if (!str)
    return false;
  return strchr(str, '%') != NULL;
}

static inline bool has_arg(parser_t *parser, const char *name) {
  for (int i = 0; i < parser->num_args; i++)
    if (strcmp(parser->defs[i].name, name) == 0)
      return parser->states[i].found;
  return false;
}

// gets the int value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns 0
static inline int get_int_arg_(parser_t *parser, const char *name) {

  const int *v = (const int *)get_arg_val(parser, name);
  if (!v) {
    parser->error.code = INVALID_VALUE_ERR;
    return 0;
  }
  return *v;
}

// gets the double value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns 0
static inline double get_double_arg_(parser_t *parser, const char *name) {
  const double *v = (const double *)get_arg_val(parser, name);
  if (!v) {
    parser->error.code = INVALID_VALUE_ERR;
    return 0.0;
  }
  return *v;
}

// gets the float value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns 0
static inline float get_float_arg_(parser_t *parser, const char *name) {
  return (float)get_double_arg_(parser, name);
}

// gets the char * value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns NULL
static inline const char *get_string_arg_(parser_t *parser, const char *name) {
  const char **v = (const char **)get_arg_val(parser, name);
  if (!v) {
    parser->error.code = INVALID_VALUE_ERR;
    return NULL;
  }
  return *v;
}

// gets the boolean/flag value of an argument by its name. On Failure: sets the
// global errno as EINVAL and returns false
static inline bool get_flag_arg_(parser_t *parser, const char *name) {
  const bool *v = (const bool *)get_arg_val(parser, name);
  if (!v) {
    parser->error.code = INVALID_VALUE_ERR;
    return false;
  }
  return *v;
}

#endif