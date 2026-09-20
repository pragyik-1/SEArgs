#ifndef SEARGS_H
#define SEARGS_H

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// ===== Public API =====

#define SEARGS_NO_SHORT_NAME '\0'
#define ARG_NAME_MAX_SIZE 64
#define ARG_VAL_MAX_SIZE 256

// Definition flags. SEARGS_FLAG_POSITIONAL marks an argument that is matched by
// position on the command line rather than by name; it is never usable as an
// option and may not declare a short name. The remaining flags are reserved for
// future features and are accepted but have no effect yet.
#define SEARGS_FLAG_POSITIONAL (1u << 0)
#define SEARGS_FLAG_CASE_INSENSITIVE (1u << 1)
#define SEARGS_FLAG_REPEATABLE (1u << 2)
#define SEARGS_FLAG_MASK                                                                           \
  (SEARGS_FLAG_POSITIONAL | SEARGS_FLAG_CASE_INSENSITIVE | SEARGS_FLAG_REPEATABLE)

typedef enum {
  SEARGS_TYPE_BOOL,
  SEARGS_TYPE_INT,
  SEARGS_TYPE_DOUBLE,
  SEARGS_TYPE_STRING,
} seargs_arg_type_t;

typedef enum {
  SEARGS_OK = 0,
  // Parse-time errors
  SEARGS_MISSING_ARG,   // a required argument was not provided
  SEARGS_INVALID_VALUE, // a value could not be converted to the argument's type
  SEARGS_MISSING_VALUE, // an argument that requires a value got none
  SEARGS_UNKNOWN_ARG,   // the option is not defined
  SEARGS_TOO_MANY_ARGS, // a token arrived after every positional slot was filled

  // Definition validation errors
  SEARGS_DEF_TABLE_NULL,
  SEARGS_DEF_NAME_NULL,
  SEARGS_DEF_NAME_EMPTY,
  SEARGS_DEF_NAME_INVALID, // leading '-' or contains ' '/'\t'/'='/'\n'/'\r'
  SEARGS_DEF_SHORT_NAME_INVALID,
  SEARGS_DEF_DUPLICATE_NAME,
  SEARGS_DEF_DUPLICATE_SHORT_NAME,
  SEARGS_DEF_INVALID_TYPE,
  SEARGS_DEF_INVALID_FLAGS,      // unknown bits set in def->flags
  SEARGS_DEF_POSITIONAL_INVALID, // a positional argument declared a short name
} seargs_error_t;

typedef union {
  bool bool_val;
  int int_val;
  const char *str_val;
  double double_val;
} seargs_arg_val_t;

typedef struct {
  const char *name;
  char short_name;  // '\0' = none
  const char *desc; // may be NULL
  bool is_required;
  seargs_arg_type_t type;
  seargs_arg_val_t default_val;
  uint32_t flags; // 0 or a combination of SEARGS_FLAG_*
} seargs_arg_def_t;

// Convenience initializers for seargs_arg_def_t; each expands to a braced
// initializer usable where a def belongs, including inside a defs[] array.
// Every optional carries an explicit default (absent never yields a silent
// NULL / 0). REQUIRED_<T>_ARG(name, short, desc) is required; OPTIONAL_<T>_ARG
// (name, short, desc, default) is optional with default. POSITIONAL_*_ARG
// match by position, take no short name and are never usable as options. BOOL
// has no positional form. desc may be NULL; pass SEARGS_NO_SHORT_NAME for none.

#define SEARGS_REQUIRED_BOOL_ARG(name_, short_, desc_)                                             \
  {.name = (name_),                                                                                \
   .short_name = (short_),                                                                         \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_BOOL}

#define SEARGS_OPTIONAL_BOOL_ARG(name_, short_, desc_, default_)                                   \
  {                                                                                                \
    .name = (name_), .short_name = (short_), .desc = (desc_), .type = SEARGS_TYPE_BOOL,            \
    .default_val = {                                                                               \
      .bool_val = (default_)                                                                       \
    }                                                                                              \
  }

#define SEARGS_REQUIRED_STRING_ARG(name_, short_, desc_)                                           \
  {.name = (name_),                                                                                \
   .short_name = (short_),                                                                         \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_STRING}

