/*
 This is not necessary to run the library
 This file provides some useful helper macros
 To hoperfully make the developer experience a bit better
*/

#include "seargs.h"
#include <errno.h>
#include <stdio.h>

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
    return "";
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
