#ifndef SEARGS_H
#define SEARGS_H

#include <errno.h>
#include <stdbool.h>
#include <string.h>

// <-------- Constant (Configurable) Macros -------->
#define MAX_SHORT_ARGS 16
// <------------------------------------------------>

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
  const char **pos_args;
  int num_pos_args;
} args_t;

args_t *parse_args(int argc, const char *argv[], const arg_def_t *args_defs,
                   size_t num_args);
const arg_def_t *get_arg_def(const arg_def_t valid_args[], const char *name,
                             int num_defs);
// Returns a void pointer with the value of the arg, strings are returned as is
// and not a pointer, Returns null if not found
void *get_arg_val(args_t *args, const char *name);
void free_args(args_t **p_args);
bool validate_arg_defs(const arg_def_t *defs, int num_args);
void print_help(const arg_def_t *defs, int num_args);

// ##########
// <-------------- INTERNAL HELPER MACROS AND FUNCTIONS ------------------->
// ##########
static bool contains_format_specifier(const char *str) {
  if (!str) return false;
  return strchr(str, '%') != NULL;
}

// ##########
// <-------------- API HELPER MACROS AND FUNCTIONS ------------------->
// ##########

#define REQUIRED_INT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                   \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_INT, DESCRIPTION, true, (arg_val_t){0})
#define REQUIRED_DOUBLE_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_DOUBLE, DESCRIPTION, true, (arg_val_t){0})
#define REQUIRED_STRING_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_STRING, DESCRIPTION, true, (arg_val_t){0})

// using explicit macros you dont need to wrap your value with macros like
// INT_VAL() on the user side as thats already handled
#define OPTIONAL_INT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)          \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_INT, DESCRIPTION, false, INT_VAL(DEFAULT))
#define OPTIONAL_DOUBLE_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)       \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_DOUBLE, DESCRIPTION, false,               \
          DOUBLE_VAL(DEFAULT))
#define OPTIONAL_STRING_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)       \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_STRING, DESCRIPTION, false,               \
          STRING_VAL(DEFAULT))
#define FLAG_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                           \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_FLAG, DESCRIPTION, false, FLAG_VAL)

// gets the int value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns 0
static inline int get_int_arg(args_t *args, const char *name) {
  
  const int *v = (const int *)get_arg_val(args, name);
  if (!v) {
    errno = EINVAL;
    return 0;
  }
  return *v;
}

// gets the double value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns 0
static inline double get_double_arg(args_t *args, const char *name) {
  const double *v = (const double *)get_arg_val(args, name);
  if (!v) {
    errno = EINVAL;
    return 0.0;
  }
  return *v;
}

// gets the float value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns 0
static inline float get_float_arg(args_t *args, const char *name) {
  return (float)get_double_arg(args, name);
}

// gets the char * value of an argument by its name, On Failure: sets the global
// errno as EINVAL and returns an empty string ""
static inline const char *get_string_arg(args_t *args, const char *name) {
  const char **v = (const char **)get_arg_val(args, name);
  if (!v) {
    errno = EINVAL;
    return NULL;
  }
  return *v;
}

// gets the boolean/flag value of an argument by its name. On Failure: sets the
// global errno as EINVAL and returns false
static inline bool get_flag_arg(args_t *args, const char *name) {
  const bool *v = (const bool *)get_arg_val(args, name);
  if (!v) {
    errno = EINVAL;
    return false;
  }
  return *v;
}
#endif