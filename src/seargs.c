
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
// properly
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
  if (!args) {
    return;
  }
  if (args->states) {
    _cleanup_string_states(args->defs, args->states, args->num_args);
  }
  free(args);
}

// Helper for parse_args() to properly cleanup on failure
static inline args_t *failure(args_t *args, const char *msg, const char *arg) {
  if (msg) {
    fprintf(stderr, "Error:  %s%s%s\n", msg, arg ? ": " : "", arg ? arg : "");
  }
  _cleanup_args(args);
  return NULL;
}

bool validate_arg_defs(const arg_def_t *defs, int num_args) {
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

void print_help(const arg_def_t *defs, int num_args) {
  if (!defs || num_args <= 0)
    return;
  printf("Usage:\n");
  for (int i = 0; i < num_args; i++) {
    char short_name[4];
    sprintf(short_name, "(%c)", defs[i].short_name ? defs[i].short_name : ' ');
    printf("%s%s: %s\n", defs[i].name, short_name, defs[i].desc);
  }
}

const arg_def_t *get_matching_arg_def(const char *name, const arg_def_t *defs,
                                      int num_args) {
  if (!defs || num_args <= 0) {
    return NULL;
  }

  for (int i = 0; i < num_args; i++) {
    if (strcmp(defs[i].name, name) == 0) {
      return &defs[i];
    }
    if (strlen(name) == 1 && defs[i].short_name == name[0]) {
      return &defs[i];
    }
  }
  return NULL;
}

// parses the provided arguments, checking validity and returning a pointer
// to the parsed args. returns NULL on failure
args_t *parse_args(int argc, const char *argv[], const arg_def_t *args_defs,
                   int num_args) {
  if (!validate_arg_defs(args_defs, num_args)) {
    return NULL;
  }

  if (argc <= 1) {
    print_help(args_defs, num_args);
    return NULL;
  }

  // settings size of args as args_t and states as num_args * arg_state_t in
  // same variable for allowing pointer arithmetic
  args_t *args = malloc(sizeof(args_t) + num_args * sizeof(arg_state_t));
  if (!args) {
    return failure(args, "Failed to allocate memory", NULL);
  }
  args->defs = args_defs;
  args->states =
      (arg_state_t *)(args + 1); // args + 1 moves memory of args by args_t
                                 // meaning the remainder is for state
  args->num_args = num_args;

  // main parse loop
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--") == 0) {
      break;
    }
    if (arg[0] != '-') {
      continue;
    }
    const arg_def_t *matched_defs[16];
    int num_matched = 0;
    if (arg[1] == '-') {
      const arg_def_t *def = get_matching_arg_def(arg + 2, args_defs, num_args);
      if (!def) {
        return failure(args, "Unknown argument", arg);
      }
      matched_defs[num_matched++] = def;
    } else {
      const char *short_args = arg + 1;
      for (int k = 0; short_args[k] != '\0'; k++) {
        const arg_def_t *def =
            get_matching_arg_def(short_args + k, args_defs, num_args);
        if (!def) {
          return failure(args, "Unknown argument", arg);
        }
        matched_defs[num_matched++] = def;
      }
    }

    if (num_matched == 0) {
      return failure(args, "Unknown argument", arg);
    }

    for (int j = 0; j < num_matched; j++) {
      const arg_def_t *matched_def = matched_defs[j];
      int index = matched_def - args->defs;
      arg_state_t *state = &args->states[index];
      if (state->found) {
        return failure(args, "Duplicate argument found", arg);
      }
      state->found = true;
      switch (matched_def->type) {
      case ARG_FLAG:
        state->value.flag_val = true;
        break;
      case ARG_INT:
        if (i + 1 >= argc) {
          return failure(args, "Missing value for", arg);
        }
        if (!str_to_int(argv[++i], &state->value.int_val)) {
          return failure(args, "Invalid value for", arg);
        }
        break;
      case ARG_DOUBLE:
        if (i + 1 >= argc) {
          return failure(args, "Missing value for", arg);
        }
        if (!str_to_double(argv[++i], &state->value.double_val)) {
          return failure(args, "Invalid value for", arg);
        }
        break;
      case ARG_STRING:
        if (i + 1 >= argc) {
          return failure(args, "Missing value for", arg);
        }
        state->value.string_val = str_dup(argv[++i]);
        if (!state->value.string_val) {
          return failure(args, "Failed to allocate memory", NULL);
        }
        state->string_allocated = true;
        break;
      default:
        break;
      }
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
      return failure(args, "Missing required argument", def->name);
    }

    if (def->type == ARG_STRING && def->default_val.string_val) {
      state->value.string_val = str_dup(def->default_val.string_val);
      if (!state->value.string_val) {
        return failure(args, "Failed to allocate memory", NULL);
      }
      state->string_allocated = true;
    } else {
      state->value = def->default_val;
      state->string_allocated = false;
    }
  }
  return args;
}

// Free all arguments after use.

void free_args(args_t **p_args) {
  if (!p_args || !*p_args)
    return;
  _cleanup_args(*p_args);
  *p_args = NULL;
}

void *get_arg_val(args_t *args, const char *name) {
  if (!args) {
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
  if (!valid_args) {
    return NULL;
  }
  for (int i = 0; i < num_defs; i++) {
    if (strcmp(valid_args[i].name, name) == 0) {
      return &valid_args[i];
    }
  }
  return NULL;
}