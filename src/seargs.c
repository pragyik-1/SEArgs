
#include "seargs.h"
#include <errno.h>
#include <float.h>
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
static inline bool str_to_int(const char *str, int *out) {
  errno = 0;
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
static inline bool str_to_double(const char *str, double *out) {
  errno = 0;
  char *endptr;
  double result = strtod(str, &endptr);
  if (*endptr != '\0') {
    return false;
  }
  if (errno == ERANGE || result > DBL_MAX) {
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
static inline void _cleanup_parser(parser_t *parser) {
  if (!parser) {
    return;
  }
  if (parser->states) {
    _cleanup_string_states(parser->defs, parser->states, parser->num_args);
  }
  free(parser);
}

// Helper for parse_args() to properly cleanup on failure
static parser_t *failure(parser_t *parser, const char *msg, const char *arg,
                         seargs_err_codes err_code) {
  if (msg) {
    fprintf(stderr, "%s%s%s\n", msg, arg ? ": " : "", arg ? arg : "");
  }
  parser->error =
      (seargs_error_t){.msg = msg, .arg_name = arg, .code = err_code};
  return NULL;
}

static parser_t *fatal_failure(parser_t *parser, const char *msg) {
  if (msg) {
    fprintf(stderr, "%s\n", msg);
  }
  _cleanup_parser(parser);
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
    for (int i = 0; i < num_args; i++) {
      if (contains_format_specifier(defs[i].name) ||
          contains_format_specifier(defs[i].desc)) {
        fprintf(
            stderr,
            "Argument name/description contains invalid format specifier: %s\n",
            defs[i].name);
        return false;
      }
    }
    for (int j = i + 1; j < num_args; j++) {
      if (!defs[j].name) {
        fprintf(stderr, "Argument name cannot be null\n");
        return false;
      }
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
  int max_name_len = 0;
  for (int i = 0; i < num_args; i++) {
    int current_len = strlen(defs[i].name);
    if (current_len > max_name_len) {
      max_name_len = current_len;
    }
  }
  int total_pad_width = max_name_len + 5;
  printf("Usage:\n");
  for (int i = 0; i < num_args; i++) {
    char short_name_part[6];
    if (defs[i].short_name) {
      snprintf(short_name_part, sizeof(short_name_part), "(-%c)",
               defs[i].short_name);
    } else {
      snprintf(short_name_part, sizeof(short_name_part), "    ");
    }
    printf("  --%s %-*s  %s\n", defs[i].name,
           (int)(total_pad_width - strlen(defs[i].name)), short_name_part,
           defs[i].desc);
  }
}

const arg_def_t *get_matching_arg_def_(const char *name, const arg_def_t *defs,
                                      int num_args, bool is_short_name) {
  if (!defs || num_args <= 0 || !name) {
    return NULL;
  }
  if (is_short_name && strlen(name) > 1) {
    return NULL;
  }
  if (!is_short_name) {
    for (int i = 0; i < num_args; i++) {
      if (strcmp(defs[i].name, name) == 0) {
        return &defs[i];
      }
    }
    return NULL;
  }
  if (is_short_name) {
    for (int i = 0; i < num_args; i++) {
      if (defs[i].short_name == name[0]) {
        return &defs[i];
      }
    }
    return NULL;
  }
  return NULL;
}

// (parser_t *) but only ever returns the parser you passed in or null for
// error. that is return truthy value on success otherwise a falsy value.
parser_t *assign_value(const arg_def_t *def, const char *argv[], int *i,
                       int argc, parser_t *parser, arg_state_t *state) {
  if (!state || !def) {
    return NULL;
  }
  if (*i >= argc) {
    return failure(parser, "Missing value for", def->name, MISSING_VALUE_ERR);
  }
  const char *arg = argv[*i];
  state->found = true;
  switch (def->type) {
  case ARG_FLAG:
    state->value.flag_val = true;
    break;
  case ARG_INT:
    if (*i + 1 >= argc) {
      return failure(parser, "Missing value for", arg, MISSING_VALUE_ERR);
    }
    if (!str_to_int(argv[++*i], &state->value.int_val)) {
      return failure(parser, "Invalid value for (expected an integer)", arg,
                     INVALID_VALUE_ERR);
    }
    break;
  case ARG_DOUBLE:
    if (*i + 1 >= argc) {
      return failure(parser, "Missing value for", arg, MISSING_VALUE_ERR);
    }
    if (!str_to_double(argv[++*i], &state->value.double_val)) {
      return failure(parser, "Invalid value for (expected float or double)",
                     arg, INVALID_VALUE_ERR);
    }
    break;
  case ARG_STRING:
    if (*i + 1 >= argc) {
      return failure(parser, "Missing value for", arg, MISSING_VALUE_ERR);
    }
    state->value.string_val = str_dup(argv[++*i]);
    if (!state->value.string_val) {
      return fatal_failure(parser, "Failed to allocate memory");
    }
    state->string_allocated = true;
    break;
  default:
    break;
  }
  return parser;
}

// parses the provided arguments, checking validity and returning a pointer
// to the parsed args. returns NULL on failure
parser_t *parse_args(int argc, const char *argv[], const arg_def_t *args_defs,
                     size_t num_args) {
  if (!validate_arg_defs(args_defs, num_args)) {
    return NULL;
  }

  if (argc <= 1) {
    print_help(args_defs, num_args);
    return NULL;
  }

  // settings size of args as parser_t and states as num_args * arg_state_t in
  // same variable for allowing pointer arithmetic
  parser_t *parser =
      calloc(1, sizeof(parser_t) + num_args * sizeof(arg_state_t));
  if (!parser) {
    return fatal_failure(parser, "Failed to allocate memory");
  }
  parser->defs = args_defs;
  parser->states =
      (arg_state_t *)(parser + 1); // args + 1 moves memory of args by parser_t
                                   // meaning the remainder is for state
  parser->num_args = num_args;

  // main parse loop
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--") == 0) {
      break;
    }
    if (arg[0] != '-') {
      continue;
    }
    if (arg[1] == '-') {
      const arg_def_t *def =
          get_matching_arg_def_(arg + 2, args_defs, num_args, false);
      if (!def) {
        return failure(parser, "Unknown argument", arg, UNKNOWN_ARG_ERR);
      }
      int def_index = def - args_defs;
      arg_state_t *state = &parser->states[def_index];
      if (!assign_value(def, argv, &i, argc, parser, state)) {
        return NULL;
      };
    } else {
      const char *arg_cluster = arg + 1;
      if (arg_cluster[0] == '\0') {
        return failure(parser, "Invalid short argument", arg, INVALID_ARG_ERR);
      }
      for (int j = 0; arg_cluster[j] != '\0'; j++) {
        char short_char[2] = {arg_cluster[j], '\0'};
        bool is_last = arg_cluster[j + 1] == '\0';
        const arg_def_t *def =
            get_matching_arg_def_(short_char, args_defs, num_args, true);
        if (!def) {
          return failure(parser, "Unknown argument", arg, UNKNOWN_ARG_ERR);
        }
        if (def->type != ARG_FLAG && !is_last) {
          return failure(parser, "Non-flag argument must be last in cluster",
                         arg, INVALID_ARG_ERR);
        }
        int def_index = def - args_defs;
        arg_state_t *state = &parser->states[def_index];
        if (!assign_value(def, argv, &i, argc, parser, state)) {
          return NULL;
        }
        if (def->type != ARG_FLAG) {
          break;
        }
      }
    }
    parser->pos_args = (i < argc) ? &argv[i] : NULL;
    parser->num_pos_args = (i < argc) ? argc - i : 0;
  }
  // loop to check argument requirements
  for (int i = 0; i < num_args; i++) {
    arg_state_t *state = &parser->states[i];
    const arg_def_t *def = &parser->defs[i];
    if (state->found) {
      continue;
    }

    if (def->required) {
      return failure(parser, "Missing required argument", def->name,
                     MISSING_ARG_ERR);
    }

    if (def->type == ARG_STRING && def->default_val.string_val) {
      state->value.string_val = str_dup(def->default_val.string_val);
      if (!state->value.string_val) {
        return fatal_failure(parser, "Failed to allocate memory");
      }
      state->string_allocated = true;
    } else {
      state->value = def->default_val;
      state->string_allocated = false;
    }
  }
  return parser;
}

// Free all arguments after use.

void free_parser(parser_t **p_parser) {
  if (!p_parser || !*p_parser)
    return;
  _cleanup_parser(*p_parser);
  *p_parser = NULL;
}

void *get_arg_val(parser_t *parser, const char *name) {
  if (!parser) {
    return NULL;
  }
  for (int i = 0; i < parser->num_args; i++) {
    if (strcmp(parser->defs[i].name, name) == 0) {
      arg_state_t *state = &parser->states[i];
      switch (parser->defs[i].type) {
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
    if (strlen(name) == 2 && valid_args[i].short_name == name[0]) {
      return &valid_args[i];
    }
  }
  return NULL;
}