// Verifies the formatting of seargs_print_help, especially that positional
// arguments are not rendered as options.
#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

#include <stdio.h>
#include <string.h>

static const seargs_arg_def_t defs[] = {
    SEARGS_REQUIRED_STRING_ARG("output", 'o', "output file"),
    SEARGS_POSITIONAL_REQUIRED_STRING_ARG("input", "input file"),
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("extra", "extra text", ""),
};

static const seargs_arg_def_t malformed[] = {
    {.name = NULL, .type = SEARGS_TYPE_BOOL},
};

int main(void) {
  const char *path = "help_test.out";
  int ok = 1;

  if (!freopen(path, "w", stdout)) {
    fprintf(stderr, "help_test: freopen failed\n");
    return 1;
  }

  seargs_print_help(defs, sizeof(defs) / sizeof(defs[0]));
  // A malformed table must not crash even if a caller invokes help directly.
  seargs_print_help(malformed, sizeof(malformed) / sizeof(malformed[0]));
  fflush(stdout);
  fclose(stdout);

  FILE *in = fopen(path, "r");
  if (!in) {
    fprintf(stderr, "help_test: cannot reopen %s\n", path);
    return 1;
  }

  char buf[4096];
  const size_t n = fread(buf, 1, sizeof(buf) - 1, in);
  buf[n] = '\0';
  fclose(in);
  remove(path);

  if (!strstr(buf, "--output"))
    ok = 0;
  if (!strstr(buf, "<input>"))
    ok = 0;
  if (!strstr(buf, "[extra]"))
    ok = 0;
  if (strstr(buf, "--input") || strstr(buf, "--extra"))
    ok = 0;

  if (!ok) {
    fprintf(stderr, "help_test: unexpected output:\n%s", buf);
    return 1;
  }

  return 0;
}