#define SEARGS_OPTIONAL_STRING_ARG(name_, short_, desc_, default_)                                 \
  {                                                                                                \
    .name = (name_), .short_name = (short_), .desc = (desc_), .type = SEARGS_TYPE_STRING,          \
    .default_val = {                                                                               \
      .str_val = (default_)                                                                        \
    }                                                                                              \
  }

#define SEARGS_POSITIONAL_REQUIRED_STRING_ARG(name_, desc_)                                        \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_STRING,                                                                     \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_POSITIONAL_OPTIONAL_STRING_ARG(name_, desc_, default_)                              \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_STRING,                                                                     \
   .default_val = {.str_val = (default_)},                                                         \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_REQUIRED_INT_ARG(name_, short_, desc_)                                              \
  {.name = (name_),                                                                                \
   .short_name = (short_),                                                                         \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_INT}

#define SEARGS_OPTIONAL_INT_ARG(name_, short_, desc_, default_)                                    \
  {                                                                                                \
    .name = (name_), .short_name = (short_), .desc = (desc_), .type = SEARGS_TYPE_INT,             \
    .default_val = {                                                                               \
      .int_val = (default_)                                                                        \
    }                                                                                              \
  }

#define SEARGS_POSITIONAL_REQUIRED_INT_ARG(name_, desc_)                                           \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_INT,                                                                        \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_POSITIONAL_OPTIONAL_INT_ARG(name_, desc_, default_)                                 \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_INT,                                                                        \
   .default_val = {.int_val = (default_)},                                                         \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_REQUIRED_DOUBLE_ARG(name_, short_, desc_)                                           \
  {.name = (name_),                                                                                \
   .short_name = (short_),                                                                         \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_DOUBLE}

#define SEARGS_OPTIONAL_DOUBLE_ARG(name_, short_, desc_, default_)                                 \
  {                                                                                                \
    .name = (name_), .short_name = (short_), .desc = (desc_), .type = SEARGS_TYPE_DOUBLE,          \
    .default_val = {                                                                               \
      .double_val = (default_)                                                                     \
    }                                                                                              \
  }

#define SEARGS_POSITIONAL_REQUIRED_DOUBLE_ARG(name_, desc_)                                        \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .is_required = true,                                                                            \
   .type = SEARGS_TYPE_DOUBLE,                                                                     \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_POSITIONAL_OPTIONAL_DOUBLE_ARG(name_, desc_, default_)                              \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_DOUBLE,                                                                     \
   .default_val = {.double_val = (default_)},                                                      \
   .flags = SEARGS_FLAG_POSITIONAL}

#define IS_SEARGS_OK(p_) (p_->error.code == SEARGS_OK)

typedef struct {
  const seargs_arg_def_t *def; // back-pointer; non-NULL after a successful parse
  size_t def_index;            // index of def within arg_defs
  seargs_arg_val_t val;
  bool is_found; // true when the argument was seen, even when set to false
} seargs_arg_state_t;

typedef struct {
  seargs_error_t code;
  size_t index;       // def index that caused the error; SIZE_MAX when not applicable
  const char *arg;    // argv token that caused the error; NULL when not applicable
  const char *detail; // optional static message; may be NULL
} seargs_error_info_t;

typedef struct {
  size_t n_arg_defs;
  const seargs_arg_def_t *arg_defs;
  seargs_arg_state_t *states;
  seargs_error_info_t error;
} seargs_parser_t;

// Parse argv against defs.
//   - On allocation failure returns NULL.
//   - On a definition or parse error returns a parser whose error.code != SEARGS_OK;
//     it must still be released with seargs_free.
//   - On success returns a parser with error.code == SEARGS_OK.
seargs_parser_t *seargs_parse_args(int argc, char *const argv[], const seargs_arg_def_t *arg_defs,
                                   size_t n_arg_defs);

// Frees the parser and everything it owns. Safe to call on any result of
// seargs_parse_args, including NULL (no-op).
void seargs_free(seargs_parser_t *parser);

// State lookup. Returns NULL when name is not a defined argument.
const seargs_arg_state_t *seargs_get_state(const seargs_parser_t *p, const char *name);

