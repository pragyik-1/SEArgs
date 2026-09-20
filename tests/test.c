// Verification driver for seargs. Compiles clean with -Wall -Wextra -Werror.
#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

#include <stdio.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                                              \
  do {                                                                                           \
    checks++;                                                                                    \
    if (!(cond)) {                                                                               \
      failures++;                                                                                \
      fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                            \
    }                                                                                            \
  } while (0)

#define PARSE(defs_arr, ...)                                                                     \
  (seargs_parse_args((int)(sizeof((char *[]){__VA_ARGS__}) / sizeof(char *)),                    \
                     (char *const *)(char *[]){__VA_ARGS__}, (defs_arr),                         \
                     sizeof(defs_arr) / sizeof((defs_arr)[0])))

static const seargs_arg_def_t common_defs[] = {
    {.name = "flag", .short_name = 'f', .type = SEARGS_TYPE_BOOL},
    {.name = "num", .short_name = 'n', .type = SEARGS_TYPE_INT},
    {.name = "dbl", .short_name = 'd', .type = SEARGS_TYPE_DOUBLE},
    {.name = "str", .short_name = 's', .type = SEARGS_TYPE_STRING, .default_val = {.str_val = "defstr"}},
    {.name = "opt", .type = SEARGS_TYPE_INT, .default_val = {.int_val = 42}},
    {.name = "a", .short_name = 'a', .type = SEARGS_TYPE_BOOL},
    {.name = "b", .short_name = 'b', .type = SEARGS_TYPE_BOOL},
};

static const seargs_arg_def_t req_defs[] = {
    {.name = "x", .type = SEARGS_TYPE_STRING},
    {.name = "y", .type = SEARGS_TYPE_INT},
    {.name = "z", .type = SEARGS_TYPE_STRING, .is_required = true},
};

static const seargs_arg_def_t pos_defs[] = {
    {.name = "output", .short_name = 'o', .type = SEARGS_TYPE_STRING},
    {.name = "input", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_STRING},
    {.name = "input2", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_STRING,
     .default_val = {.str_val = "none"}},
    {.name = "verbose", .short_name = 'v', .type = SEARGS_TYPE_BOOL},
};

static const seargs_arg_def_t pos_req_defs[] = {
    {.name = "a", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_STRING},
    {.name = "b", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_STRING,
     .is_required = true},
};

static const seargs_arg_def_t pos_typed_defs[] = {
    {.name = "num", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_INT},
    {.name = "d", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_DOUBLE},
    {.name = "b", .flags = SEARGS_FLAG_POSITIONAL, .type = SEARGS_TYPE_BOOL},
};

// The macro frontend is pure sugar: every SEARGS_*_ARG macro expands to a def
// that must behave identically to the hand-written initializer it stands for.
static const seargs_arg_def_t macro_defs[] = {
    SEARGS_OPTIONAL_BOOL_ARG("flag", 'f', NULL, false),
    SEARGS_REQUIRED_BOOL_ARG("rb", 'r', NULL),
    SEARGS_OPTIONAL_BOOL_ARG("bd", 'b', NULL, true),
    SEARGS_OPTIONAL_STRING_ARG("str", 's', NULL, NULL),
    SEARGS_REQUIRED_STRING_ARG("qstr", 'q', NULL),
    SEARGS_OPTIONAL_STRING_ARG("dstr", 'd', NULL, "def"),
    SEARGS_OPTIONAL_INT_ARG("num", 'n', NULL, 0),
    SEARGS_REQUIRED_INT_ARG("rnum", 'm', NULL),
    SEARGS_OPTIONAL_INT_ARG("dnum", 'i', NULL, 7),
    SEARGS_OPTIONAL_DOUBLE_ARG("dbl", 'x', NULL, 0.0),
    SEARGS_REQUIRED_DOUBLE_ARG("rdbl", 'y', NULL),
    SEARGS_OPTIONAL_DOUBLE_ARG("ddbl", 'z', NULL, 2.5),
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("pstr", NULL, ""),
    SEARGS_POSITIONAL_REQUIRED_STRING_ARG("prstr", NULL),
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("pdstr", NULL, "pd"),
    SEARGS_POSITIONAL_OPTIONAL_INT_ARG("pnum", NULL, 0),
    SEARGS_POSITIONAL_REQUIRED_INT_ARG("prnum", NULL),
    SEARGS_POSITIONAL_OPTIONAL_INT_ARG("pdnum", NULL, 3),
    SEARGS_POSITIONAL_OPTIONAL_DOUBLE_ARG("pdbl", NULL, 0.0),
    SEARGS_POSITIONAL_REQUIRED_DOUBLE_ARG("prdbl", NULL),
    SEARGS_POSITIONAL_OPTIONAL_DOUBLE_ARG("pddbl", NULL, 1.5),
};

