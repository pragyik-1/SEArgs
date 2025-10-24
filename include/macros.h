/*
 This is not necessary to run the library
 This file provides some useful helper macros
 To hoperfully make the developer experience a bit better
*/
#define GET_INT_ARG(args, name) (*(int *)get_arg_val(args, name))
#define GET_FLOAT_ARG(args, name) (*(float *)get_arg_val(args, name))
#define GET_STRING_ARG(args, name) (*(char **)get_arg_val(args, name))
#define GET_FLAG_ARG(args, name) (*(bool *)get_arg_val(args, name))

#define REQUIRED_INT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                   \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_INT, DESCRIPTION, true, (arg_val_t){0})
#define REQUIRED_FLOAT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                 \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_FLOAT, DESCRIPTION, true, (arg_val_t){0})
#define REQUIRED_STRING_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_STRING, DESCRIPTION, true, (arg_val_t){0})
#define REQUIRED_FLAG_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                  \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_FLAG, DESCRIPTION, true, (arg_val_t){0})

// using explicit macros you dont need to wrap your value with macros like
// INT_VAL() on the user side as thats already handled
#define OPTIONAL_INT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)          \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_INT, DESCRIPTION, false, INT_VAL(DEFAULT))
#define OPTIONAL_FLOAT_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)        \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_FLOAT, DESCRIPTION, false,                \
          FLOAT_VAL(DEFAULT))
#define OPTIONAL_STRING_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION, DEFAULT)       \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_STRING, DESCRIPTION, false,               \
          STRING_VAL(DEFAULT))
#define OPTIONAL_FLAG_ARG(LONG_NAME, SHORT_NAME, DESCRIPTION)                  \
  ARG_DEF(LONG_NAME, SHORT_NAME, ARG_FLAG, DESCRIPTION, false, FLAG_VAL)
// NOTE: Only use this macro if defs is a static array in scope, doesnt work
// with pointers if so use get_arg_def()
#define GET_DEF(valid_args, name)                                              \
  get_arg_def(valid_args, name, sizeof(valid_args) / sizeof(valid_args[0]))