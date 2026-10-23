#ifndef SEARGS_H
#define SEARGS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===== Public API =====

#define SEARGS_NO_SHORT_NAME '\0'
#ifndef ARG_NAME_MAX_SIZE
#define ARG_NAME_MAX_SIZE 64
#endif // ARG_NAME_MAX_SIZE
#ifndef ARG_VAL_MAX_SIZE
#define ARG_VAL_MAX_SIZE 256
#endif // ARG_VAL_MAX_SIZE

#define SEARGS_FLAG_POSITIONAL (1u << 0)
// Match the long name case-insensitively.
#define SEARGS_FLAG_CASE_INSENSITIVE (1u << 1)
// Allow a value-taking option to appear more than once; the last value wins.
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
  SEARGS_DUPLICATE_ARG, // a non-repeatable argument was given more than once

  // Definition validation errors
  SEARGS_DEF_TABLE_NULL,
  SEARGS_DEF_NAME_NULL,
  SEARGS_DEF_NAME_EMPTY,
  SEARGS_DEF_NAME_INVALID, // leading '-' or contains ' '/'\t'/'='/'\n'/'\r'
  SEARGS_DEF_NAME_TOO_LONG,
  SEARGS_DEF_DEFAULT_TOO_LONG,
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
  const char *desc; // may be NULL
  seargs_arg_val_t default_val;
  seargs_arg_type_t type;
  uint32_t flags;  // 0 or a combination of SEARGS_FLAG_*
  char short_name; // '\0' = none
  bool is_required;
} seargs_arg_def_t;

#define SEARGS_REQUIRED_BOOL_ARG(name_, short_, desc_)                                             \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_BOOL,                                                                       \
   .short_name = (short_),                                                                         \
   .is_required = true}

#define SEARGS_OPTIONAL_BOOL_ARG(name_, short_, desc_, default_)                                   \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.bool_val = (default_)},                                                        \
   .type = SEARGS_TYPE_BOOL,                                                                       \
   .short_name = (short_)}

#define SEARGS_REQUIRED_STRING_ARG(name_, short_, desc_)                                           \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_STRING,                                                                     \
   .short_name = (short_),                                                                         \
   .is_required = true}

#define SEARGS_OPTIONAL_STRING_ARG(name_, short_, desc_, default_)                                 \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.str_val = (default_)},                                                         \
   .type = SEARGS_TYPE_STRING,                                                                     \
   .short_name = (short_)}

#define SEARGS_POSITIONAL_REQUIRED_STRING_ARG(name_, desc_)                                        \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_STRING,                                                                     \
   .flags = SEARGS_FLAG_POSITIONAL,                                                                \
   .is_required = true}

#define SEARGS_POSITIONAL_OPTIONAL_STRING_ARG(name_, desc_, default_)                              \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.str_val = (default_)},                                                         \
   .type = SEARGS_TYPE_STRING,                                                                     \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_REQUIRED_INT_ARG(name_, short_, desc_)                                              \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_INT,                                                                        \
   .short_name = (short_),                                                                         \
   .is_required = true}

#define SEARGS_OPTIONAL_INT_ARG(name_, short_, desc_, default_)                                    \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.int_val = (default_)},                                                         \
   .type = SEARGS_TYPE_INT,                                                                        \
   .short_name = (short_)}

#define SEARGS_POSITIONAL_REQUIRED_INT_ARG(name_, desc_)                                           \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_INT,                                                                        \
   .flags = SEARGS_FLAG_POSITIONAL,                                                                \
   .is_required = true}

#define SEARGS_POSITIONAL_OPTIONAL_INT_ARG(name_, desc_, default_)                                 \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.int_val = (default_)},                                                         \
   .type = SEARGS_TYPE_INT,                                                                        \
   .flags = SEARGS_FLAG_POSITIONAL}

#define SEARGS_REQUIRED_DOUBLE_ARG(name_, short_, desc_)                                           \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_DOUBLE,                                                                     \
   .short_name = (short_),                                                                         \
   .is_required = true}

#define SEARGS_OPTIONAL_DOUBLE_ARG(name_, short_, desc_, default_)                                 \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.double_val = (default_)},                                                      \
   .type = SEARGS_TYPE_DOUBLE,                                                                     \
   .short_name = (short_)}

#define SEARGS_POSITIONAL_REQUIRED_DOUBLE_ARG(name_, desc_)                                        \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .type = SEARGS_TYPE_DOUBLE,                                                                     \
   .flags = SEARGS_FLAG_POSITIONAL,                                                                \
   .is_required = true}