static const seargs_arg_def_t raw_defs[] = {
    {.name = "flag",
     .short_name = 'f',
     .type = SEARGS_TYPE_BOOL,
     .default_val = {.bool_val = false}},
    {.name = "rb", .short_name = 'r', .type = SEARGS_TYPE_BOOL, .is_required = true},
    {.name = "bd", .short_name = 'b', .type = SEARGS_TYPE_BOOL, .default_val = {.bool_val = true}},
    {.name = "str",
     .short_name = 's',
     .type = SEARGS_TYPE_STRING,
     .default_val = {.str_val = NULL}},
    {.name = "qstr", .short_name = 'q', .type = SEARGS_TYPE_STRING, .is_required = true},
    {.name = "dstr",
     .short_name = 'd',
     .type = SEARGS_TYPE_STRING,
     .default_val = {.str_val = "def"}},
    {.name = "num", .short_name = 'n', .type = SEARGS_TYPE_INT, .default_val = {.int_val = 0}},
    {.name = "rnum", .short_name = 'm', .type = SEARGS_TYPE_INT, .is_required = true},
    {.name = "dnum", .short_name = 'i', .type = SEARGS_TYPE_INT, .default_val = {.int_val = 7}},
    {.name = "dbl",
     .short_name = 'x',
     .type = SEARGS_TYPE_DOUBLE,
     .default_val = {.double_val = 0.0}},
    {.name = "rdbl", .short_name = 'y', .type = SEARGS_TYPE_DOUBLE, .is_required = true},
    {.name = "ddbl",
     .short_name = 'z',
     .type = SEARGS_TYPE_DOUBLE,
     .default_val = {.double_val = 2.5}},
    {.name = "pstr",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_STRING,
     .default_val = {.str_val = ""}},
    {.name = "prstr",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_STRING,
     .is_required = true},
    {.name = "pdstr",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_STRING,
     .default_val = {.str_val = "pd"}},
    {.name = "pnum",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_INT,
     .default_val = {.int_val = 0}},
    {.name = "prnum",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_INT,
     .is_required = true},
    {.name = "pdnum",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_INT,
     .default_val = {.int_val = 3}},
    {.name = "pdbl",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_DOUBLE,
     .default_val = {.double_val = 0.0}},
    {.name = "prdbl",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_DOUBLE,
     .is_required = true},
    {.name = "pddbl",
     .flags = SEARGS_FLAG_POSITIONAL,
     .type = SEARGS_TYPE_DOUBLE,
     .default_val = {.double_val = 1.5}},
};

// A table of only optional positionals: the defaults must apply when no token
// ever reaches them (a middle optional positional is otherwise always filled).
static const seargs_arg_def_t pos_default_defs[] = {
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("ps", NULL, ""),
    SEARGS_POSITIONAL_OPTIONAL_INT_ARG("pi", NULL, -1),
    SEARGS_POSITIONAL_OPTIONAL_DOUBLE_ARG("pd", NULL, -1.5),
};

// True when both parsers report the same found-state and value for 'name'.
static bool same_val(const seargs_parser_t *a, const seargs_parser_t *b, const char *name) {
  const seargs_arg_state_t *sa = seargs_get_state(a, name);
  const seargs_arg_state_t *sb = seargs_get_state(b, name);
  if (!sa || !sb)
    return false;
  if (sa->is_found != sb->is_found)
    return false;
  switch (sa->def->type) {
  case SEARGS_TYPE_BOOL:
    return sa->val.bool_val == sb->val.bool_val;
  case SEARGS_TYPE_INT:
    return sa->val.int_val == sb->val.int_val;
  case SEARGS_TYPE_DOUBLE:
    return sa->val.double_val == sb->val.double_val;
  case SEARGS_TYPE_STRING:
    return (sa->val.str_val == sb->val.str_val) ||
           (sa->val.str_val != NULL && sb->val.str_val != NULL &&
            strcmp(sa->val.str_val, sb->val.str_val) == 0);
  }
  return false;
}

