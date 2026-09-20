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

  printf("%d checks, %d failures\n", checks, failures);
  return failures ? 1 : 0;
}