// Returns true if the argument was provided on the command line.
bool seargs_is_found(const seargs_parser_t *p, const char *name);

void print_help(const seargs_arg_def_t *defs, int num_args);

// Typed accessors. They assert (in debug builds) that the name exists and that the
// argument's type matches; in release builds they return the default, 0 or NULL on
// mismatch. They return the parsed value, or the definition's default when the
// argument was not provided. Only meaningful when the defs validated
// (parser->error.code == SEARGS_OK).
bool seargs_get_bool(const seargs_parser_t *p, const char *name);
int seargs_get_int(const seargs_parser_t *p, const char *name);
double seargs_get_double(const seargs_parser_t *p, const char *name);
const char *seargs_get_string(const seargs_parser_t *p, const char *name);

#endif // SEARGS_H

#ifdef SEARGS_IMPLEMENTATION

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static bool str_to_int(const char *str, int *out) {
  if (!str || !out)
    return false;
  errno = 0;
  char *endptr = NULL;
  long result = strtol(str, &endptr, 10);

  if (endptr == str)
    return false;
  if (*endptr != '\0')
    return false;
  if (errno == ERANGE || result < INT_MIN || result > INT_MAX)
    return false;

  *out = (int)result;
  return true;
}

static bool str_to_double(const char *str, double *out) {
  if (!str || !out)
    return false;
  errno = 0;
  char *endptr = NULL;
  double result = strtod(str, &endptr);

  if (endptr == str)
    return false;
  if (*endptr != '\0')
    return false;
  // Reject only overflow; underflow of a tiny-but-valid value (e.g. "1e-400")
  // is accepted. "inf"/"nan" are accepted as well.
  if (errno == ERANGE && (result > DBL_MAX || result < -DBL_MAX))
    return false;

  *out = result;
  return true;
}

static bool str_ieq(const char *a, const char *b) {
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
      return false;
    a++;
    b++;
  }
  return *a == *b;
}

static bool str_to_bool(const char *s, bool *out) {
  if (!s || !out)
    return false;

  if (str_ieq(s, "true") || strcmp(s, "1") == 0) {
    *out = true;
    return true;
  }

  if (str_ieq(s, "false") || strcmp(s, "0") == 0) {
    *out = false;
    return true;
  }

  return false;
}

// Single error funnel. Semantics: index is a **def index** (SIZE_MAX when not
// applicable); arg is the **argv token** that caused the error (NULL when none).
// The first error reported wins.
static void seargs_report_err(seargs_error_info_t *err, seargs_error_t code, size_t def_index,
                              const char *argv_token, const char *detail) {
  if (!err || err->code != SEARGS_OK)
    return;

  err->code = code;
  err->index = def_index;
  err->arg = argv_token;
  err->detail = detail;
}

static inline bool is_valid_name_char(char c) {
  return c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '=';
}

static inline bool is_valid_short_name(char c) {
  unsigned char u = (unsigned char)c;
  return c != '-' && c != '=' && c != ' ' && u >= 0x20 && u != 0x7f;
}

static inline bool is_long_arg(const char *arg) { return arg[0] == '-' && arg[1] == '-'; }

// A token that looks like an option. A lone '-' is deliberately NOT an option
// token; it is conventionally a positional ("stdin") marker.
static inline bool is_option_token(const char *t) { return t[0] == '-' && t[1] != '\0'; }

static size_t seargs_find_def_index(const seargs_parser_t *p, const char *name) {
  if (!p || !name)
    return SIZE_MAX;

  for (size_t i = 0; i < p->n_arg_defs; i++) {
    if (strcmp(p->arg_defs[i].name, name) == 0)
      return i;
  }
  return SIZE_MAX;
}

// Like seargs_find_def_index, but skips positional arguments: their names are
// matched by position and cannot be used as options.
static size_t seargs_find_option_def_index(const seargs_parser_t *p, const char *name) {
  if (!p || !name)
    return SIZE_MAX;

  for (size_t i = 0; i < p->n_arg_defs; i++) {
    if (!(p->arg_defs[i].flags & SEARGS_FLAG_POSITIONAL) && strcmp(p->arg_defs[i].name, name) == 0)
      return i;
  }
  return SIZE_MAX;
}