static void t_long_bool(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--flag");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_OK);
  CHECK(seargs_is_found(p, "flag"));
  CHECK(seargs_get_bool(p, "flag") == true);
  CHECK(seargs_get_state(p, "zzz") == NULL); // unknown name
  seargs_free(p);
}

static void t_bool_eq(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--flag=false");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_is_found(p, "flag"));
  CHECK(seargs_get_bool(p, "flag") == false);

  seargs_free(p);
  p = PARSE(common_defs, "prog", "--flag=1");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_bool(p, "flag") == true);
  seargs_free(p);
}

static void t_bool_absent_default(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_is_found(p, "flag") == false);
  CHECK(seargs_get_bool(p, "flag") == false);
  CHECK(seargs_get_int(p, "opt") == 42); // default
  CHECK(seargs_get_string(p, "str") != NULL);
  CHECK(strcmp(seargs_get_string(p, "str"), "defstr") == 0); // default
  seargs_free(p);
}

static void t_int_values(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--num", "5");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_int(p, "num") == 5);

  seargs_free(p);
  p = PARSE(common_defs, "prog", "--num=-5");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_int(p, "num") == -5);

  seargs_free(p);
  p = PARSE(common_defs, "prog", "--num", "-5"); // dash-prefixed numeric value
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_int(p, "num") == -5);
  seargs_free(p);
}

static void t_int_dash_bad(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--num", "-x"); // not numeric
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  CHECK(p->error.index == 1); // def index of "num"
  CHECK(p->error.arg != NULL && strcmp(p->error.arg, "-x") == 0);
  seargs_free(p);
}

static void t_int_terminator_value(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--num", "--"); // "--" is never a value
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_MISSING_VALUE);
  CHECK(p->error.index == 1);
  seargs_free(p);
}

static void t_str_values(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--str", "abc");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "str"), "abc") == 0);
  seargs_free(p);

  // Dash-prefixed string values must use '='.
  p = PARSE(common_defs, "prog", "--str", "-x");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  seargs_free(p);

  p = PARSE(common_defs, "prog", "--str=-x");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "str"), "-x") == 0);
  seargs_free(p);

  // A lone "-" is not option-shaped, so it is acceptable as a value.
  p = PARSE(common_defs, "prog", "--str", "-");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "str"), "-") == 0);
  seargs_free(p);
}

static void t_short_cluster(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "-abn", "5");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_is_found(p, "a"));
  CHECK(seargs_is_found(p, "b"));
  CHECK(seargs_get_bool(p, "a") == true);
  CHECK(seargs_get_bool(p, "b") == true);
  CHECK(seargs_get_int(p, "num") == 5);
  seargs_free(p);
}

static void t_short_cluster_bad(void) {
  // 'n' is value-taking and not last in the cluster -> error.
  seargs_parser_t *p = PARSE(common_defs, "prog", "-n5");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  seargs_free(p);

  // "-an 5" is valid: 'a' is a flag, 'n' is last and takes the value.
  p = PARSE(common_defs, "prog", "-an", "5");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_bool(p, "a") == true);
  CHECK(seargs_get_int(p, "num") == 5);
  seargs_free(p);
}

static void t_lone_dash(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "-");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_UNKNOWN_ARG);
  seargs_free(p);
}

static void t_unknown_long(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--nope");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_UNKNOWN_ARG);
  CHECK(p->error.index == SIZE_MAX);
  CHECK(p->error.arg != NULL && strcmp(p->error.arg, "--nope") == 0);
  seargs_free(p);
}

static void t_required_missing(void) {
  seargs_parser_t *p = PARSE(req_defs, "prog");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_MISSING_ARG);
  CHECK(p->error.index == 2); // def index of the required "z"
  CHECK(p->error.arg == NULL);
  seargs_free(p);
}

static void t_missing_value(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--num");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_MISSING_VALUE);
  seargs_free(p);

  p = PARSE(common_defs, "prog", "-n");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_MISSING_VALUE);
  seargs_free(p);
}