#define SEARGS_POSITIONAL_OPTIONAL_DOUBLE_ARG(name_, desc_, default_)                              \
  {.name = (name_),                                                                                \
   .desc = (desc_),                                                                                \
   .default_val = {.double_val = (default_)},                                                      \
   .type = SEARGS_TYPE_DOUBLE,                                                                     \
   .flags = SEARGS_FLAG_POSITIONAL}

#define IS_SEARGS_OK(p_) ((p_)->error.code == SEARGS_OK)

typedef struct {
  const seargs_arg_def_t *def; // back-pointer; non-NULL after a successful parse
  size_t def_index;            // index of def within arg_defs
  seargs_arg_val_t val;
  bool is_found; // true when the argument was seen
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
  bool help_requested; // true when argc == 1 triggered the automatic help listing
} seargs_parser_t;

// Parse argv against defs.
//   - On allocation failure returns NULL.
//   - On a definition or parse error returns a parser whose error.code != SEARGS_OK;
//     it must still be released with seargs_free.
//   - On success returns a parser with error.code == SEARGS_OK.
//
// When argc == 1, a usage listing is printed (after definitions validate) and
// parser->help_requested is set; the parser is returned with error.code ==
// SEARGS_OK and no required-argument checks. Callers should check
// seargs_help_requested() before using the parsed values and normally exit
// successfully in that case.
seargs_parser_t *seargs_parse_args(int argc, char *const argv[], const seargs_arg_def_t *arg_defs,
                                   size_t n_arg_defs);

// Same as seargs_parse_args, but print_help_on_no_args controls the automatic
// argc == 1 help listing. Pass false for tools whose no-argument behavior is
// meaningful.
seargs_parser_t *seargs_parse_args_ex(int argc, char *const argv[],
                                      const seargs_arg_def_t *arg_defs, size_t n_arg_defs,
                                      bool print_help_on_no_args);

// Frees the parser and everything it owns.
void seargs_free(seargs_parser_t *parser);

// True when the automatic argc == 1 help listing was printed.
bool seargs_help_requested(const seargs_parser_t *p);

// State lookup. Returns NULL when name is not a defined argument. Safe to call
// on a parser returned with a definition-validation error.
const seargs_arg_state_t *seargs_get_state(const seargs_parser_t *p, const char *name);

// Returns true if the argument was provided on the command line.
bool seargs_is_found(const seargs_parser_t *p, const char *name);

// Prints a usage listing to stdout. Options are shown as --name; positional
// arguments are shown as <name> (required) or [name] (optional).
void seargs_print_help(const seargs_arg_def_t *defs, size_t num_args);

