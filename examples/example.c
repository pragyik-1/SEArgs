#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const seargs_arg_def_t defs[] = {
    SEARGS_REQUIRED_STRING_ARG("text", 't', "the text to repeat"),
    SEARGS_OPTIONAL_INT_ARG("count", 'c', "how many times to print it", 1),
    SEARGS_OPTIONAL_BOOL_ARG("upper", 'u', "print in uppercase", false),
    SEARGS_OPTIONAL_STRING_ARG("sep", 's', "text printed between repeats", ""),
    // Positionals are matched by position, never by name (--extra is unknown).
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("extra", "optional trailing text", ""),
};

int main(int argc, char *const argv[]) {
  seargs_parser_t *p = seargs_parse_args(argc, argv, defs, sizeof(defs) / sizeof(defs[0]));

  if (!p) {
    fprintf(stderr, "error: out of memory\n");
    return EXIT_FAILURE;
  }

  // Any definition or command-line problem lands here. index is the def index
  // that caused it (SIZE_MAX when not applicable), arg is the argv token
  // involved (NULL when none), and detail is a short human-readable reason.
  if (!IS_SEARGS_OK(p)) {
    fprintf(stderr, "error %d", (int)p->error.code);
    if (p->error.index != SIZE_MAX)
      fprintf(stderr, " (argument '%s')", defs[p->error.index].name);
    if (p->error.arg)
      fprintf(stderr, ", near '%s'", p->error.arg);
    if (p->error.detail)
      fprintf(stderr, ": %s", p->error.detail);
    fputc('\n', stderr);
    seargs_free(p);
    return EXIT_FAILURE;
  }

  // Typed accessors: each asserts that the argument's type matches, and
  // returns the definition's default when the argument was not provided.
  // String values alias the caller's argv strings, so they stay valid for
  // the lifetime of the process.
  const char *text = seargs_get_string(p, "text");
  const int count = seargs_get_int(p, "count");
  const char *sep = seargs_get_string(p, "sep");

  // An accepted value is at most ARG_VAL_MAX_SIZE - 1 characters long, so a
  // fixed buffer is always big enough for the uppercased copy.
  char upper[ARG_VAL_MAX_SIZE];
  const char *out = text;
  if (seargs_get_bool(p, "upper")) {
    strncpy(upper, text, sizeof(upper) - 1);
    upper[sizeof(upper) - 1] = '\0';
    for (char *c = upper; *c; c++)
      *c = (char)toupper((unsigned char)*c);
    out = upper;
  }

  for (int i = 0; i < count; i++) {
    if (i > 0 && sep[0])
      fputs(sep, stdout);
    fputs(out, stdout);
  }
  fputc('\n', stdout);

  // seargs_is_found distinguishes "not given" (default returned, false here)
  // from "explicitly given" (true).
  if (seargs_is_found(p, "extra"))
    printf("extra: %s\n", seargs_get_string(p, "extra"));

  seargs_free(p);
  return EXIT_SUCCESS;
}