static void t_value_too_long(void) {
  char big[ARG_VAL_MAX_SIZE + 20];
  memset(big, 'x', sizeof(big) - 1);
  big[sizeof(big) - 1] = '\0';

  seargs_parser_t *p =
      seargs_parse_args(4, (char *const[]){ "prog", "--num", big, NULL }, common_defs,
                        sizeof(common_defs) / sizeof(common_defs[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  seargs_free(p);

  p = seargs_parse_args(4, (char *const[]){ "prog", "-n", big, NULL }, common_defs,
                        sizeof(common_defs) / sizeof(common_defs[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE); // both paths report, no stderr
  seargs_free(p);
}

static void t_duplicates(void) {
  const seargs_arg_def_t dup_long[] = {
      {.name = "same", .type = SEARGS_TYPE_INT},
      {.name = "same", .type = SEARGS_TYPE_STRING},
  };
  seargs_parser_t *p = seargs_parse_args(1, (char *const[]){ "prog" }, dup_long,
                                         sizeof(dup_long) / sizeof(dup_long[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_DEF_DUPLICATE_NAME);
  CHECK(p->error.index == 1);
  seargs_free(p);

  const seargs_arg_def_t dup_short[] = {
      {.name = "a1", .short_name = 'a', .type = SEARGS_TYPE_BOOL},
      {.name = "a2", .short_name = 'a', .type = SEARGS_TYPE_BOOL},
  };
  p = seargs_parse_args(1, (char *const[]){ "prog" }, dup_short,
                        sizeof(dup_short) / sizeof(dup_short[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_DEF_DUPLICATE_SHORT_NAME);
  CHECK(p->error.index == 1);
  seargs_free(p);
}

static void t_empty_values(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--str=");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "str"), "") == 0);
  seargs_free(p);

  p = PARSE(common_defs, "prog", "--num=");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  seargs_free(p);

  p = PARSE(common_defs, "prog", "--flag=");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  seargs_free(p);
}

static void t_double_policies(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--dbl", "1e-400"); // underflow accepted
  CHECK(p && p->error.code == SEARGS_OK);
  seargs_free(p);

  p = PARSE(common_defs, "prog", "--dbl", "1e400"); // overflow rejected
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  seargs_free(p);

  p = PARSE(common_defs, "prog", "--dbl", "-1.5e-3"); // dash-prefixed numeric
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_double(p, "dbl") == -1.5e-3);
  seargs_free(p);
}

static void t_terminator(void) {
  seargs_parser_t *p = PARSE(common_defs, "prog", "--", "-x", "foo", "--num");
  CHECK(p && p->error.code == SEARGS_OK); // everything after "--" is ignored
  CHECK(seargs_is_found(p, "num") == false);
  seargs_free(p);
}

static void t_invalid_flags(void) {
  const seargs_arg_def_t bad_flags[] = {
      {.name = "x", .type = SEARGS_TYPE_INT, .flags = 0x80000000u},
  };
  seargs_parser_t *p = seargs_parse_args(1, (char *const[]){ "prog" }, bad_flags,
                                         sizeof(bad_flags) / sizeof(bad_flags[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_DEF_INVALID_FLAGS);
  seargs_free(p);

  // Reserved-but-unused flags are accepted.
  const seargs_arg_def_t ok_flags[] = {
      {.name = "x", .type = SEARGS_TYPE_INT, .flags = SEARGS_FLAG_POSITIONAL},
  };
  p = seargs_parse_args(1, (char *const[]){ "prog" }, ok_flags,
                        sizeof(ok_flags) / sizeof(ok_flags[0]));
  CHECK(p && p->error.code == SEARGS_OK);
  seargs_free(p);
}

static void t_null_defs(void) {
  seargs_parser_t *p = seargs_parse_args(1, (char *const[]){ "prog" }, NULL, 0);
  CHECK(p && p->error.code == SEARGS_OK);
  seargs_free(p);

  p = seargs_parse_args(1, (char *const[]){ "prog" }, NULL, 3);
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_DEF_TABLE_NULL);
  seargs_free(p);
}

static void t_free_null(void) {
  seargs_free(NULL);
  CHECK(1);
}

static void t_name_too_long(void) {
  char name[ARG_NAME_MAX_SIZE + 10];
  memset(name, 'a', sizeof(name) - 1);
  name[0] = '-';
  name[1] = '-';
  name[sizeof(name) - 1] = '\0';

  seargs_parser_t *p = seargs_parse_args(2, (char *const[]){ "prog", name }, common_defs,
                                         sizeof(common_defs) / sizeof(common_defs[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  CHECK(p->error.index == SIZE_MAX);
  seargs_free(p);
}

static void t_pos_fill_order(void) {
  // Positionals fill in def order, with options interleaved freely.
  seargs_parser_t *p = PARSE(pos_defs, "prog", "a.txt", "b.txt", "-v");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "input"), "a.txt") == 0);
  CHECK(strcmp(seargs_get_string(p, "input2"), "b.txt") == 0);
  CHECK(seargs_is_found(p, "input"));
  CHECK(seargs_is_found(p, "input2"));
  CHECK(seargs_get_bool(p, "verbose") == true);
  CHECK(seargs_is_found(p, "output") == false);
  seargs_free(p);
}

static void t_pos_option_value_priority(void) {
  // The value of --output is consumed as its value, never as a positional.
  seargs_parser_t *p = PARSE(pos_defs, "prog", "--output", "out.txt", "a.txt");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "output"), "out.txt") == 0);
  CHECK(strcmp(seargs_get_string(p, "input"), "a.txt") == 0);
  CHECK(strcmp(seargs_get_string(p, "input2"), "none") == 0); // default applied
  CHECK(seargs_is_found(p, "input2") == false);
  seargs_free(p);
}

static void t_pos_after_terminator(void) {
  // After `--` option-shaped tokens become positional.
  seargs_parser_t *p = PARSE(pos_defs, "prog", "--", "-x", "-v");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "input"), "-x") == 0);
  CHECK(strcmp(seargs_get_string(p, "input2"), "-v") == 0);
  CHECK(seargs_get_bool(p, "verbose") == false);
  seargs_free(p);
}

static void t_pos_lone_dash(void) {
  seargs_parser_t *p = PARSE(pos_defs, "prog", "-");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "input"), "-") == 0);
  seargs_free(p);
}