static size_t seargs_find_def_index_s(const seargs_parser_t *p, char short_name) {
  if (!p || short_name == SEARGS_NO_SHORT_NAME)
    return SIZE_MAX;

  for (size_t i = 0; i < p->n_arg_defs; i++) {
    if (p->arg_defs[i].short_name == short_name)
      return i;
  }
  return SIZE_MAX;
}

static void validate_defs(const seargs_arg_def_t *defs, size_t n, seargs_error_info_t *p_err) {

  if (!p_err || p_err->code != SEARGS_OK)
    return;

  if (!defs && n > 0) {
    seargs_report_err(p_err, SEARGS_DEF_TABLE_NULL, SIZE_MAX, NULL, "arg_defs is NULL but n > 0");
    return;
  }

  for (size_t i = 0; i < n; i++) {
    const seargs_arg_def_t *d = &defs[i];
    const char sc = d->short_name;

    // --- Long name ---
    if (!d->name) {
      seargs_report_err(p_err, SEARGS_DEF_NAME_NULL, i, NULL, "arg name is NULL");
      return;
    }

    if (d->name[0] == '\0') {
      seargs_report_err(p_err, SEARGS_DEF_NAME_EMPTY, i, NULL, "arg name is empty");
      return;
    }

    if (d->name[0] == '-') {
      seargs_report_err(p_err, SEARGS_DEF_NAME_INVALID, i, NULL,
                        "arg name must not start with '-'");
      return;
    }

    for (const char *p = d->name; *p; p++)
      if (!is_valid_name_char(*p)) {
        seargs_report_err(p_err, SEARGS_DEF_NAME_INVALID, i, NULL,
                          "arg name contains an invalid character");
        return;
      }

    // --- Short name ---
    if (sc != SEARGS_NO_SHORT_NAME && !is_valid_short_name(sc)) {
      seargs_report_err(p_err, SEARGS_DEF_SHORT_NAME_INVALID, i, NULL,
                        "short name is not a printable, non-reserved char");
      return;
    }

    // --- Type ---
    switch (d->type) {
    case SEARGS_TYPE_BOOL:
    case SEARGS_TYPE_INT:
    case SEARGS_TYPE_DOUBLE:
    case SEARGS_TYPE_STRING:
      break;
    default:
      seargs_report_err(p_err, SEARGS_DEF_INVALID_TYPE, i, NULL,
                        "arg type is not a valid enum value");
      return;
    }

    // --- Flags ---
    if (d->flags & ~SEARGS_FLAG_MASK) {
      seargs_report_err(p_err, SEARGS_DEF_INVALID_FLAGS, i, NULL, "arg flags contain unknown bits");
      return;
    }

    // --- Positional arguments ---
    if ((d->flags & SEARGS_FLAG_POSITIONAL) && sc != SEARGS_NO_SHORT_NAME) {
      seargs_report_err(p_err, SEARGS_DEF_POSITIONAL_INVALID, i, NULL,
                        "a positional argument cannot declare a short name");
      return;
    }

    // --- Duplicates ---
    for (size_t j = 0; j < i; j++) {
      if (strcmp(d->name, defs[j].name) == 0) {
        seargs_report_err(p_err, SEARGS_DEF_DUPLICATE_NAME, i, NULL, "duplicate long arg name");
        return;
      }

      if (sc != SEARGS_NO_SHORT_NAME && defs[j].short_name == sc) {
        seargs_report_err(p_err, SEARGS_DEF_DUPLICATE_SHORT_NAME, i, NULL,
                          "duplicate short arg name");
        return;
      }
    }
  }
}

static bool validate_and_default_state(seargs_arg_state_t *state, seargs_error_info_t *p_err) {
  if (!state || !p_err)
    return false;

  const seargs_arg_def_t *def = state->def;
  if (!state->is_found) {
    if (def->is_required) {
      seargs_report_err(p_err, SEARGS_MISSING_ARG, state->def_index, NULL,
                        "Missing required argument");
      return false;
    }

    state->val = def->default_val;
    return true;
  }
  return true;
}

