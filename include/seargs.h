#ifndef SEARGS_H
#define SEARGS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARG_DEF(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, REQUIRED, DEFAULT)   \
  (arg_def_t) {                                                                \
    .name = LONG_NAME, .short_name = SHORT_NAME, .desc = DESCRIPTION,          \
    .required = REQUIRED, .type = TYPE, .default_val = DEFAULT,                \
  }

#define REQUIRED_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION)                 \
  ARG_DEF(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, true, (arg_val_t){0})

#define OPTIONAL_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, DEFAULT)        \
  ARG_DEF(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, false, DEFAULT)

#define PARSE_ARGS(argc, argv, defs)                                           \
  ((defs) ? parse_args(argc, argv, defs, sizeof(defs) / sizeof(defs[0])) : NULL)

// This expects you to pass a static string, dynamic strings may cause dangling
// pointers.
#define STRING_VAL(value) ((arg_val_t){.string_val = (value)})
#define INT_VAL(value) ((arg_val_t){.int_val = (value)})
#define FLOAT_VAL(value) ((arg_val_t){.float_val = (value)})
// Usually presence of the flag determines its effects so it will always be
// defaulted as false
#define FLAG_VAL ((arg_val_t){.flag_val = false})

#define GET_DEF(valid_args, name)                                              \
  get_arg_def(valid_args, name, sizeof(valid_args) / sizeof(valid_args[0]))

// The type of the arg (int, float, string or flag)
typedef enum {
  ARG_FLAG,
  ARG_INT,
  ARG_FLOAT,
  ARG_STRING,
} arg_type_t;

// Value provided to the specific argument (for flags if the argument exists its
// counted as true or 1)
typedef union {
  bool flag_val;
  int int_val;
  float float_val;
  char *string_val;
} arg_val_t;

// Representation of and arg, provides necessary metadata
typedef struct {
  const char *name;
  const char short_name;
  const char *desc;
  bool required;
  arg_type_t type;
  arg_val_t default_val;
} arg_def_t;

// State of the argument, contains its value and whether it is found or not.
typedef struct {
  arg_val_t value;
  bool found;
} arg_state_t;

// list of args with their definitions and states.
typedef struct {
  int num_args;
  const arg_def_t *defs;
  arg_state_t *states;
} args_t;

// parses the provided arguments, checking validity and returning a pointer
// to the parsed args. returns NULL on failure
inline static args_t *parse_args(int argc, const char *argv[],
                                 const arg_def_t *args_defs, int num_args) {
// macro for exiting function incase of error
#define SAFE_EXIT                                                              \
  free(states);                                                                \
  free(args);                                                                  \
  return NULL;

  if (!args_defs || num_args <= 0) {
    return NULL;
  }
  args_t *args = (args_t *)malloc(sizeof(args_t));
  arg_state_t *states = (arg_state_t *)calloc(num_args, sizeof(arg_state_t));
  if (args == NULL || states == NULL) {
    SAFE_EXIT;
  }
  args->defs = args_defs;
  args->states = states;
  args->num_args = num_args;

  // main parse loop
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (arg[0] != '-') {
      continue;
    }
    // pointer to the address of whatever argument matched
    const arg_def_t *matched_def = NULL;
    if (arg[1] == '-') {
      for (int j = 0; j < num_args; j++) {
        if (strcmp(args->defs[j].name, arg + 2) == 0) {
          matched_def = &args->defs[j];
          break;
        }
      }
    } else {
      char short_name = arg[1];
      for (int j = 0; j < num_args; j++) {
        if (args->defs[j].short_name == short_name) {
          matched_def = &args->defs[j];
          break;
        }
      }
    }
    if (matched_def == NULL) {
      fprintf(stderr, "Unknown option: %s\n", arg);
      SAFE_EXIT;
    }
    int index = matched_def - args->defs;
    arg_state_t *state = &args->states[index];
    if (state->found) {
      fprintf(stderr, "Error: Duplicate argument found for --%s\n",
              matched_def->name);
      SAFE_EXIT;
    }
    state->found = true;

    switch (matched_def->type) {
    case ARG_FLAG:
      state->value.flag_val = true;
      break;
    case ARG_INT:
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for %s\n", arg);
        SAFE_EXIT;
      }
      state->value.int_val = atoi(argv[++i]);
      break;
    case ARG_FLOAT:
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for %s\n", arg);
        SAFE_EXIT;
      }
      state->value.float_val = atof(argv[++i]);
      break;
    case ARG_STRING:
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for %s\n", arg);
        SAFE_EXIT;
      }
      state->value.string_val = strdup(argv[++i]);
      if (state->value.string_val == NULL) {
        for (int j = 0; j < index; j++) {
          if (args->states[j].value.string_val) {
            free(args->states[j].value.string_val);
          }
        }
        SAFE_EXIT;
      }
      break;
    }
  }
  // loop to check argument requirements
  for (int i = 0; i < num_args; i++) {
    if (!states[i].found) {
      // if required but not found then exit program.
      if (args->defs[i].required) {
        fprintf(stderr, "Missing required argument: --%s (%s)\n",
                args->defs[i].name,
                args->defs[i].short_name ? (char[]){args->defs[i].short_name, 0}
                                         : "none");
        SAFE_EXIT;
        // if not found but is optional then default value is used
      } else {
        states[i].value = args->defs[i].default_val;
      }
    }
  }
  return args;
#undef SAFE_EXIT
}

// Free all arguments after use.
inline static void free_args(args_t *args) {
  if (args == NULL) {
    return;
  }
  if (args->states) {
    // free the strings created from strdup()
    for (int i = 0; i < args->num_args; i++) {
      if (args->defs[i].type == ARG_STRING && args->states[i].found &&
          args->states[i].value.string_val) {
        free(args->states[i].value.string_val);
      }
    }
    free(args->states);
  }
  free(args);
  args = NULL;
}

// Returns a void pointer with the value of the arg, strings are returned as is
// and not a pointer, Returns null if not found
inline static void *get_arg_val(args_t *args, const char *name) {
  if (args == NULL) {
    return NULL;
  }
  for (int i = 0; i < args->num_args; i++) {
    if (strcmp(args->defs[i].name, name) == 0) {
      arg_state_t *state = &args->states[i];
      switch (args->defs[i].type) {
      case ARG_FLAG:
        return &state->value.flag_val;
      case ARG_INT:
        return &state->value.int_val;
      case ARG_FLOAT:
        return &state->value.float_val;
      case ARG_STRING:
        return state->value.string_val;
      }
    }
  }
  return NULL; // not found
}
// Returns null if the arg was not found
inline static const arg_def_t *get_arg_def(const arg_def_t valid_args[],
                                           const char *name, int num_defs) {
  if (valid_args == NULL) {
    return NULL;
  }
  for (int i = 0; i < num_defs; i++) {
    if (strcmp(valid_args[i].name, name) == 0) {
      return &valid_args[i];
    }
  }
  return NULL;
}

#endif