static void t_pos_required_missing(void) {
  // "a" is filled first, then the unfilled required "b" triggers MISSING_ARG.
  seargs_parser_t *p = PARSE(pos_req_defs, "prog", "x");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_MISSING_ARG);
  CHECK(p->error.index == 1); // def index of "b"
  seargs_free(p);
}

static void t_pos_too_many(void) {
  seargs_parser_t *p = PARSE(pos_defs, "prog", "a", "b", "c");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_TOO_MANY_ARGS);
  CHECK(p->error.index == SIZE_MAX);
  CHECK(p->error.arg != NULL && strcmp(p->error.arg, "c") == 0);
  seargs_free(p);
}

static void t_pos_typed(void) {
  // Negative positional values must be forced with `--`: a dash-shaped token is
  // an option otherwise.
  seargs_parser_t *p = PARSE(pos_typed_defs, "prog", "5", "--", "-1.5", "true");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_get_int(p, "num") == 5);
  CHECK(seargs_get_double(p, "d") == -1.5);
  CHECK(seargs_get_bool(p, "b") == true);
  seargs_free(p);

  p = PARSE(pos_typed_defs, "prog", "5", "-1.5", "true"); // "-1.5" is option-shaped
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_UNKNOWN_ARG);
  seargs_free(p);

  p = PARSE(pos_typed_defs, "prog", "5", "x", "true");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  CHECK(p->error.index == 1); // "d" failed to convert
  seargs_free(p);
}

static void t_pos_name_not_option(void) {
  // A positional's name is not usable as an option.
  seargs_parser_t *p = PARSE(pos_defs, "prog", "--input", "x");
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_UNKNOWN_ARG);
  seargs_free(p);
}

static void t_no_pos_unaffected(void) {
  // Without positional defs, bare tokens stay ignored and a lone '-' errors.
  seargs_parser_t *p = PARSE(common_defs, "prog", "foo", "bar");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(seargs_is_found(p, "flag") == false);
  seargs_free(p);
}

static void t_pos_value_too_long(void) {
  char big[ARG_VAL_MAX_SIZE + 20];
  memset(big, 'x', sizeof(big) - 1);
  big[sizeof(big) - 1] = '\0';

  seargs_parser_t *p = seargs_parse_args(2, (char *const[]){"prog", big}, pos_defs,
                                         sizeof(pos_defs) / sizeof(pos_defs[0]));
  CHECK(p != NULL);
  CHECK(p->error.code == SEARGS_INVALID_VALUE);
  CHECK(p->error.index == 1); // "input"
  seargs_free(p);
}