// Fetch the value that follows the option at argv[*p_i + 1]. Does not advance
// *p_i; callers consume the value token themselves. Returns NULL with the error
// set when no usable value is present. See the header docs for the dash-prefix
// policy.
static const char *option_next_value(char *const argv[], int argc, int *p_i,
                                     const seargs_arg_def_t *def, size_t def_index,
                                     const char *opt_token, seargs_error_info_t *p_err) {
  if (*p_i + 1 >= argc || argv[*p_i + 1] == NULL) {
    seargs_report_err(p_err, SEARGS_MISSING_VALUE, def_index, opt_token,
                      "Value not provided for the given argument");
    return NULL;
  }

  const char *next = argv[*p_i + 1];

  // The "--" terminator can never serve as a value.
  if (strcmp(next, "--") == 0) {
    seargs_report_err(p_err, SEARGS_MISSING_VALUE, def_index, opt_token,
                      "Value not provided for the given argument");
    return NULL;
  }

  // A value shaped like another option is only allowed when it parses as a number
  // (negative values); everything else must be attached via '='.
  if (is_option_token(next)) {
    bool numeric = false;
    if (def->type == SEARGS_TYPE_INT) {
      int tmp;
      numeric = str_to_int(next, &tmp);
    } else if (def->type == SEARGS_TYPE_DOUBLE) {
      double tmp;
      numeric = str_to_double(next, &tmp);
    }

    if (!numeric) {
      seargs_report_err(p_err, SEARGS_INVALID_VALUE, def_index, next,
                        "value looks like an option; attach values that start with "
                        "'-' via '=', e.g. --name=-value");
      return NULL;
    }
  }

  return next;
}

static bool assign_value(seargs_parser_t *parser, seargs_arg_state_t *state, size_t def_index,
                         const char *opt_token, const char *val) {
  assert(parser);
  assert(state && state->def);
  assert(val);

  const size_t val_len = strlen(val);
  if (val_len >= ARG_VAL_MAX_SIZE) {
    seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                      "argument value exceeds maximum size; override "
                      "ARG_VAL_MAX_SIZE to raise the limit");
    return false;
  }

  switch (state->def->type) {
  case SEARGS_TYPE_BOOL:
    if (!str_to_bool(val, &state->val.bool_val)) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                        "Given argument's value is not a boolean");
      return false;
    }
    break;
  case SEARGS_TYPE_INT:
    if (!str_to_int(val, &state->val.int_val)) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                        "Given argument's value is not an integer");
      return false;
    }
    break;
  case SEARGS_TYPE_DOUBLE:
    if (!str_to_double(val, &state->val.double_val)) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                        "Given argument's value is not a double");
      return false;
    }
    break;
  case SEARGS_TYPE_STRING:
    // Aliases the caller's argv string; see header docs.
    state->val.str_val = val;
    break;
  default:
    seargs_report_err(&parser->error, SEARGS_DEF_INVALID_TYPE, def_index, opt_token,
                      "Given argument type not part of enum");
    return false;
  }

  state->is_found = true;
  return true;
}

// True when at least one def is declared positional.
static bool parser_has_positionals(const seargs_parser_t *p) {
  for (size_t i = 0; i < p->n_arg_defs; i++)
    if (p->arg_defs[i].flags & SEARGS_FLAG_POSITIONAL)
      return true;
  return false;
}

// Gives token to the next positional def that has no value yet, scanning from
// *cursor. Exactly one slot is filled per call. Reports SEARGS_TOO_MANY_ARGS
// when every positional def is already filled.
static bool parse_positional(seargs_parser_t *parser, size_t *cursor, const char *token) {
  size_t idx = *cursor;

  while (idx < parser->n_arg_defs && !(parser->arg_defs[idx].flags & SEARGS_FLAG_POSITIONAL))
    idx++;

  if (idx == parser->n_arg_defs) {
    seargs_report_err(&parser->error, SEARGS_TOO_MANY_ARGS, SIZE_MAX, token,
                      "too many positional arguments");
    return false;
  }

  *cursor = idx + 1;
  return assign_value(parser, &parser->states[idx], idx, token, token);
}

