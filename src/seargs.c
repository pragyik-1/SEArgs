
#include "../headers/seargs.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------ */
/* Utility functions. */
/* ------------------ */

static char *str_dup(const char *string) {
  size_t len = strlen(string) + 1;
  char *copy = malloc(len);
  return copy ? memcpy(copy, string, len) : NULL;
}

// tries to convert a str into an int, returns false on failure
static bool str_to_int(const char *str, int *out) {
  char *endptr;
  long result = strtol(str, &endptr, 0);
  if (*endptr != '\0') {
    return false;
  }
  if (errno == ERANGE || result < INT_MIN || result > INT_MAX) {
    return false;
  }
  *out = (int)result;
  return true;
}

// tries to convert a str into a double, returns false on failure
static bool str_to_double(const char *str, double *out) {
  char *endptr;
  double result = strtod(str, &endptr);
  if (*endptr != '\0') {
    return false;
  }
  if (errno == ERANGE) {
    return false;
  }
  *out = result;
  return true;
}

// Internal helper to free the strings allocated by strdup() in parse_args()
// properly. has no use standalone.
static inline void _cleanup_string_states(const arg_def_t *defs,
                                          arg_state_t *states, int limit) {
  if (!states || !defs)
    return;
  for (int i = 0; i < limit; i++) {
    if (defs[i].type == ARG_STRING && states[i].string_allocated) {
      free(states[i].value.string_val);
    }
  }
}

// Internal helper to do a full cleanup of memory even if it wasnt fully
// initialized yet.
static inline void _cleanup_args(args_t *args) {
  if (args == NULL) {
    return;
  }
  if (args->states) {
    _cleanup_string_states(args->defs, args->states, args->num_args);
    free(args->states);
  }
  free(args);
}

static bool validate_arg_defs(const arg_def_t *defs, int num_args) {
  if (!defs || num_args <= 0)
    return false;

  for (int i = 0; i < num_args; i++) {
    if (!defs[i].name) {
      fprintf(stderr, "Argument name cannot be null\n");
      return false;
    }
    for (int j = i + 1; j < num_args; j++) {
      if (strcmp(defs[i].name, defs[j].name) == 0) {
        fprintf(stderr, "Duplicate argument name: %s\n", defs[i].name);
        return false;
      }
      if (defs[i].short_name == defs[j].short_name) {
        fprintf(stderr, "Duplicate short name: %c\n", defs[i].short_name);
        return false;
      }
    }
  }
  return true;
}

static void print_help(const arg_def_t *defs, int num_args) {
  if (!defs || num_args <= 0)
    return;
  printf("Usage:\n");
  for (int i = 0; i < num_args; i++) {
    char *short_name = malloc(4);
    sprintf(short_name, "(%c)", defs[i].short_name);
    printf("%s%s: %s\n", defs[i].name, short_name, defs[i].desc);
  }
}

// parses the provided arguments, checking validity and returning a pointer
// to the parsed args. returns NULL on failure
args_t *parse_args(int argc, const char *argv[], const arg_def_t *args_defs,
                   int num_args) {
// macro for exiting function incase of error
#define SAFE_EXIT                                                              \
  do {                                                                         \
    _cleanup_args(args);                                                       \
    return NULL;                                                               \
  } while (0)

  if (!validate_arg_defs(args_defs, num_args)) {
    return NULL;
  }

  if (argc <= 1) {
    print_help(args_defs, num_args);
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
    if (strcmp(arg, "--") == 0)
      break;
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
      if (!str_to_int(argv[++i], &state->value.int_val)) {
        SAFE_EXIT;
      }
      break;
    case ARG_DOUBLE:
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for %s\n", arg);
        SAFE_EXIT;
      }
      if (!str_to_double(argv[++i], &state->value.double_val)) {
        SAFE_EXIT;
      }
      break;
    case ARG_STRING:
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for %s\n", arg);
        SAFE_EXIT;
      }
      state->value.string_val = str_dup(argv[++i]);
      if (state->value.string_val == NULL) {
        SAFE_EXIT;
      }
      state->string_allocated = true;
      break;
    default:
      break;
    }
  }
  // loop to check argument requirements
  for (int i = 0; i < num_args; i++) {
    arg_state_t *state = &args->states[i];
    const arg_def_t *def = &args->defs[i];
    if (state->found) {
      continue;
    }

    if (def->required) {
      fprintf(stderr, "Missing required argument: --%s (%c)\n", def->name,
              def->short_name);
      SAFE_EXIT;
    }

    if (def->type == ARG_STRING) {
      if (def->default_val.string_val == NULL) {
        fprintf(stderr, "Missing default for optional string arg: --%s\n",
                def->name);
        SAFE_EXIT;
      }
      state->value.string_val = str_dup(def->default_val.string_val);
      if (state->value.string_val == NULL) {
        SAFE_EXIT;
      }
      state->string_allocated = true;
    } else {
      state->value = def->default_val;
      state->string_allocated = false;
    }
  }
  return args;
#undef SAFE_EXIT
}

// Free all arguments after use.

void free_args(args_t **p_args) {
  if (!p_args || !*p_args)
    return;
  _cleanup_args(*p_args);
  *p_args = NULL;
}

// Returns a void pointer with the value of the arg, strings are returned as is
// and not a pointer, Returns null if not found
void *get_arg_val(args_t *args, const char *name) {
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
      case ARG_DOUBLE:
        return &state->value.double_val;
      case ARG_STRING:
        return &state->value.string_val;
      }
    }
  }
  return NULL; // not found
}
// Returns null if the arg was not found
const arg_def_t *get_arg_def(const arg_def_t valid_args[], const char *name,
                             int num_defs) {
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