// Typed accessors. They assert (in debug builds) that the name exists and that the
// argument's type matches; in release builds a missing name or type mismatch returns
// the fallback value (false, 0, 0.0 or NULL). They return the parsed value, or the
// definition's default when the argument was not provided. Only meaningful when the
// defs validated (parser->error.code == SEARGS_OK).
bool seargs_get_bool(const seargs_parser_t *p, const char *name);
int seargs_get_int(const seargs_parser_t *p, const char *name);
double seargs_get_double(const seargs_parser_t *p, const char *name);
const char *seargs_get_string(const seargs_parser_t *p, const char *name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SEARGS_H

#ifdef SEARGS_IMPLEMENTATION
#ifndef SEARGS_IMPLEMENTATION_ONCE
#define SEARGS_IMPLEMENTATION_ONCE

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool seargs_str_to_int(const char *str, int *out) {
  if (!str || !out)
    return false;
  // strtol skips leading whitespace. reject it so numeric parsing is symmetric
  // with the trailing-garbage check below.
  if (isspace((unsigned char)str[0]))
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

static bool seargs_str_to_double(const char *str, double *out) {
  if (!str || !out)
    return false;
  // See seargs_str_to_int: reject leading whitespace accepted by strtod.
  if (isspace((unsigned char)str[0]))
    return false;
  errno = 0;
  char *endptr = NULL;
  double result = strtod(str, &endptr);

  if (endptr == str)
    return false;
  if (*endptr != '\0')
    return false;
  if (errno == ERANGE && (result == HUGE_VAL || result == -HUGE_VAL))
    return false;

  *out = result;
  return true;
}

static bool seargs_str_ieq(const char *a, const char *b) {
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
      return false;
    a++;
    b++;
  }
  return *a == *b;
}

static bool seargs_str_to_bool(const char *s, bool *out) {
  if (!s || !out)
    return false;

  if (seargs_str_ieq(s, "true") || strcmp(s, "1") == 0) {
    *out = true;
    return true;
  }

  if (seargs_str_ieq(s, "false") || strcmp(s, "0") == 0) {
    *out = false;
    return true;
  }

  return false;
}

static void seargs_report_err(seargs_error_info_t *err, seargs_error_t code, size_t def_index,
                              const char *argv_token, const char *detail) {
  if (!err || err->code != SEARGS_OK)
    return;

  err->code = code;
  err->index = def_index;
  err->arg = argv_token;
  err->detail = detail;
}

static inline bool seargs_is_valid_name_char(char c) {
  return c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '=';
}

static inline bool seargs_is_valid_short_name(char c) {
  unsigned char u = (unsigned char)c;
  return c != '-' && c != '=' && c != ' ' && u >= 0x20 && u != 0x7f;
}

static inline bool seargs_is_long_arg(const char *arg) { return arg[0] == '-' && arg[1] == '-'; }

// A token that looks like an option. A lone '-' is deliberately not an option
// token.
static inline bool seargs_is_option_token(const char *t) { return t[0] == '-' && t[1] != '\0'; }

// True when token is a standalone boolean literal (true/false/1/0). This lets
// `--flag false` and `-f 0` work without treating arbitrary following tokens as
// values.
static bool seargs_is_bool_value_token(const char *token) {
  bool ignored;
  return token && token[0] != '\0' && strcmp(token, "--") != 0 && !seargs_is_option_token(token) &&
         seargs_str_to_bool(token, &ignored);
}

static size_t seargs_find_def_index(const seargs_parser_t *p, const char *name) {
  if (!p || !p->arg_defs || !name)
    return SIZE_MAX;

  for (size_t i = 0; i < p->n_arg_defs; i++) {
    if (p->arg_defs[i].name && strcmp(p->arg_defs[i].name, name) == 0)
      return i;
  }
  return SIZE_MAX;
}

// Long-name match
static bool seargs_name_equals(const seargs_arg_def_t *def, const char *name) {
  if (!def || !def->name || !name)
    return false;
  if (def->flags & SEARGS_FLAG_CASE_INSENSITIVE)
    return seargs_str_ieq(def->name, name);
  return strcmp(def->name, name) == 0;
}

static bool seargs_names_conflict(const seargs_arg_def_t *a, const seargs_arg_def_t *b) {
  if (!a || !b || !a->name || !b->name)
    return false;
  if (strcmp(a->name, b->name) == 0)
    return true;
  // A case-insensitive definition claims every spelling of its name, so two
  // definitions that differ only in case are ambiguous if either opts in.
  if (((a->flags | b->flags) & SEARGS_FLAG_CASE_INSENSITIVE) && seargs_str_ieq(a->name, b->name))
    return true;
  return false;
}

// Like seargs_find_def_index, but skips positional arguments: their names are
// matched by position and cannot be used as options.
static size_t seargs_find_option_def_index(const seargs_parser_t *p, const char *name) {
  if (!p || !p->arg_defs || !name)
    return SIZE_MAX;

  for (size_t i = 0; i < p->n_arg_defs; i++) {
    if (!(p->arg_defs[i].flags & SEARGS_FLAG_POSITIONAL) &&
        seargs_name_equals(&p->arg_defs[i], name))
      return i;
  }
  return SIZE_MAX;
}

static size_t seargs_find_def_index_s(const seargs_parser_t *p, char short_name) {
  if (!p || !p->arg_defs || short_name == SEARGS_NO_SHORT_NAME)
    return SIZE_MAX;

  for (size_t i = 0; i < p->n_arg_defs; i++) {
    if (p->arg_defs[i].short_name == short_name)
      return i;
  }
  return SIZE_MAX;
}

static void seargs_validate_defs(const seargs_arg_def_t *defs, size_t n,
                                 seargs_error_info_t *p_err) {

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
      if (!seargs_is_valid_name_char(*p)) {
        seargs_report_err(p_err, SEARGS_DEF_NAME_INVALID, i, NULL,
                          "arg name contains an invalid character");
        return;
      }

    // Names are copied into a fixed-size stack buffer while parsing. reject
    // unusable definitions here instead of failing later on the command line.
    if (strlen(d->name) >= ARG_NAME_MAX_SIZE) {
      seargs_report_err(p_err, SEARGS_DEF_NAME_TOO_LONG, i, NULL,
                        "arg name exceeds ARG_NAME_MAX_SIZE");
      return;
    }

    // --- Short name ---
    if (sc != SEARGS_NO_SHORT_NAME && !seargs_is_valid_short_name(sc)) {
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

    // Default strings are returned verbatim to callers, so enforce the same
    // value bound the parser applies to command-line values.
    if (d->type == SEARGS_TYPE_STRING && d->default_val.str_val &&
        strlen(d->default_val.str_val) >= ARG_VAL_MAX_SIZE) {
      seargs_report_err(p_err, SEARGS_DEF_DEFAULT_TOO_LONG, i, NULL,
                        "default string exceeds ARG_VAL_MAX_SIZE");
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
      if (seargs_names_conflict(d, &defs[j])) {
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

static bool seargs_validate_and_default_state(seargs_arg_state_t *state,
                                              seargs_error_info_t *p_err) {
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
static const char *seargs_option_next_value(char *const argv[], int argc, int *p_i,
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
  if (seargs_is_option_token(next)) {
    bool numeric = false;
    if (def->type == SEARGS_TYPE_INT) {
      int tmp;
      numeric = seargs_str_to_int(next, &tmp);
    } else if (def->type == SEARGS_TYPE_DOUBLE) {
      double tmp;
      numeric = seargs_str_to_double(next, &tmp);
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

static bool seargs_assign_value(seargs_parser_t *parser, seargs_arg_state_t *state,
                                size_t def_index, const char *opt_token, const char *val) {
  assert(parser);
  assert(state && state->def);
  assert(val);

  // Repeating a boolean is harmless (it is presence-based), but silently
  // overwriting a value-taking option is usually a mistake. SEARGS_FLAG_REPEATABLE
  // opts into last-one-wins behavior.
  if (state->is_found && state->def->type != SEARGS_TYPE_BOOL &&
      !(state->def->flags & SEARGS_FLAG_REPEATABLE)) {
    seargs_report_err(&parser->error, SEARGS_DUPLICATE_ARG, def_index, opt_token,
                      "argument was specified more than once; set "
                      "SEARGS_FLAG_REPEATABLE to allow it");
    return false;
  }

  const size_t val_len = strlen(val);
  if (val_len >= ARG_VAL_MAX_SIZE) {
    seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                      "argument value exceeds maximum size; override "
                      "ARG_VAL_MAX_SIZE to raise the limit");
    return false;
  }

  switch (state->def->type) {
  case SEARGS_TYPE_BOOL:
    if (!seargs_str_to_bool(val, &state->val.bool_val)) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                        "Given argument's value is not a boolean");
      return false;
    }
    break;
  case SEARGS_TYPE_INT:
    if (!seargs_str_to_int(val, &state->val.int_val)) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, val,
                        "Given argument's value is not an integer");
      return false;
    }
    break;
  case SEARGS_TYPE_DOUBLE:
    if (!seargs_str_to_double(val, &state->val.double_val)) {
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
static bool seargs_parser_has_positionals(const seargs_parser_t *p) {
  for (size_t i = 0; i < p->n_arg_defs; i++)
    if (p->arg_defs[i].flags & SEARGS_FLAG_POSITIONAL)
      return true;
  return false;
}

// Gives token to the next positional def that has no value yet, scanning from
// *cursor. Exactly one slot is filled per call. Reports SEARGS_TOO_MANY_ARGS
// when every positional def is already filled.
static bool seargs_parse_positional(seargs_parser_t *parser, size_t *cursor, const char *token) {
  size_t idx = *cursor;

  while (idx < parser->n_arg_defs && !(parser->arg_defs[idx].flags & SEARGS_FLAG_POSITIONAL))
    idx++;

  if (idx == parser->n_arg_defs) {
    seargs_report_err(&parser->error, SEARGS_TOO_MANY_ARGS, SIZE_MAX, token,
                      "too many positional arguments");
    return false;
  }

  *cursor = idx + 1;
  return seargs_assign_value(parser, &parser->states[idx], idx, token, token);
}

static bool seargs_parse_long_arg(char *const argv[], int argc, seargs_parser_t *parser, int *p_i) {
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
    // Accept an explicitly separated boolean value, but only when the next token
    // is unambiguously a boolean literal else presence is true.
    const char *next = (*p_i + 1 < argc) ? argv[*p_i + 1] : NULL;
    if (seargs_is_bool_value_token(next)) {
      val = next;
      consumed = 2;
    } else {
      val = "1";
      consumed = 1;
    }
  } else {
    val = seargs_option_next_value(argv, argc, p_i, def, def_index, arg, &parser->error);
    if (!val)
      return false; // error already set by seargs_option_next_value
    consumed = 2;
  }

  if (!seargs_assign_value(parser, state, def_index, arg, val))
    return false; // error already set by seargs_assign_value

  *p_i += consumed - 1;
  return true;
}

static bool seargs_parse_short_arg(char *const argv[], int argc, seargs_parser_t *parser,
                                   int *p_i) {
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
      // A standalone short bool may take an explicit value (`-f false`). Inside a
      // cluster the literal is left alone, so `-ab false` keeps `false` as a
      // positional token rather than guessing which flag it belongs to.
      const char *val = "1";
      if (j == 0 && is_last) {
        const char *next = (*p_i + 1 < argc) ? argv[*p_i + 1] : NULL;
        if (seargs_is_bool_value_token(next)) {
          val = next;
          ++*p_i; // consume the value token
        }
      }
      if (!seargs_assign_value(parser, state, def_index, opt_token, val))
        return false; // error already set by seargs_assign_value
      continue;
    }

    if (!is_last) {
      seargs_report_err(&parser->error, SEARGS_INVALID_VALUE, def_index, opt_token,
                        "Non-flag argument must be last in cluster.");
      return false;
    }

    const char *val =
        seargs_option_next_value(argv, argc, p_i, def, def_index, opt_token, &parser->error);
    if (!val)
      return false; // error already set by seargs_option_next_value

    ++*p_i; // consume the value token; the outer loop advances past it as well
    if (!seargs_assign_value(parser, state, def_index, opt_token, val))
      return false; // error already set by seargs_assign_value
  }
  return true;
}

// A single allocation containing the parser followed by a correctly aligned
// states array
typedef struct {
  seargs_parser_t parser;
  seargs_arg_state_t states[1];
} seargs_parse_block_t;

seargs_parser_t *seargs_parse_args_ex(int argc, char *const argv[],
                                      const seargs_arg_def_t *arg_defs, size_t n_arg_defs,
                                      bool print_help_on_no_args) {
  const size_t states_offset = offsetof(seargs_parse_block_t, states);

  // Reject a state count that would overflow the single-block allocation.
  if (n_arg_defs > (SIZE_MAX - states_offset) / sizeof(seargs_arg_state_t))
    return NULL;

  const size_t block_size = states_offset + n_arg_defs * sizeof(seargs_arg_state_t);
  seargs_parse_block_t *block = (seargs_parse_block_t *)calloc(1, block_size);
  if (!block)
    return NULL;

  seargs_parser_t *parser = &block->parser;
  parser->arg_defs = arg_defs;
  parser->n_arg_defs = n_arg_defs;
  parser->states = block->states;

  parser->error.code = SEARGS_OK;
  parser->error.index = SIZE_MAX;
  parser->error.arg = NULL;
  parser->error.detail = NULL;
  parser->help_requested = false;

  // Wire states before validation so accessors never dereference a NULL def
  // pointer. A NULL arg_defs table (with n_arg_defs > 0) is caught by validation
  // below and leaves the states unwired.
  if (arg_defs) {
    for (size_t i = 0; i < n_arg_defs; i++) {
      parser->states[i].def = &arg_defs[i];
      parser->states[i].def_index = i;
    }
  }

  // Validate before printing help: an invalid table must report its error
  // instead of crashing in the help formatter.
  seargs_validate_defs(parser->arg_defs, parser->n_arg_defs, &parser->error);
  if (parser->error.code != SEARGS_OK)
    return parser; // error is already set in this path.

  if (print_help_on_no_args && argc == 1) {
    parser->help_requested = true;
    seargs_print_help(parser->arg_defs, parser->n_arg_defs);
    // No required-argument checks: the caller should exit successfully after
    // showing help. Accessors still fall back to definition defaults.
    return parser;
  }

  // Main parse loop
  const bool has_positionals = seargs_parser_has_positionals(parser);
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
      if (has_positionals && !seargs_parse_positional(parser, &pos_cursor, arg))
        return parser; // error already set
      continue;
    }

    // A lone '-' is not option-shaped: it fills a positional slot when one is
    // open and is an error otherwise.
    if (strcmp(arg, "-") == 0) {
      if (has_positionals) {
        if (!seargs_parse_positional(parser, &pos_cursor, arg))
          return parser; // error already set
      } else {
        seargs_report_err(&parser->error, SEARGS_UNKNOWN_ARG, SIZE_MAX, arg,
                          "lone '-' is not a valid option");
        return parser; // error already set
      }
      continue;
    }

    if (seargs_is_long_arg(arg)) {
      if (!seargs_parse_long_arg(argv, argc, parser, &i))
        return parser; // error already set
    } else if (arg[0] == '-') {
      if (!seargs_parse_short_arg(argv, argc, parser, &i))
        return parser; // error already set
    } else if (has_positionals) {
      if (!seargs_parse_positional(parser, &pos_cursor, arg))
        return parser; // error already set
    }
    // Without positional arguments, bare tokens are ignored.
  }

  for (size_t i = 0; i < n_arg_defs; i++) {
    seargs_arg_state_t *state = &parser->states[i];
    if (!seargs_validate_and_default_state(state, &parser->error))
      return parser; // error already set
  }

  return parser;
}

seargs_parser_t *seargs_parse_args(int argc, char *const argv[], const seargs_arg_def_t *arg_defs,
                                   size_t n_arg_defs) {
  return seargs_parse_args_ex(argc, argv, arg_defs, n_arg_defs, true);
}

void seargs_print_help(const seargs_arg_def_t *defs, size_t n_args) {
  if (!defs)
    return;

  size_t max_label_len = 0;
  for (size_t i = 0; i < n_args; i++) {
    if (!defs[i].name)
      continue; // tolerate a malformed table; validation reports the real error
    const size_t label_len = strlen(defs[i].name) + 2; // "--name" or "<name>"/"[name]"
    if (label_len > max_label_len)
      max_label_len = label_len;
  }

  printf("Usage:\n");
  for (size_t i = 0; i < n_args; i++) {
    const char *name = defs[i].name;
    if (!name)
      continue;

    const bool positional = (defs[i].flags & SEARGS_FLAG_POSITIONAL) != 0;
    const size_t label_len = strlen(name) + 2;
    char short_name_part[6];

    if (positional) {
      const char open = defs[i].is_required ? '<' : '[';
      const char close = defs[i].is_required ? '>' : ']';
      printf("  %c%s%c", open, name, close);
      snprintf(short_name_part, sizeof(short_name_part), "    ");
    } else {
      printf("  --%s", name);
      if (defs[i].short_name)
        snprintf(short_name_part, sizeof(short_name_part), "(-%c)", defs[i].short_name);
      else
        snprintf(short_name_part, sizeof(short_name_part), "    ");
    }

    size_t pad = max_label_len > label_len ? max_label_len - label_len : 0;
    if (pad > (size_t)(INT_MAX - 1))
      pad = (size_t)(INT_MAX - 1);
    if (!defs[i].desc || defs[i].desc[0] == '\0') {
      putchar('\n');
      continue;
    }
    printf("%*s %s  %s\n", (int)(pad + 1), "", short_name_part, defs[i].desc);
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

bool seargs_help_requested(const seargs_parser_t *p) { return p && p->help_requested; }

bool seargs_get_bool(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_BOOL);
  if (!state || !state->def || state->def->type != SEARGS_TYPE_BOOL)
    return false;
  return state->is_found ? state->val.bool_val : state->def->default_val.bool_val;
}

int seargs_get_int(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_INT);
  if (!state || !state->def || state->def->type != SEARGS_TYPE_INT)
    return 0;
  return state->is_found ? state->val.int_val : state->def->default_val.int_val;
}

double seargs_get_double(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_DOUBLE);
  if (!state || !state->def || state->def->type != SEARGS_TYPE_DOUBLE)
    return 0.0;
  return state->is_found ? state->val.double_val : state->def->default_val.double_val;
}

const char *seargs_get_string(const seargs_parser_t *p, const char *name) {
  const seargs_arg_state_t *state = seargs_get_state(p, name);
  assert(state && state->def && state->def->type == SEARGS_TYPE_STRING);
  if (!state || !state->def || state->def->type != SEARGS_TYPE_STRING)
    return NULL;
  return state->is_found ? state->val.str_val : state->def->default_val.str_val;
}

void seargs_free(seargs_parser_t *parser) { free(parser); }

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SEARGS_IMPLEMENTATION_ONCE
#endif // SEARGS_IMPLEMENTATION