static bool parse_long_arg(char *const argv[], int argc, seargs_parser_t *parser, int *p_i) {
  const char *arg = argv[*p_i];
  const char *opt_name = arg + 2;
  char name[ARG_NAME_MAX_SIZE];

  const char *eq = strchr(opt_name, '=');
  const size_t name_len = eq ? (size_t)(eq - opt_name) : strlen(opt_name);

  if (name_len >= ARG_NAME_MAX_SIZE) {
    seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, SIZE_MAX, arg,
                      "argument name exceeds maximum size; override "
                      "ARG_NAME_MAX_SIZE to raise the limit");
    return false;
  }

  memcpy(name, opt_name, name_len);
  name[name_len] = '\0';

  const size_t def_index = seargs_find_option_def_index(parser, name);
  if (def_index == SIZE_MAX) {
    seargs_report_err(&parser->error, SEARGS_UNKNOWN_ARG, SIZE_MAX, arg, "No such argument exists");
    return false;
  }

  seargs_arg_state_t *state = &parser->states[def_index];
  const seargs_arg_def_t *def = &parser->arg_defs[def_index];

  const char *val;
  int consumed;

  if (eq) {
    val = eq + 1;
    consumed = 1;
  } else if (def->type == SEARGS_TYPE_BOOL) {
    val = "1";
    consumed = 1;
  } else {
    val = option_next_value(argv, argc, p_i, def, def_index, arg, &parser->error);
    if (!val)
      return false; // error already set by option_next_value
    consumed = 2;
  }

  if (!assign_value(parser, state, def_index, arg, val))
    return false; // error already set by assign_value

  *p_i += consumed - 1;
  return true;
}

static bool parse_short_arg(char *const argv[], int argc, seargs_parser_t *parser, int *p_i) {
  const char *cluster = argv[*p_i] + 1;
  const char *opt_token = argv[*p_i];

  if (cluster[0] == '\0') {
    seargs_report_err(&parser->error, SEARGS_UNKNOWN_ARG, SIZE_MAX, opt_token,
                      "lone '-' is not a valid option");
    return false;
  }

  for (size_t j = 0; cluster[j] != '\0'; j++) {
    const size_t def_index = seargs_find_def_index_s(parser, cluster[j]);
    if (def_index == SIZE_MAX) {
      seargs_report_err(&parser->error, SEARGS_UNKNOWN_ARG, SIZE_MAX, opt_token,
                        "No such argument exists");
      return false;
    }

    const bool is_last = cluster[j + 1] == '\0';
    const seargs_arg_def_t *def = &parser->arg_defs[def_index];
    seargs_arg_state_t *state = &parser->states[def_index];

    if (def->type == SEARGS_TYPE_BOOL) {
      if (!assign_value(parser, state, def_index, opt_token, "1"))
        return false; // error already set by assign_value
      continue;
    }

    if (!is_last) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, opt_token,
                        "Non-flag argument must be last in cluster.");
      return false;
    }

    const char *val = option_next_value(argv, argc, p_i, def, def_index, opt_token, &parser->error);
    if (!val)
      return false; // error already set by option_next_value

    ++*p_i; // consume the value token; the outer loop advances past it as well
    if (!assign_value(parser, state, def_index, opt_token, val))
      return false; // error already set by assign_value
  }
  return true;
}