static void t_pos_empty_value(void) {
  seargs_parser_t *p = PARSE(pos_defs, "prog", "", "b");
  CHECK(p && p->error.code == SEARGS_OK);
  CHECK(strcmp(seargs_get_string(p, "input"), "") == 0);
  CHECK(strcmp(seargs_get_string(p, "input2"), "b") == 0);
  seargs_free(p);
}

static void t_macro_parity(void) {
  // Fill every argument explicitly, including all nine positional slots.
  char *const argv_full[] = {
      "prog", "--flag", "-r",  "--bd=false", "--str", "hi",    "-q",  "qv", "--dstr=x", "--num",
      "5",    "-m",     "2",   "--dnum",     "9",     "--dbl", "1.5", "-y", "3",        "--ddbl",
      "4.5",  "one",    "two", "three",      "4",     "5",     "6",   "7",  "8",        "9"};
  seargs_parser_t *pa =
      seargs_parse_args((int)(sizeof(argv_full) / sizeof(argv_full[0])), argv_full, macro_defs,
                        sizeof(macro_defs) / sizeof(macro_defs[0]));
  seargs_parser_t *pb =
      seargs_parse_args((int)(sizeof(argv_full) / sizeof(argv_full[0])), argv_full, raw_defs,
                        sizeof(raw_defs) / sizeof(raw_defs[0]));
  CHECK(pa != NULL && pa->error.code == SEARGS_OK);
  CHECK(pb != NULL && pb->error.code == SEARGS_OK);
#define SAME_VAL(n) CHECK(same_val(pa, pb, #n))
  SAME_VAL(flag);
  SAME_VAL(rb);
  SAME_VAL(bd);
  SAME_VAL(str);
  SAME_VAL(qstr);
  SAME_VAL(dstr);
  SAME_VAL(num);
  SAME_VAL(rnum);
  SAME_VAL(dnum);
  SAME_VAL(dbl);
  SAME_VAL(rdbl);
  SAME_VAL(ddbl);
  SAME_VAL(pstr);
  SAME_VAL(prstr);
  SAME_VAL(pdstr);
  SAME_VAL(pnum);
  SAME_VAL(prnum);
  SAME_VAL(pdnum);
  SAME_VAL(pdbl);
  SAME_VAL(prdbl);
  SAME_VAL(pddbl);
#undef SAME_VAL
  seargs_free(pa);
  seargs_free(pb);

  // Same options minus the explicit defaults and the trailing positional: the
  // absent optional arguments must fall back to their macro-declared defaults
  // (bd=true, dstr="def", dnum=7, ddbl=2.5, pddbl=1.5) in both tables.
  char *const argv_def[] = {"prog", "--flag", "-r",    "-q",  "qv", "--num", "5",
                            "-m",   "2",      "--dbl", "1.5", "-y", "3",     "one",
                            "two",  "three",  "4",     "5",   "6",  "7",     "8"};
  pa = seargs_parse_args((int)(sizeof(argv_def) / sizeof(argv_def[0])), argv_def, macro_defs,
                         sizeof(macro_defs) / sizeof(macro_defs[0]));
  pb = seargs_parse_args((int)(sizeof(argv_def) / sizeof(argv_def[0])), argv_def, raw_defs,
                         sizeof(raw_defs) / sizeof(raw_defs[0]));
  CHECK(pa != NULL && pa->error.code == SEARGS_OK);
  CHECK(pb != NULL && pb->error.code == SEARGS_OK);
#define SAME_VAL(n) CHECK(same_val(pa, pb, #n))
  SAME_VAL(flag);
  SAME_VAL(rb);
  SAME_VAL(bd);
  SAME_VAL(str);
  SAME_VAL(qstr);
  SAME_VAL(dstr);
  SAME_VAL(num);
  SAME_VAL(rnum);
  SAME_VAL(dnum);
  SAME_VAL(dbl);
  SAME_VAL(rdbl);
  SAME_VAL(ddbl);
  SAME_VAL(pstr);
  SAME_VAL(prstr);
  SAME_VAL(pdstr);
  SAME_VAL(pnum);
  SAME_VAL(prnum);
  SAME_VAL(pdnum);
  SAME_VAL(pdbl);
  SAME_VAL(prdbl);
  SAME_VAL(pddbl);
#undef SAME_VAL
  seargs_free(pa);
  seargs_free(pb);

  // Only the required arguments and the required positionals are given; every
  // optional falls back to its macro-declared default in both tables.
  char *const argv_abs[] = {"prog", "-r",  "-q",    "qv", "-m", "2", "-y", "3",
                            "one",  "two", "three", "4",  "5",  "6", "7",  "8"};
  pa = seargs_parse_args((int)(sizeof(argv_abs) / sizeof(argv_abs[0])), argv_abs, macro_defs,
                         sizeof(macro_defs) / sizeof(macro_defs[0]));
  pb = seargs_parse_args((int)(sizeof(argv_abs) / sizeof(argv_abs[0])), argv_abs, raw_defs,
                         sizeof(raw_defs) / sizeof(raw_defs[0]));
  CHECK(pa != NULL && pa->error.code == SEARGS_OK);
  CHECK(pb != NULL && pb->error.code == SEARGS_OK);
#define SAME_VAL(n) CHECK(same_val(pa, pb, #n))
  SAME_VAL(flag);
  SAME_VAL(rb);
  SAME_VAL(bd);
  SAME_VAL(str);
  SAME_VAL(qstr);
  SAME_VAL(dstr);
  SAME_VAL(num);
  SAME_VAL(rnum);
  SAME_VAL(dnum);
  SAME_VAL(dbl);
  SAME_VAL(rdbl);
  SAME_VAL(ddbl);
  SAME_VAL(pstr);
  SAME_VAL(prstr);
  SAME_VAL(pdstr);
  SAME_VAL(pnum);
  SAME_VAL(prnum);
  SAME_VAL(pdnum);
  SAME_VAL(pdbl);
  SAME_VAL(prdbl);
  SAME_VAL(pddbl);
#undef SAME_VAL
  // The defaults themselves, as produced by the macro-built table.
  CHECK(!seargs_is_found(pa, "flag") && seargs_get_bool(pa, "flag") == false);
  CHECK(!seargs_is_found(pa, "bd") && seargs_get_bool(pa, "bd") == true);
  CHECK(!seargs_is_found(pa, "str") && seargs_get_string(pa, "str") == NULL);
  CHECK(!seargs_is_found(pa, "dstr") && strcmp(seargs_get_string(pa, "dstr"), "def") == 0);
  CHECK(!seargs_is_found(pa, "num") && seargs_get_int(pa, "num") == 0);
  CHECK(!seargs_is_found(pa, "dnum") && seargs_get_int(pa, "dnum") == 7);
  CHECK(!seargs_is_found(pa, "dbl") && seargs_get_double(pa, "dbl") == 0.0);
  CHECK(!seargs_is_found(pa, "ddbl") && seargs_get_double(pa, "ddbl") == 2.5);
  CHECK(!seargs_is_found(pa, "pddbl") && seargs_get_double(pa, "pddbl") == 1.5);
  seargs_free(pa);
  seargs_free(pb);

  // A table with only optional positionals and no tokens: the defaults apply.
  pa = seargs_parse_args(1, (char *const[]){"prog"}, pos_default_defs,
                         sizeof(pos_default_defs) / sizeof(pos_default_defs[0]));
  CHECK(pa != NULL && pa->error.code == SEARGS_OK);
  CHECK(!seargs_is_found(pa, "ps") && strcmp(seargs_get_string(pa, "ps"), "") == 0);
  CHECK(!seargs_is_found(pa, "pi") && seargs_get_int(pa, "pi") == -1);
  CHECK(!seargs_is_found(pa, "pd") && seargs_get_double(pa, "pd") == -1.5);
  seargs_free(pa);
}

int main(void) {
  t_long_bool();
  t_bool_eq();
  t_bool_absent_default();
  t_int_values();
  t_int_dash_bad();
  t_int_terminator_value();
  t_str_values();
  t_short_cluster();
  t_short_cluster_bad();
  t_lone_dash();
  t_unknown_long();
  t_required_missing();
  t_missing_value();
  t_value_too_long();
  t_duplicates();
  t_empty_values();
  t_double_policies();
  t_terminator();
  t_invalid_flags();
  t_null_defs();
  t_free_null();
  t_name_too_long();
  t_pos_fill_order();
  t_pos_option_value_priority();
  t_pos_after_terminator();
  t_pos_lone_dash();
  t_pos_required_missing();
  t_pos_too_many();
  t_pos_typed();
  t_pos_name_not_option();
  t_no_pos_unaffected();
  t_pos_value_too_long();
  t_pos_empty_value();
  t_macro_parity();

  printf("%d checks, %d failures\n", checks, failures);
  return failures ? 1 : 0;
}