seargs_parser_t *seargs_parse_args(int argc, char *const argv[], const seargs_arg_def_t *arg_defs,
                                   size_t n_arg_defs) {
  // Allocate the parser and all its argument states in one block.
  seargs_parser_t *parser =
      calloc(1, sizeof(seargs_parser_t) + n_arg_defs * sizeof(seargs_arg_state_t));

  if (!parser)
    return NULL;

  parser->arg_defs = arg_defs;
  parser->n_arg_defs = n_arg_defs;
  parser->states = (seargs_arg_state_t *)(parser + 1);

  parser->error.code = SEARGS_OK;
  parser->error.index = SIZE_MAX;
  parser->error.arg = NULL;
  parser->error.detail = NULL;

  if (argc == 1) {
    print_help(parser->arg_defs, parser->n_arg_defs);
  }

  // Wire states before validation so accessors never dereference a NULL def
  // pointer. A NULL arg_defs table (with n_arg_defs > 0) is caught by validation
  // below and leaves the states unwired.
  if (arg_defs) {
    for (size_t i = 0; i < n_arg_defs; i++) {
      parser->states[i].def = &arg_defs[i];
      parser->states[i].def_index = i;
    }
  }

  validate_defs(parser->arg_defs, parser->n_arg_defs, &parser->error);
  if (parser->error.code != SEARGS_OK)
    return parser; // error is already set in this path.

  // Main parse loop
  const bool has_positionals = parser_has_positionals(parser);
  bool after_terminator = false;
  size_t pos_cursor = 0; // next positional def to fill

  for (int i = 1; i < argc; i++) {

    const char *arg = argv[i];

    // `--` ends option parsing; the tokens after it are positional.
    if (!after_terminator && strcmp(arg, "--") == 0) {
      after_terminator = true;
      continue;
    }

    // After `--` every token is positional, even option-shaped ones. With no
    // positional arguments declared, they are ignored.
    if (after_terminator) {
      if (has_positionals && !parse_positional(parser, &pos_cursor, arg))
        return parser; // error already set
      continue;
    }

    // A lone '-' is not option-shaped: it fills a positional slot when one is
    // open (the conventional "stdin" marker) and is an error otherwise.
    if (strcmp(arg, "-") == 0) {
      if (has_positionals) {
        if (!parse_positional(parser, &pos_cursor, arg))
          return parser; // error already set
      } else {
        seargs_report_err(&parser->error, SEARGS_UNKNOWN_ARG, SIZE_MAX, arg,
                          "lone '-' is not a valid option");
        return parser; // error already set
      }
      continue;
    }

    if (is_long_arg(arg)) {
      if (!parse_long_arg(argv, argc, parser, &i))
        return parser; // error already set
    } else if (arg[0] == '-') {
      if (!parse_short_arg(argv, argc, parser, &i))
        return parser; // error already set
    } else if (has_positionals) {
      if (!parse_positional(parser, &pos_cursor, arg))
        return parser; // error already set
    }
    // Without positional arguments, bare tokens are ignored.
  }

  for (size_t i = 0; i < n_arg_defs; i++) {
    seargs_arg_state_t *state = &parser->states[i];
    if (!validate_and_default_state(state, &parser->error))
      return parser; // error already set
  }

  return parser;
}

void print_help(const seargs_arg_def_t *defs, int n_args) {
  if (!defs || n_args <= 0)
    return;
  int max_name_len = 0;
  for (int i = 0; i < n_args; i++) {
    int current_len = strlen(defs[i].name);
    if (current_len > max_name_len) {
      max_name_len = current_len;
    }
  }
  int total_pad_width = max_name_len + 5;
  printf("Usage:\n");
  for (int i = 0; i < n_args; i++) {
    char short_name_part[6];
    if (defs[i].short_name) {
      snprintf(short_name_part, sizeof(short_name_part), "(-%c)", defs[i].short_name);
    } else {
      snprintf(short_name_part, sizeof(short_name_part), "    ");
    }
    printf("  --%s %-*s  %s\n", defs[i].name, (int)(total_pad_width - strlen(defs[i].name)),
           short_name_part, defs[i].desc);
  }
}

const seargs_arg_state_t *seargs_get_state(const seargs_parser_t *p, const char *name) {
  const size_t def_index = seargs_find_def_index(p, name);
  if (def_index == SIZE_MAX)
    return NULL;
  return &p->states[def_index];
}

bool seargs_is_found(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  return state ? state->is_found : false;
}

bool seargs_get_bool(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_BOOL);
  if (!state)
    return false;
  return state->is_found ? state->val.bool_val : state->def->default_val.bool_val;
}

int seargs_get_int(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_INT);
  if (!state)
    return 0;
  return state->is_found ? state->val.int_val : state->def->default_val.int_val;
}

double seargs_get_double(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_DOUBLE);
  if (!state)
    return 0.0;
  return state->is_found ? state->val.double_val : state->def->default_val.double_val;
}

const char *seargs_get_string(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_STRING);
  if (!state)
    return NULL;
  return state->is_found ? state->val.str_val : state->def->default_val.str_val;
}

void seargs_free(seargs_parser_t *parser) { free(parser); } // free(NULL) is a no-op

#endif // SEARGS_IMPLEMENTATION